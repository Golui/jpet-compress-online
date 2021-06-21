#include "ansu_fpga.hpp"

auto mainCtx = ContextT(CHANNEL_COUNT);
std::shared_ptr<ContextT> mainCtxPtr = std::shared_ptr<ContextT>(&mainCtx, [](ContextT* ptr){});

void fpga_compress(ANS::backend::side_stream<typename ContextT::ReducedSymbolT>& message,
		ANS::backend::stream<typename ContextT::StateT>& out,
		ANS::backend::stream<typename ContextT::Meta>& meta)
{
	PRAGMA_HLS(interface axis port=message depth=AVG_MESSAGE_LENGTH)
	mainCtx.compressImpl(message, out, meta);
}
