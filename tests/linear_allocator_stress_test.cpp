#include <gtest/gtest.h>

#include "aegis_alloc/LinearAllocator.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace {

constexpr std::size_t kArenaSizeBytes = 1024ULL * 1024ULL;

[[nodiscard]] bool IsAligned(const void* pointer, std::size_t alignment) {
  const auto raw = reinterpret_cast<std::uintptr_t>(pointer);
  return (raw % static_cast<std::uintptr_t>(alignment)) == static_cast<std::uintptr_t>(0);
}

enum class AllocateResult {
  kSuccess,
  kNullptr,
  kBadAlloc
};

struct Attempt {
  AllocateResult result;
  void* ptr;
};

[[nodiscard]] Attempt TryAllocate(aegis_alloc::LinearAllocator& allocator, std::size_t size, std::size_t alignment) {
  try {
    void* ptr = allocator.Allocate(size, alignment);
    if (ptr == nullptr) {
      return Attempt{AllocateResult::kNullptr, nullptr};
    }
    return Attempt{AllocateResult::kSuccess, ptr};
  } catch (const std::bad_alloc&) {
    return Attempt{AllocateResult::kBadAlloc, nullptr};
  }
}

[[nodiscard]] bool IsNotImplemented(const std::runtime_error& error) {
  return std::strcmp(error.what(), "Not implemented yet") == 0;
}

TEST(LinearAllocatorAlignmentStress, OddSizedAllocationsRespectAlignment8_16_64) {
  alignas(64) std::array<std::byte, kArenaSizeBytes> arena{};

  try {
    aegis_alloc::LinearAllocator allocator(arena.data(), arena.size());

    constexpr std::array<std::size_t, 3> kOddSizes = {3U, 17U, 105U};
    constexpr std::array<std::size_t, 3> kAlignments = {8U, 16U, 64U};
    constexpr std::size_t kIterations = 20000U;

    for (const std::size_t size : kOddSizes) {
      for (const std::size_t alignment : kAlignments) {
        allocator.Reset();
        std::size_t success_count = 0U;
        for (std::size_t i = 0U; i < kIterations; ++i) {
          const Attempt attempt = TryAllocate(allocator, size, alignment);
          if (attempt.result != AllocateResult::kSuccess) {
            FAIL() << "Allocator exhausted before finishing alignment stress case "
                   << "(size=" << size << ", alignment=" << alignment << ", i=" << i << ").";
            break;  // Unreachable after FAIL, keeps control flow explicit.
          }
          ASSERT_NE(attempt.ptr, nullptr);
          EXPECT_TRUE(IsAligned(attempt.ptr, alignment))
            << "size=" << size << ", alignment=" << alignment << ", i=" << i;
          ++success_count;
        }
        EXPECT_EQ(success_count, kIterations) << "Alignment stress case did not complete all iterations.";
      }
    }
  } catch (const std::runtime_error& error) {
    if (IsNotImplemented(error)) {
      GTEST_SKIP() << "LinearAllocator implementation pending.";
    }
    throw;
  }
}

TEST(LinearAllocatorOverflowStress, ExceedingArenaReturnsNullptrOrThrowsBadAlloc) {
  alignas(64) std::array<std::byte, kArenaSizeBytes> arena{};

  try {
    aegis_alloc::LinearAllocator allocator(arena.data(), arena.size());
    allocator.Reset();

    constexpr std::size_t kTooLargeSize = kArenaSizeBytes + 1U;
    constexpr std::size_t kAlignment = 16U;

    void* ptr = allocator.Allocate(kTooLargeSize, kAlignment);
    EXPECT_EQ(ptr, nullptr) << "Overflow allocation is expected to return nullptr.";
  } catch (const std::bad_alloc&) {
    FAIL() << "Overflow allocation must return nullptr instead of throwing std::bad_alloc.";
  } catch (const std::runtime_error& error) {
    if (IsNotImplemented(error)) {
      GTEST_SKIP() << "LinearAllocator implementation pending.";
    }
    throw;
  }
}

TEST(LinearAllocatorPressureStress, HundredThousandAllocationsNeverOverlap) {
  alignas(64) std::array<std::byte, kArenaSizeBytes> arena{};

  try {
    aegis_alloc::LinearAllocator allocator(arena.data(), arena.size());
    allocator.Reset();

    constexpr std::size_t kRequestedAllocations = 100000U;
    constexpr std::size_t kAlignment = 8U;
    std::size_t successful_allocations = 0U;
    std::uintptr_t previous_end = 0U;

    for (std::size_t i = 0U; i < kRequestedAllocations; ++i) {
      const std::size_t size = 1U + ((i * 17U) % 7U);
      const Attempt attempt = TryAllocate(allocator, size, kAlignment);

      if (attempt.result != AllocateResult::kSuccess) {
        FAIL() << "Allocator exhausted before completing 100,000 stress allocations at i=" << i << ".";
        break;  // Unreachable after FAIL, keeps control flow explicit.
      }

      ASSERT_NE(attempt.ptr, nullptr);
      ASSERT_TRUE(IsAligned(attempt.ptr, kAlignment));

      const auto begin = reinterpret_cast<std::uintptr_t>(attempt.ptr);
      const auto end = begin + size;

      if (i > 0U) {
        EXPECT_GE(begin, previous_end) << "Found overlapping allocations in linear arena.";
      }

      previous_end = end;
      ++successful_allocations;
    }

    EXPECT_EQ(successful_allocations, kRequestedAllocations);
  } catch (const std::runtime_error& error) {
    if (IsNotImplemented(error)) {
      GTEST_SKIP() << "LinearAllocator implementation pending.";
    }
    throw;
  }
}

}  // namespace
