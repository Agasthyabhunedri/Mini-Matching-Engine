# Mini Matching Engine

**Tech:** C++20 · Lock-Free · Multithreading  
**Goal:** Low-latency, in-memory matching engine simulating exchange-style price-time priority with asynchronous event handling.

---

## Features
- **In‑memory limit order book** (price‑time priority) per symbol
- **Lock‑free MPMC rings** for ingest and trade outflow (bounded, power‑of‑two)
- **Multi‑threaded workers** (configurable) processing order flow
- **Synthetic benchmark** targeting **100K+ orders/sec** on commodity hardware

> This is a compact educational/portfolio engine — optimized data structures & concurrency without external deps.

---

## Build

### Linux / macOS
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

### Windows (MSVC)
```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

---

## Run

### Benchmark
```bash
./build/mmx_bench
# Output:
# Orders submitted: 200000
# Elapsed: 1.5 s
# Throughput: 133333 orders/sec
# Workers: 4
```

### Basic Test
```bash
./build/mmx_tests
```

---

## Code Flow
1. **Producer** submits `Order` events into a **lock‑free ring** (`LockFreeRing<T>`).
2. **Worker threads** dequeue, call `OrderBook::match`, and enqueue `Trade` outputs.
3. **OrderBook** maintains price‑time priority using `std::map<price, deque<Order>>`.
4. **Bench driver** generates synthetic flow and prints throughput.

---

## Notes
- Rings are **bounded MPMC** using sequence numbers (power‑of‑two capacity).
- For clarity and portability, **no external dependencies** are used.
- The queue is designed for **demo purposes**; production engines require more edge‑case handling.

---

## License
MIT
