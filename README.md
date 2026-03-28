# Aegis-Alloc

Aegis-Alloc is a C++20 custom memory allocator library scaffold for high-performance runtime systems such as game engines.

## Why game engines need custom allocators

General-purpose allocators are designed for broad workloads, not frame-budget determinism. Real-time engines care about:

- **Predictability**: avoid sporadic stalls from global heap contention or expensive allocation paths.
- **Cache efficiency**: pack frequently-accessed data tightly to increase spatial locality.
- **Fragmentation control**: reserve memory arenas by subsystem (rendering, animation, ECS, scripting).
- **Debuggability**: track ownership and lifetimes with allocator boundaries and markers.

Custom allocators let you define memory behavior per system instead of relying on opaque heap internals.

## DOD and cache hit rate

Data-Oriented Design (DOD) focuses on arranging data by access pattern rather than by object hierarchy.  
When data used together is stored contiguously:

- CPU cache lines contain more useful bytes.
- Hardware prefetchers become more effective.
- Branch and TLB pressure drops.
- Throughput improves under tight frame budgets.

Allocator strategy directly affects DOD outcomes: linear/stack arenas naturally support contiguous, short-lived batches and thus can dramatically improve cache hit rates.

## Project structure

```text
Aegis-Alloc/
├── CMakeLists.txt
├── Makefile
├── docs/
│   └── allocator_design.md
├── include/
│   └── aegis_alloc/
│       ├── Allocator.h
│       ├── LinearAllocator.h
│       └── StackAllocator.h
├── src/
│   ├── LinearAllocator.cpp
│   └── StackAllocator.cpp
└── tests/
    ├── sanity_test.cpp
    └── linear_allocator_stress_test.cpp
```

## Build and test

Requirements:
- GCC with C++20 support
- CMake >= 3.20

Strict warning profile enabled:
- `-Wall -Wextra -Werror -Wpedantic -Wconversion`

One command:

```bash
make test
```

This configures, builds, and runs all tests via CTest.

## Current status

- Allocator interfaces are declared.
- `LinearAllocator` and `StackAllocator` source files are intentionally placeholders.
- Methods currently throw `std::runtime_error("Not implemented yet")`.
- Stress tests are implemented and will automatically run once allocator logic is implemented.

## Task list (next steps)

- [ ] Implement alignment helper utilities (power-of-two validation, padding computation).
- [ ] Implement `LinearAllocator::Allocate` with strict alignment guarantees.
- [ ] Implement `LinearAllocator` exhaustion behavior contract (choose: `nullptr` or `std::bad_alloc`).
- [ ] Implement `LinearAllocator::Reset` and optional telemetry (`peak usage`, `allocation count`).
- [ ] Implement `StackAllocator` marker semantics (`GetMarker`, `Rollback`, LIFO safety checks).
- [ ] Add debug-mode guard rails (canaries, bounds checks, double-free misuse detection where applicable).
- [ ] Add benchmark suite comparing system allocator vs. arena allocators under representative workloads.
- [ ] Add CI matrix for GCC/Clang and sanitizer profiles (ASan/UBSan/TSan).
