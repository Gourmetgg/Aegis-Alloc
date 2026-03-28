#pragma once

#include <cstddef>

namespace aegis_alloc {

/**
 * @brief Common interface for all custom allocators in Aegis-Alloc.
 *
 * Concrete allocators define how memory is reserved, aligned, and released.
 */
class Allocator {
 public:
  virtual ~Allocator() = default;

  /**
   * @brief Allocate a block of memory.
   *
   * @param size Number of bytes requested.
   * @param alignment Requested pointer alignment in bytes.
   * @return Pointer to the allocated memory, or nullptr when allocation fails.
   */
  virtual void* Allocate(std::size_t size, std::size_t alignment = sizeof(void*)) = 0;

  /**
   * @brief Release a previously allocated block.
   *
   * Semantics are allocator-specific. Some allocators may treat this as a no-op.
   *
   * @param ptr Pointer previously returned by Allocate.
   */
  virtual void Free(void* ptr) = 0;

  /**
   * @brief Reset allocator state back to its initial empty state.
   */
  virtual void Reset() = 0;
};

}  // namespace aegis_alloc
