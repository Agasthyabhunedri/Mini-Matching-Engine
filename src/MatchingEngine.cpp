#include "mmx/OrderBook.hpp"
#include "mmx/LockFreeRing.hpp"
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <iostream>

namespace mmx {

struct Event {
    Order order;
};

class MatchingEngineImpl {
public:
    MatchingEngineImpl(size_t qcap_pow2, int workers)
        : inq_(qcap_pow2), outq_(qcap_pow2), workers_(workers) {}

    void start() {
        stop_.store(false, std::memory_order_relaxed);
        for (int i = 0; i < workers_; ++i) {
            threads_.emplace_back([this] { this->worker(); });
        }
    }

    void stop() {
        stop_.store(true, std::memory_order_relaxed);
        for (auto& t : threads_) if (t.joinable()) t.join();
    }

    bool submit(const Order& o) {
        return inq_.enqueue(Event{ o });
    }

    bool poll_trade(Trade& t) {
        return outq_.dequeue(t);
    }

private:
    void worker() {
        Event ev;
        while (!stop_.load(std::memory_order_relaxed)) {
            if (!inq_.dequeue(ev)) {
                std::this_thread::yield();
                continue;
            }
            auto trades = book_.match(ev.order);
            for (auto& tr : trades) {
                while (!outq_.enqueue(tr)) {
                    std::this_thread::yield();
                }
            }
        }
    }

    LockFreeRing<Event> inq_;
    LockFreeRing<Trade> outq_;
    OrderBook book_;
    std::vector<std::thread> threads_;
    std::atomic<bool> stop_{false};
    int workers_;
};

} // namespace mmx
