#pragma once

#include <atomic>
#include <array>

/** A Lock-Free buffer that always reads the last written data. */
template <typename T>
class TripleBuffer
{
public:
    void write (T& data)
    {
        // Write to back buffer
        buffer[back] = std::move (data);

        // Swap buffers
        uint8_t newSpare = (uint8_t)back | (1 << 7);
        uint8_t prevSpare = spare.exchange (newSpare, std::memory_order_acq_rel);
        back = prevSpare & 0x0F;
    }

    T& read()
    {
        if (newDataReady())
        {
            // Swap buffers
            auto newSpare = (uint8_t)front;
            uint8_t prevSpare = spare.exchange (newSpare, std::memory_order_acq_rel);
            front = prevSpare & 0x0F;
        }

        return buffer[front];
    }

    bool newDataReady() const
    {
        return spare.load (std::memory_order_acquire) >> 7;
    }

private:
    //======================================================================
    size_t front { 0 };
    std::atomic<uint8_t> spare { 1 }; // initial buffer idx = 1, dataReady = false
    size_t back { 2 };

    //======================================================================
    std::array<T, 3> buffer {};

    // Implementation is lock-free as long as
    // this static_assert passes
    static_assert(std::atomic<uint8_t>::is_always_lock_free);
};