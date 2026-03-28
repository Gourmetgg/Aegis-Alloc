#include <gtest/gtest.h>

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>

#if __has_include("linear_allocator.hpp")
#include "linear_allocator.hpp"
#define AEGIS_LINEAR_ALLOCATOR_HEADER_FOUND 1
#elif __has_include("aegis/linear_allocator.hpp")
#include "aegis/linear_allocator.hpp"
#define AEGIS_LINEAR_ALLOCATOR_HEADER_FOUND 1
#else
#define AEGIS_LINEAR_ALLOCATOR_HEADER_FOUND 0
#endif

namespace {

constexpr std::size_t kOneMiB = 1024ULL * 1024ULL;

#if AEGIS_LINEAR_ALLOCATOR_HEADER_FOUND

#ifndef AEGIS_LINEAR_ALLOCATOR_TYPE
#define AEGIS_LINEAR_ALLOCATOR_TYPE LinearAllocator
#endif

using LinearAllocatorUnderTest = AEGIS_LINEAR_ALLOCATOR_TYPE;

template <typename AllocatorT>
concept SupportsAllocateLower = requires(AllocatorT& allocator, std::size_t bytes, std::size_t alignment) {
  { allocator.allocate(bytes, alignment) } -> std::convertible_to<void*>;
};

template <typename AllocatorT>
concept SupportsAllocateUpper = requires(AllocatorT& allocator, std::size_t bytes, std::size_t alignment) {
  { allocator.Allocate(bytes, alignment) } -> std::convertible_to<void*>;
};

template <typename T>
inline constexpr bool kAlwaysFalse = false;

template <typename AllocatorT>
[[nodiscard]] AllocatorT MakeAllocator(std::byte* buffer, std::size_t size_in_bytes) {
  if constexpr (std::is_constructible_v<AllocatorT, std::byte*, std::size_t>) {
    return AllocatorT{buffer, size_in_bytes};
  } else if constexpr (std::is_constructible_v<AllocatorT, void*, std::size_t>) {
    return AllocatorT{static_cast<void*>(buffer), size_in_bytes};
  } else if constexpr (std::is_constructible_v<AllocatorT, char*, std::size_t>) {
    return AllocatorT{reinterpret_cast<char*>(buffer), size_in_bytes};
  } else {
    static_assert(
      kAlwaysFalse<AllocatorT>,
      "LinearAllocator must be constructible from (buffer_ptr, size_in_bytes)."
    );
  }
}

template <typename AllocatorT>
[[nodiscard]] void* InvokeAllocate(AllocatorT& allocator, std::size_t bytes, std::size_t alignment) {
  if constexpr (SupportsAllocateLower<AllocatorT>) {
    return static_cast<void*>(allocator.allocate(bytes, alignment));
  } else if constexpr (SupportsAllocateUpper<AllocatorT>) {
    return static_cast<void*>(allocator.Allocate(bytes, alignment));
  } else {
    static_assert(
      kAlwaysFalse<AllocatorT>,
      "LinearAllocator must expose allocate(size_t, size_t) or Allocate(size_t, size_t)."
    );
  }
}

enum class AllocationOutcome {
  kSuccess,
  kNullptr,
  kBadAlloc,
  kUnexpectedException,
};

struct AllocationAttempt {
  AllocationOutcome outcome;
  void* pointer;
};

[[nodiscard]] bool IsAligned(const void* pointer, std::size_t alignment) {
  const auto address = reinterpret_cast<std::uintptr_t>(pointer);
  return (address % static_cast<std::uintptr_t>(alignment)) == static_cast<std::uintptr_t>(0);
}

template <typename AllocatorT>
[[nodiscard]] AllocationAttempt TryAllocate(AllocatorT& allocator, std::size_t bytes, std::size_t alignment) noexcept {
  try {
    void* ptr = InvokeAllocate(allocator, bytes, alignment);
    if (ptr == nullptr) {
      return AllocationAttempt{AllocationOutcome::kNullptr, nullptr};
    }
    return AllocationAttempt{AllocationOutcome::kSuccess, ptr};
  } catch (const std::bad_alloc&) {
    return AllocationAttempt{AllocationOutcome::kBadAlloc, nullptr};
  } catch (...) {
    return AllocationAttempt{AllocationOutcome::kUnexpectedException, nullptr};
  }
}

TEST(LinearAllocatorAlignmentStress, SmallOddRequestsHonorExplicitAlignmentUntilExhausted) {
  constexpr std::array<std::size_t, 3> kRequestSizes = {3U, 17U, 105U};
  constexpr std::array<std::size_t, 3> kAlignments = {8U, 16U, 64U};
  constexpr std::size_t kHardCapAllocationsPerCase = 400000U;

  for (const std::size_t request_size : kRequestSizes) {
    for (const std::size_t alignment : kAlignments) {
      alignas(64) std::array<std::byte, kOneMiB> backing_buffer{};
      auto allocator = MakeAllocator<LinearAllocatorUnderTest>(backing_buffer.data(), backing_buffer.size());

      std::size_t success_count = 0U;
      bool saw_exhaustion_signal = false;

      for (std::size_t i = 0U; i < kHardCapAllocationsPerCase; ++i) {
        const AllocationAttempt attempt = TryAllocate(allocator, request_size, alignment);

        if (attempt.outcome == AllocationOutcome::kSuccess) {
          ASSERT_NE(attempt.pointer, nullptr);
          EXPECT_TRUE(IsAligned(attempt.pointer, alignment))
            << "size=" << request_size << ", alignment=" << alignment;
          ++success_count;
          continue;
        }

        ASSERT_NE(attempt.outcome, AllocationOutcome::kUnexpectedException)
          << "Only std::bad_alloc or nullptr are valid exhaustion signals.";
        ASSERT_TRUE(
          attempt.outcome == AllocationOutcome::kBadAlloc || attempt.outcome == AllocationOutcome::kNullptr
        );
        saw_exhaustion_signal = true;
        break;
      }

      EXPECT_GT(success_count, 0U) << "Allocator failed before serving any request.";
      EXPECT_TRUE(saw_exhaustion_signal)
        << "1MB buffer should eventually exhaust under sustained allocations.";
    }
  }
}

TEST(LinearAllocatorExhaustionStress, ReportsBadAllocOrNullptrWhenBufferIsDepleted) {
  constexpr std::size_t kRequestSize = 4096U;
  constexpr std::size_t kAlignment = 64U;
  constexpr std::size_t kHardCapAttempts = 16384U;

  alignas(64) std::array<std::byte, kOneMiB> backing_buffer{};
  auto allocator = MakeAllocator<LinearAllocatorUnderTest>(backing_buffer.data(), backing_buffer.size());

  std::size_t success_count = 0U;
  AllocationOutcome first_failure = AllocationOutcome::kSuccess;

  for (std::size_t i = 0U; i < kHardCapAttempts; ++i) {
    const AllocationAttempt attempt = TryAllocate(allocator, kRequestSize, kAlignment);
    if (attempt.outcome == AllocationOutcome::kSuccess) {
      ASSERT_NE(attempt.pointer, nullptr);
      EXPECT_TRUE(IsAligned(attempt.pointer, kAlignment));
      ++success_count;
      continue;
    }

    ASSERT_NE(attempt.outcome, AllocationOutcome::kUnexpectedException)
      << "Exhaustion must not raise unrelated exception types.";
    first_failure = attempt.outcome;
    break;
  }

  ASSERT_GT(success_count, 0U) << "No successful allocations happened before exhaustion.";
  ASSERT_TRUE(first_failure == AllocationOutcome::kBadAlloc || first_failure == AllocationOutcome::kNullptr)
    << "Exhaustion must throw std::bad_alloc or return nullptr.";

  for (std::size_t i = 0U; i < 8U; ++i) {
    const AllocationAttempt attempt = TryAllocate(allocator, kRequestSize, kAlignment);
    EXPECT_NE(attempt.outcome, AllocationOutcome::kUnexpectedException);
    EXPECT_TRUE(attempt.outcome == AllocationOutcome::kBadAlloc || attempt.outcome == AllocationOutcome::kNullptr)
      << "Post-exhaustion allocations must continue to fail with bad_alloc or nullptr.";
  }
}

#else

TEST(LinearAllocatorContract, WaitingForLinearAllocatorHeaderAndType) {
  GTEST_SKIP() << "Add linear_allocator.hpp (or aegis/linear_allocator.hpp) with a LinearAllocator type. "
                  "Expected API: ctor(buffer_ptr, size_in_bytes) + allocate(size, alignment).";
}

#endif

}  // namespace
