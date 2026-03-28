#include "aegis_alloc/StackAllocator.h"

#include <stdexcept>

namespace aegis_alloc {

StackAllocator::StackAllocator(void* arena, std::size_t arena_size) : arena_(static_cast<std::byte*>(arena)), arena_size_(arena_size) {
  throw std::runtime_error("Not implemented yet");
}

void* StackAllocator::Allocate(std::size_t size, std::size_t alignment) {
  static_cast<void>(size);
  static_cast<void>(alignment);
  throw std::runtime_error("Not implemented yet");
}

void StackAllocator::Free(void* ptr) {
  static_cast<void>(ptr);
  throw std::runtime_error("Not implemented yet");
}

void StackAllocator::Reset() {
  throw std::runtime_error("Not implemented yet");
}

StackAllocator::Marker StackAllocator::GetMarker() const {
  throw std::runtime_error("Not implemented yet");
}

void StackAllocator::Rollback(Marker marker) {
  static_cast<void>(marker);
  throw std::runtime_error("Not implemented yet");
}

std::size_t StackAllocator::GetCapacity() const {
  throw std::runtime_error("Not implemented yet");
}

}  // namespace aegis_alloc
