#pragma once
#include "Order.hpp"
#include <map>
#include <deque>
#include <vector>
#include <optional>

namespace mmx {

// Price-time priority order book per symbol.
class OrderBook {
public:
    void add_order(const Order& o) {
        if (o.side == Side::Buy) {
            bids_[o.price].push_back(o);
        } else {
            asks_[o.price].push_back(o);
        }
    }

    // Match incoming order against the opposite side.
    std::vector<Trade> match(Order taker) {
        std::vector<Trade> trades;
        if (taker.side == Side::Buy) {
            // Crosses asks from best (lowest) price
            auto it = asks_.begin();
            while (taker.qty > 0 && it != asks_.end() && it->first <= taker.price) {
                auto& q = it->second;
                while (taker.qty > 0 && !q.empty()) {
                    auto& maker = q.front();
                    int64_t qty = std::min(maker.qty, taker.qty);
                    trades.push_back(Trade{maker.id, taker.id, taker.symbol, it->first, qty, taker.ts_ns});
                    maker.qty -= qty;
                    taker.qty -= qty;
                    if (maker.qty == 0) q.pop_front();
                }
                if (q.empty()) it = asks_.erase(it);
                else ++it;
            }
            if (taker.qty > 0) add_order(taker);
        } else {
            // Sell crosses bids from best (highest) price
            auto it = bids_.rbegin();
            while (taker.qty > 0 && it != bids_.rend() && it->first >= taker.price) {
                auto& q = it->second;
                while (taker.qty > 0 && !q.empty()) {
                    auto& maker = q.front();
                    int64_t qty = std::min(maker.qty, taker.qty);
                    trades.push_back(Trade{maker.id, taker.id, taker.symbol, it->first, qty, taker.ts_ns});
                    maker.qty -= qty;
                    taker.qty -= qty;
                    if (maker.qty == 0) q.pop_front();
                }
                if (q.empty()) {
                    // erase reverse iterator
                    auto base = it.base();
                    if (base != bids_.begin()) {
                        --base;
                        base = bids_.erase(base);
                        it = std::make_reverse_iterator(base);
                    } else {
                        bids_.erase(bids_.begin());
                        it = std::make_reverse_iterator(bids_.begin());
                    }
                } else ++it;
            }
            if (taker.qty > 0) add_order(taker);
        }
        return trades;
    }

    size_t depth() const noexcept { return bids_.size() + asks_.size(); }

private:
    // For bids: ascending map; we read from rbegin() for best price
    std::map<int64_t, std::deque<Order>> bids_;
    // For asks: ascending; we read from begin() for best price
    std::map<int64_t, std::deque<Order>> asks_;
};

} // namespace mmx
