#pragma once
#include <atomic>
#include <cstddef>
#include <optional>
#include <type_traits>

// A bounded MPMC lock-free ring buffer with power-of-two capacity.
// Simple scheme using sequence numbers; adapted conceptually from classic patterns.
// For educational/demo benchmarks; not a production queue.
namespace mmx {

template <typename T>
class LockFreeRing {
public:
    explicit LockFreeRing(size_t capacity_pow2)
        : mask_(capacity_pow2 - 1),
          buffer_(new Node[capacity_pow2]),
          capacity_(capacity_pow2)
    {
        // capacity must be power of two
        for (size_t i = 0; i < capacity_; ++i) {
            buffer_[i].seq.store(i, std::memory_order_relaxed);
        }
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
    }

    ~LockFreeRing() { delete[] buffer_; }

    bool enqueue(const T& v) noexcept {
        Node* node;
        size_t pos = head_.load(std::memory_order_relaxed);
        for (;;) {
            node = &buffer_[pos & mask_];
            size_t seq = node->seq.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)pos;
            if (dif == 0) {
                if (head_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    break;
                }
            } else if (dif < 0) {
                return false; // full
            } else {
                pos = head_.load(std::memory_order_relaxed);
            }
        }
        node->val = v;
        node->seq.store(pos + 1, std::memory_order_release);
        return true;
    }

    bool dequeue(T& out) noexcept {
        Node* node;
        size_t pos = tail_.load(std::memory_order_relaxed);
        for (;;) {
            node = &buffer_[pos & mask_];
            size_t seq = node->seq.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
            if (dif == 0) {
                if (tail_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    break;
                }
            } else if (dif < 0) {
                return false; // empty
            } else {
                pos = tail_.load(std::memory_order_relaxed);
            }
        }
        out = std::move(*node->val);
        node->val.reset();
        node->seq.store(pos + mask_ + 1, std::memory_order_release);
        return true;
    }

    size_t capacity() const noexcept { return capacity_; }

private:
    struct Node {
        std::atomic<size_t> seq;
        std::optional<T> val;
        char pad[64]; // reduce false sharing
    };

    const size_t mask_;
    Node* buffer_;
    const size_t capacity_;
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};
};

} // namespace mmx
