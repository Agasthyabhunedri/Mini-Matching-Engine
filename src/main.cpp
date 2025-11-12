#include "mmx/Order.hpp"
#include "mmx/LockFreeRing.hpp"
#include "mmx/OrderBook.hpp"
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

// Simple synthetic benchmark driving the engine to demonstrate 100K+ orders/sec on typical machines.

using namespace std::chrono;

#include "mmx/Order.hpp"
#include <atomic>

namespace mmx {
struct Event { Order order; };

class MatchingEngineImpl {
public:
    MatchingEngineImpl(size_t qcap_pow2, int workers);
    void start();
    void stop();
    bool submit(const Order& o);
    bool poll_trade(Trade& t);

private:
    void worker();
    LockFreeRing<Event> inq_{ 1u << 20 }; // ~1M capacity
    LockFreeRing<Trade> outq_{ 1u << 20 };
    OrderBook book_;
    std::vector<std::thread> threads_;
    std::atomic<bool> stop_{ false };
    int workers_{ 1 };
};

MatchingEngineImpl::MatchingEngineImpl(size_t qcap_pow2, int workers)
    : inq_(qcap_pow2), outq_(qcap_pow2), workers_(workers) {}

void MatchingEngineImpl::start() {
    stop_.store(false, std::memory_order_relaxed);
    for (int i = 0; i < workers_; ++i) {
        threads_.emplace_back([this] { this->worker(); });
    }
}

void MatchingEngineImpl::stop() {
    stop_.store(true, std::memory_order_relaxed);
    for (auto& t : threads_) if (t.joinable()) t.join();
}

bool MatchingEngineImpl::submit(const Order& o) { return inq_.enqueue(Event{ o }); }

bool MatchingEngineImpl::poll_trade(Trade& t) { return outq_.dequeue(t); }

void MatchingEngineImpl::worker() {
    Event ev;
    while (!stop_.load(std::memory_order_relaxed)) {
        if (!inq_.dequeue(ev)) {
            std::this_thread::yield();
            continue;
        }
        auto trades = book_.match(ev.order);
        for (auto& tr : trades) {
            while (!outq_.enqueue(tr)) std::this_thread::yield();
        }
    }
}
} // namespace mmx

int main() {
    using namespace mmx;
    const int workers = std::max(1u, std::thread::hardware_concurrency() / 2);
    MatchingEngineImpl engine(1u << 18, workers); // 262,144 capacity queues
    engine.start();

    const size_t N = 200'000; // total simulated orders
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<int> price(99'50, 100'50); // ticks (e.g., 9950=99.50)
    std::uniform_int_distribution<int> qty(1, 10);
    std::bernoulli_distribution side(0.5);

    auto t0 = high_resolution_clock::now();
    for (size_t i = 0; i < N; ++i) {
        Order o;
        o.id = i + 1;
        o.side = side(rng) ? Side::Buy : Side::Sell;
        o.symbol = "TEST";
        o.price = price(rng);
        o.qty = qty(rng);
        o.ts_ns = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
        // Busy-wait if queue is momentarily full to keep benchmark simple
        while (!engine.submit(o)) { std::this_thread::yield(); }
    }

    // Give workers time to drain
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    engine.stop();
    auto t1 = high_resolution_clock::now();

    double sec = duration<double>(t1 - t0).count();
    double rate = N / sec;

    std::cout << "Orders submitted: " << N << "\n";
    std::cout << "Elapsed: " << sec << " s\n";
    std::cout << "Throughput: " << static_cast<uint64_t>(rate) << " orders/sec\n";
    std::cout << "Workers: " << workers << "\n";
    return 0;
}
