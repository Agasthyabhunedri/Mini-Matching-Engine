# Mini-Matching-Engine
This project implements a **low-latency order-matching backend** simulating exchange systems with in-memory order books, asynchronous event handling, and **lock-free queues**, capable of processing **100K+ orders per second** on a modern CPU.

```markdown
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
