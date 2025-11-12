# Mini Matching Engine (C++20, Lock-Free, Low-Latency)


A lightweight, **low-latency order-matching backend** that simulates core exchange behavior with **in-memory order books**, **asynchronous event handling**, and a **lock-free SPSC ring buffer**. Designed to be simple, portable (no Boost), and technically credible for trading/HFT roles.


## 🎯 Goal
- Deterministic, price–time priority matching
- Minimal allocations in the hot path
- Single-threaded matcher for predictable book state
- Lock-free ingestion path from producers to matcher
- Demonstrate **100K+ orders/sec** on a modern desktop CPU


## 🧱 Architecture
- **Producers → (Lock-free SPSC queue) → Matcher → Event Sink**
- Per-symbol `OrderBook` with bid/ask maps and FIFO at each price level
- Async-style event handling simulated via non-blocking publish/print


### Data Model
- `Order { id, symbol, side, price, qty }`
- `Trade { symbol, price, qty, buy_id, sell_id }`


### Code Flow
```text
main.cpp
└── Engine.start()
└── matcher thread loop
├─ pop order from LockFreeSPSC
├─ locate OrderBook (by symbol)
├─ OrderBook.add(order)
│ ├─ cross against opposite side (best price first)
│ ├─ emit trades (price–time priority)
│ └─ enqueue remainder to same-side book
└─ publish trades (stdout placeholder)
```


## 🚀 Build
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```


## ▶️ Run (Demo)
```bash
./build/mini_match
```
- Generates random BUY/SELL orders around price 100 for symbol `XYZ`
- Prints matched trades in real-time


## 📈 Run (Benchmark)
```bash
./build/benchmark
```
- Sends 100,000 synthetic orders through the engine and prints throughput
- Adjust `N` in `bench/benchmark.cpp` to stress further


## 🧪 Tests
```bash
ctest --test-dir build --output-on-failure
```
- `tests/sanity_tests.cpp` verifies price–time priority and crossing


## ⚙️ Configuration Tips
- Queue size: constructor param in `Engine(queue_size)` (power-of-two recommended)
- CPU pinning: add `pthread_setaffinity_np`/`sched_setaffinity` around `matcher_`
- Logging: replace `std::cout` in the hot path with a non-blocking
