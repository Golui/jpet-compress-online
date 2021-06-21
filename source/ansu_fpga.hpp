#pragma once

#include "ansu.hpp"

#include "ansu/backend/stream.hpp"
#include "ansu/data/compression_table.hpp"
#include "ansu/ints.hpp"
#include "ansu/settings.hpp"

#include <array>

template <typename... T>
using static_vector = std::array<T..., CHANNEL_COUNT>;

using ContextT = ANS::ChannelCompressionContext<ANS::StaticCompressionTable, static_vector>;

extern std::shared_ptr<ContextT> mainCtxPtr;

void fpga_compress(ANS::backend::side_stream<typename ContextT::ReducedSymbolT>& message,
		ANS::backend::stream<typename ContextT::StateT>& out,
		ANS::backend::stream<typename ContextT::Meta>& meta);
