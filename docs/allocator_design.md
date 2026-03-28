# Aegis-Alloc Design Notes

## High-level goals

- Provide deterministic allocator behavior for frame-based and job-based workflows.
- Minimize runtime allocator overhead and branch unpredictability.
- Keep APIs explicit about alignment and failure semantics.

## Planned allocator families

- `LinearAllocator`: monotonic bump-pointer allocator for short-lived bursts.
- `StackAllocator`: LIFO allocator with marker/rollback semantics.
- Future candidates: pool allocator, slab allocator, TLSF-style fallback allocator.

## Integration direction

- `Allocator` base class defines common polymorphic API expected by engine subsystems.
- Concrete allocators own no external memory by default; they operate over caller-owned arenas.
- Profiling hooks and telemetry can be introduced behind compile-time flags later.
