#pragma once

#include "aegis_alloc/Allocator.h"

#include <cstddef>
#include <cstdint>

namespace aegis_alloc {

/**
 * @brief LIFO allocator with marker-based rollback.
 *
 * @details
 * StackAllocator behaves like a memory stack over a pre-allocated arena.
 * Allocations advance a top offset, and deallocation semantics are expected to
 * follow strict last-in-first-out (LIFO) order. In practice, marker-based
 * rollback is the common workflow:
 * - capture a Marker at a stable point
 * - allocate temporary data for nested tasks
 * - Rollback(marker) to reclaim everything allocated after that point
 *
 * This design provides deterministic memory behavior and minimal bookkeeping,
 * which is useful for transient systems in high-performance runtimes.
 */
class StackAllocator final : public Allocator {
 public:
  using Marker = std::size_t;

  /**
   * @brief Construct a stack allocator over an existing arena.
   *
   * @param arena Pointer to caller-owned contiguous memory.
   * @param arena_size Size of the arena in bytes.
   */
  StackAllocator(void* arena, std::size_t arena_size);

  ~StackAllocator() override = default;
  StackAllocator(const StackAllocator&) = delete;
  StackAllocator& operator=(const StackAllocator&) = delete;
  StackAllocator(StackAllocator&&) = delete;
  StackAllocator& operator=(StackAllocator&&) = delete;

  /**
   * @brief Allocate aligned memory by pushing the stack top.
   *
   * @param size Number of bytes requested.
   * @param alignment Required alignment in bytes.
   * @return Aligned pointer within the arena, or nullptr on exhaustion.
   */
  void* Allocate(std::size_t size, std::size_t alignment = sizeof(void*)) override;

  /**
   * @brief Release memory according to stack semantics.
   *
   * @param ptr Pointer previously returned by Allocate.
   */
  void Free(void* ptr) override;

  /**
   * @brief Reset allocator state to an empty stack.
   */
  void Reset() override;

  /**
   * @brief Capture the current stack top as a rollback point.
   *
   * @return Current marker value.
   */
  [[nodiscard]] Marker GetMarker() const;

  /**
   * @brief Rewind stack top to a previously captured marker.
   *
   * @param marker Marker returned by GetMarker().
   */
  void Rollback(Marker marker);

  /**
   * @brief Total arena capacity in bytes.
   */
  [[nodiscard]] std::size_t GetCapacity() const;

 private:
  std::byte* arena_ = nullptr;
  std::size_t arena_size_ = 0;
  Marker top_ = 0;
};

}  // namespace aegis_alloc
