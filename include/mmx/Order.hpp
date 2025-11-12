#pragma once
#include <cstdint>
#include <string>

namespace mmx {

enum class Side : uint8_t { Buy=0, Sell=1 };

struct Order {
    uint64_t id;
    Side side;
    std::string symbol;
    int64_t price;   // price in ticks
    int64_t qty;     // quantity in lots
    uint64_t ts_ns;  // event time (ns since epoch or monotonic)
};

struct Trade {
    uint64_t maker_id;
    uint64_t taker_id;
    std::string symbol;
    int64_t price;
    int64_t qty;
    uint64_t ts_ns;
};

} // namespace mmx
