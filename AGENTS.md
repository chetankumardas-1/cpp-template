
# C++ Project Instructions

## 1. Project Overview

- Language: C++20
- Platform: Ubuntu 24.04 under WSL2
- Compiler: GCC 13
- Build system: CMake
- Build generator: Ninja
- Debugger: GDB
- Unit testing: GoogleTest and CTest
- Runtime analysis: AddressSanitizer and UBSan

## 2. Coding Standards

- Use modern C++20.
- Prefer RAII and deterministic resource management.
- Follow the Rule of Zero wherever possible.
- Use smart pointers for explicit ownership.
- Avoid raw owning pointers.
- Prefer const correctness.
- Avoid unnecessary copies.
- Use move semantics when appropriate.
- Prefer standard library facilities over custom implementations.
- Write readable, maintainable and efficient code.
- Avoid unnecessary abstractions and overengineering.

## 3. Memory Management

- Prevent memory leaks, dangling pointers and use-after-free.
- Consider object lifetime and ownership carefully.
- Use unique_ptr for exclusive ownership.
- Use shared_ptr only when shared ownership is required.
- Use weak_ptr to break shared ownership cycles.
- Check allocation and resource-management paths.

## 4. Multithreading

- Avoid data races and undefined behavior.
- Use mutexes, condition variables and atomics appropriately.
- Prefer RAII-based locking.
- Avoid unnecessary locking and deadlocks.
- Explain thread-safety assumptions.
- Consider synchronization and object lifetime.

## 5. CMake and Build

- Keep CMake configuration maintainable.
- Use target-based include directories and dependencies.
- Do not introduce global compiler flags unnecessarily.
- Preserve the existing C++20 configuration.
- Preserve ASan and UBSan support in Debug builds.
- Do not change compiler or generator without approval.

Build command:

cmake --build build --parallel

Configure command:

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

## 6. Unit Testing

- Use GoogleTest for unit tests.
- Add tests for new functionality.
- Cover normal cases, boundary cases and failure cases.
- Do not modify existing tests merely to make implementation pass.
- Run CTest after relevant changes.

Test command:

ctest --test-dir build --output-on-failure

## 7. Debugging and Analysis

- Investigate root causes instead of applying superficial fixes.
- Use GDB when debugging runtime issues.
- Consider ASan and UBSan reports carefully.
- Explain the cause of bugs and the reasoning behind fixes.
- Never hide compiler warnings or sanitizer failures.

## 8. Change Management

Before implementing a task:

1. Inspect relevant source files.
2. Understand existing architecture.
3. Explain the proposed approach.
4. Identify affected files.

While implementing:

- Make focused changes.
- Avoid unrelated modifications.
- Preserve existing behavior unless explicitly requested.
- Do not delete or overwrite user work.
- Ask before destructive operations or changes outside the workspace.

After implementation:

1. Summarize modified files.
2. Explain important design decisions.
3. Build the project.
4. Run relevant unit tests.
5. Report actual command results.
6. Clearly mention anything not verified.

## 9. Engineering Expectations

Act as a senior C++ engineer.

Prioritize:

- Correctness
- Memory safety
- Thread safety
- Performance
- Maintainability
- Testability
- Clear architecture

Explain important trade-offs rather than blindly implementing code.
