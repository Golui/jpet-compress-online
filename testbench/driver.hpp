#pragma once

#include "CLI11.hpp"

#include "ansu/data/table_generator.hpp"
#include "ansu/ints.hpp"
#include "ansu/settings.hpp"

#include <map>
#include <memory.h>

namespace ANS
{
	namespace driver
	{
		enum class Alphabet : int
		{
			U8	= 8,
			U16 = 16,
		};

		enum class SummaryType : int
		{
			None,
			Human,
			CSV
		};

		static const std::map<std::string, Alphabet> ALPHABET_STR_TO_ENUM {
			{"8", Alphabet::U8}, {"16", Alphabet::U16}};

		static const std::map<std::string, SummaryType> SUMMARY_STR_TO_ENUM {
			{"none", SummaryType::None},
			{"human", SummaryType::Human},
			{"csv", SummaryType::CSV}};

		namespace compress
		{
			struct Options
			{
				std::string inFilePath			= "STDIN";
				std::string outFilePath			= "compressed.ansu";
				u64 checkpoint					= CHECKPOINT;
				u32 channels					= CHANNEL_COUNT;
				u64 chunkSize					= AVG_MESSAGE_LENGTH;
				Alphabet alphabet				= Alphabet::U8;
				u32 tableSizeLog				= 10;
				std::string tableFilePath		= "static";
				SummaryType printSummary		= SummaryType::None;
				bool skipPrompt					= false;
				bool warnUnknownSymbol			= false;
				strategies::Quantizer quantizer = strategies::Quantizer::Fast;
				strategies::Spreader spreader	= strategies::Spreader::Fast;
			};

			using OptionsP = std::shared_ptr<Options>;

			void subRegister(CLI::App& app);
		} // namespace compress
	}
} // namespace ANS
