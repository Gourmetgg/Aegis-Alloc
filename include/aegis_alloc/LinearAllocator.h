#pragma once

#include "aegis_alloc/Allocator.h"

#include <cstddef>
#include <cstdint>

namespace aegis_alloc {

/**
 * @brief Monotonic allocator backed by a fixed-size contiguous memory arena.
 *
 * @details
 * LinearAllocator serves allocation requests by moving an internal offset
 * forward inside a pre-allocated arena. It never reuses released memory until
 * Reset() is called, making it extremely cache-friendly and predictable for
 * frame-temporary allocations in a game engine.
 *
 * Typical usage pattern:
 * - create allocator with a caller-owned arena (e.g., 1 MiB buffer)
 * - Allocate() many short-lived objects during a frame/job
 * - call Reset() at a known synchronization point
 */
class LinearAllocator final : public Allocator {
 public:
  /**
   * @brief Construct a linear allocator over an existing arena.
   *
   * @param arena Pointer to caller-owned contiguous memory.
   * @param arena_size Size of the arena in bytes.
   */
  LinearAllocator(void* arena, std::size_t arena_size);

  ~LinearAllocator() override = default;
  LinearAllocator(const LinearAllocator&) = delete;
  LinearAllocator& operator=(const LinearAllocator&) = delete;
  LinearAllocator(LinearAllocator&&) = delete;
  LinearAllocator& operator=(LinearAllocator&&) = delete;

  /**
   * @brief Allocate aligned memory by advancing the internal offset.
   *
   * @param size Number of bytes requested.
   * @param alignment Required alignment in bytes (power-of-two expected).
   * @return Aligned pointer inside the arena, or nullptr on exhaustion.
   */
  void* Allocate(std::size_t size, std::size_t alignment = sizeof(void*)) override;

  /**
   * @brief Linear allocators usually cannot free individual allocations.
   *
   * @param ptr Pointer previously returned by Allocate.
   */
  void Free(void* ptr) override;

  /**
   * @brief Rewind allocator offset to the beginning of the arena.
   */
  void Reset() override;

  /**
   * @brief Current consumed bytes from the arena start.
   */
  [[nodiscard]] std::size_t GetOffset() const;

  /**
   * @brief Total arena capacity in bytes.
   */
  [[nodiscard]] std::size_t GetCapacity() const;

 private:
  std::byte* arena_ = nullptr;
  std::size_t arena_size_ = 0;
  std::size_t offset_ = 0;
};

}  // namespace aegis_alloc
