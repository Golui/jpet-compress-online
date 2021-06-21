#include "ansu_fpga.hpp"
#include "driver.hpp"

#include "ansu.hpp"
#include "ansu/io/archive.hpp"
#include "ansu/io/table_archive.hpp"
#include "ansu/util.hpp"

#include "CLI11.hpp"

#include "cereal/types/array.hpp"

#include <array>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <stdio.h>

struct SpecialValidator : public CLI::Validator
{
	SpecialValidator()
	{
		name_ = "ANSUSPECIAL";
		func_ = [](const std::string& str) {
			if(str != "" && str != "static")
				return std::string("Not a special table type.");
			return std::string("");
		};
	}
};

template <typename T>
std::streamsize getFileSize(T& file)
{
	file.clear(); //  Since ignore will have set eof.
	file.seekg(0, std::ios_base::beg);
	file.ignore(std::numeric_limits<std::streamsize>::max());
	std::streamsize length = file.gcount();
	file.clear(); //  Since ignore will have set eof.
	file.seekg(0, std::ios_base::beg);
	return length;
}

u32 filter(ANS::backend::side_stream<typename ContextT::SymbolT>& in,
		ANS::backend::side_stream<typename ContextT::ReducedSymbolT>& out)
{
	using SideSymbolT = ANS::backend::side<typename ContextT::SymbolT>;
	using SideReducedSymbolT = ANS::backend::side<typename ContextT::ReducedSymbolT>;

	bool hadAny = false;
	bool hadLast = false;
	SideReducedSymbolT buffer[4];
	SideSymbolT cur[4];

	u32 count = 0;
	while(!in.empty())
	{
		for(u32 i = 0; i < 4; i++)
		{
			in >> cur[i];
			if(cur[i].last) hadLast = true;
		}

		bool skip = false;
		for(u32 i = 0; i < 4; i++)
		{
			skip |= !mainCtxPtr->ansTable.hasSymbolInAlphabet(cur[i].data);
		}

		if(!skip)
		{
			if(hadAny)
			{
				for(u32 i = 0; i < 4; i++) out << buffer[i];
				count += 4;
			}
			for(u32 i = 0; i < 4; i++)
			{
				SideReducedSymbolT& rcur = buffer[i];
				rcur.data = mainCtxPtr->ansTable.reverseAlphabet(cur[i].data);
				rcur.last = false;
			}
			hadAny = true;
		}
	}

	if(hadAny)
	{
		if(hadLast) buffer[3].last = true;
		for(u32 i = 0; i < 4; i++) out << buffer[i];
		count += 4;
	}

	return count;
}

template <typename SymbolT>
int compressTask(ANS::driver::compress::OptionsP opts,
				 std::istream& in)
{
	using StateT		 = typename ContextT::StateT;
	using ReducedSymbolT = typename ContextT::ReducedSymbolT;
	using Meta			 = typename ContextT::Meta;
	using InDataT		 = ANS::backend::side<SymbolT>;

	using CharT					  = std::istream::char_type;
	constexpr auto streamCharSize = sizeof(CharT);

	auto& mainCtx = *mainCtxPtr;

	mainCtx.setCheckpointFrequency(opts->checkpoint);
	mainCtx.setChunkSize(opts->chunkSize);

	ANS::io::ArchiveWriter<ContextT> writer(opts->outFilePath);

	writer.bindContext(mainCtxPtr);

	auto begin = std::chrono::steady_clock::now();

	const auto symbolWidth =
		(ANS::integer::nextPowerOfTwo(mainCtx.ansTable.symbolWidth()) >> 3);
	const auto chunkByteSize = opts->chunkSize * symbolWidth;

	std::array<u64, 256> counts = {0};
	SymbolT* msgbuf				= new SymbolT[opts->chunkSize];
	StateT* statebuf			= new StateT[opts->checkpoint];

	ANS::backend::side_stream<SymbolT> msg("Message");
	ANS::backend::side_stream<ReducedSymbolT> fmsg("FilteredMessage");
	ANS::backend::stream<StateT> out("Output");
	ANS::backend::stream<Meta> ometa("Meta");

	// TODO we only support types with power of 2 widths

	u32 j			= 0;
	u64 dataWritten = 0;
	u64 inSize = 0;
	while(in.peek() != EOF)
	{
		u32 i = 0;

		in.read((CharT*) msgbuf, chunkByteSize / streamCharSize);
		u64 read		= in.gcount();
		u64 readSymbols = read / symbolWidth;

		for(decltype(read) k = 0; k < read; k++)
		{
			counts[((u8*) msgbuf)[k]]++;
		}

		u32 validSymbols = 0;
		if(read != chunkByteSize || in.peek() == EOF)
		{
			for(; i < readSymbols - 1; i++)
			{
				auto data = InDataT();
				data.data = msgbuf[i];
				msg << data;
			}
			auto data = InDataT();
			data.data = msgbuf[i];
			data.last = true;
			msg << data;
		} else
		{
			for(; i < readSymbols; i++)
			{
				auto data = InDataT();
				data.data = msgbuf[i];
				msg << data;
			}
		}

		validSymbols = filter(msg, fmsg);
		fpga_compress(fmsg, out, ometa);

		inSize += validSymbols * symbolWidth;

		while(!out.empty())
		{
			for(; !out.empty() && j < opts->checkpoint; j++)
				statebuf[j] = out.read();
			if(!ometa.empty())
			{
				Meta meta	= ometa.read();
				auto result = writer.writeBlock(j, statebuf, meta);
				dataWritten += result;
				j = 0;
			}
		}
	}

	if(!ometa.empty())
	{
		Meta meta	= ometa.read();
		auto result = writer.writeBlock(0, statebuf, meta);
		dataWritten += result;
		if(!ometa.empty())
		{
			std::cerr << "Too many meta objects! Aborting." << std::endl;
			return EXIT_FAILURE;
		}
	}

	inSize *= sizeof(std::istream::char_type);

	// TODO encapsualte header, think of a better way to track inputSize
	writer.header.inputSize = inSize;

	if(inSize % symbolWidth != 0)
	{
		std::cerr
			<< "The given data does not evenly divide into symbols of width "
			<< mainCtx.ansTable.symbolWidth() << " bits. Aborting."
			<< std::endl;
		return EXIT_FAILURE;
	}

	if(opts->printSummary != ANS::driver::SummaryType::None)
	{
		auto end = std::chrono::steady_clock::now();
		auto timeS =
			std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
				.count()
			/ 1.0e6;

		auto outSize = getFileSize(writer.fileHandle);

		double entropy = 0.0;

		u64 sum = 0;

		for(u16 k = 0; k < 256; k++) sum += counts[k];
		for(u16 k = 0; k < 256; k++)
		{
			u32 count = counts[k];
			if(count != 0)
				entropy -= count / double(sum) * (log2(count / double(sum)));
		}

		auto entropy2percent = 100 / (symbolWidth << 3);

		auto dataWrittenBytes = dataWritten * sizeof(std::fstream::char_type);
		auto percentageFull	  = 100.0 * outSize / ((double) inSize);
		auto percentageData	  = 100.0 * dataWrittenBytes / ((double) inSize);
		auto percentageTheory = entropy * entropy2percent;

		auto entropyFull = percentageFull / entropy2percent;
		auto entropyData = percentageData / entropy2percent;

		auto entrUnit = " bits/byte";

		switch(opts->printSummary)
		{
			case ANS::driver::SummaryType::Human:
			{
				std::cout << std::setprecision(6);
				std::cout << "Done! Took " << timeS << "s \n";
				std::cout << "Stats:"
						  << "\n";
				std::cout << "\t"
						  << "Input size:       " << inSize << "\n";
				std::cout << "\t"
						  << "Output size:      " << outSize << "\n";
				std::cout << "\t\t"
						  << "Header size:      "
						  << writer.getHeader().totalHeaderSize << "\n";
				std::cout << "\t\t"
						  << "Data size:      " << dataWrittenBytes << "\n";
				std::cout << "\t"
						  << "Ratio:\n"
						  << "\t\t Full file:        " << percentageFull
						  << "%\n"
						  << "\t\t Without Header:   " << percentageData
						  << "%\n"
						  << "\t\t Theoretical best: " << percentageTheory
						  << "%\n";
				std::cout << "\t"
						  << "Entropy:\n"
						  << "\t\t Full file:        " << entropyFull
						  << entrUnit << "\n"
						  << "\t\t Without Header:   " << entropyData
						  << entrUnit << "\n"
						  << "\t\t Theoretical best: " << entropy << entrUnit
						  << "\n";
				break;
			}
			case ANS::driver::SummaryType::CSV:
			{
				std::cout << std::setprecision(6);
				std::cout << timeS << ", " << inSize << ", " << outSize << ", "
						  << writer.getHeader().totalHeaderSize << ", "
						  << dataWrittenBytes << ", " << percentageFull << ", "
						  << percentageData << ", " << percentageTheory << ", "
						  << entropyFull << ", " << entropyData << ", "
						  << entropy << "\n";
				break;
			}
			default: break;
		}
	}

	delete[] msgbuf;
	delete[] statebuf;

	return 0;
}

int run(ANS::driver::compress::OptionsP opts)
{
	std::ifstream inFile = std::ifstream(opts->inFilePath, std::ios::binary);

	return compressTask<message_t>(opts, inFile);
}

void ANS::driver::compress::subRegister(CLI::App& app)
{
	CLI::App* sub = app.add_subcommand("compress");

	auto opts = std::make_shared<Options>();

	sub->add_option("-f,--infile",
					opts->inFilePath,
					"the file to compress.")
		->check(CLI::ExistingFile);

	sub->add_option("-o,--outfile", opts->outFilePath, "the resulting archive");

	sub->add_option("-s",
					opts->printSummary,
					"Whether to print no, a human readable, or csv formatted "
					"summary after compressing or not.")
		->transform(
			CLI::CheckedTransformer(SUMMARY_STR_TO_ENUM, CLI::ignore_case));

	sub->add_flag("--warn-unknown-symbol",
				  opts->warnUnknownSymbol,
				  "Warn if an unknown symbol is encountered in the file.");

	sub->add_option(
		"-c",
		opts->checkpoint, "NYI");
//		"Specfy how often a checkpoint should be emitted. This is used to "
//		"verify data integrity, but increases the file size.");

	sub->add_option(
		"-n",
		opts->channels, "NYI");
//		"Specify how many channels (parallel coders) should be used.");

	sub->add_option(
		"-l", opts->chunkSize, "NYI"); //"How much data to consume per function call.");

	sub->add_option(
		   "-a", opts->alphabet, "NYI") // "The length of the alphabet when generating")
		->transform(
			CLI::CheckedTransformer(ALPHABET_STR_TO_ENUM, CLI::ignore_case));

	sub->add_option("-k", opts->tableSizeLog, "NYI"); // "Logarithm of the table size.");

	sub->final_callback([opts]() {
		run(opts);
	});
}


int main(int argc, const char** argv)
{
	CLI::App app {"ansu - a tANS implementation targeting FPGAs."};

	ANS::driver::compress::subRegister(app);
	app.require_subcommand(1);

	try
	{
		app.parse(argc, argv);
	} catch(const CLI::ParseError& e)
	{
		return app.exit(e);
	}

	return 0;

}
