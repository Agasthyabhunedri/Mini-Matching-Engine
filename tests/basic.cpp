#include "mmx/OrderBook.hpp"
#include <cassert>
#include <iostream>

int main() {
    using namespace mmx;
    OrderBook ob;
    ob.add_order(Order{1, Side::Buy, "TEST", 10000, 10, 0});
    auto trades = ob.match(Order{2, Side::Sell, "TEST", 10000, 5, 0});
    assert(trades.size() == 1);
    assert(trades[0].qty == 5);
    assert(ob.depth() >= 1); // remaining buy qty stays
    std::cout << "basic test passed\n";
    return 0;
}
