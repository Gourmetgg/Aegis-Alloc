#include "aegis_alloc/LinearAllocator.h"

#include <stdexcept>

namespace aegis_alloc {

LinearAllocator::LinearAllocator(void* arena, std::size_t arena_size)
    : arena_(static_cast<std::byte*>(arena)), arena_size_(arena_size) {
  throw std::runtime_error("Not implemented yet");
}

void* LinearAllocator::Allocate(std::size_t size, std::size_t alignment) {
  static_cast<void>(size);
  static_cast<void>(alignment);
  throw std::runtime_error("Not implemented yet");
}

void LinearAllocator::Free(void* ptr) {
  static_cast<void>(ptr);
  throw std::runtime_error("Not implemented yet");
}

void LinearAllocator::Reset() {
  throw std::runtime_error("Not implemented yet");
}

std::size_t LinearAllocator::GetOffset() const {
  throw std::runtime_error("Not implemented yet");
}

std::size_t LinearAllocator::GetCapacity() const {
  throw std::runtime_error("Not implemented yet");
}

}  // namespace aegis_alloc
