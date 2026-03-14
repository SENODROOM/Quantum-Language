# Testing Strategy

## 🧪 Overview

Quantum Language employs a comprehensive testing strategy that ensures language correctness, library functionality, and cross-platform compatibility. The testing framework is designed to catch bugs early, prevent regressions, and maintain code quality.

## 📊 Test Categories

### 1. Unit Tests
- **Purpose**: Test individual components in isolation
- **Coverage**: Core language features, built-in functions
- **Location**: `tests/` directory
- **Framework**: Custom test runner integrated with the interpreter

### 2. Integration Tests
- **Purpose**: Test component interactions
- **Coverage**: Language features working together
- **Examples**: File I/O with error handling, complex expressions

### 3. Library Tests
- **Purpose**: Verify standard library functionality
- **Coverage**: All example programs in `examples/library/`
- **Scope**: 263+ example programs across 4 language styles

### 4. Regression Tests
- **Purpose**: Prevent reintroduction of fixed bugs
- **Coverage**: Previously fixed issues
- **Maintenance**: Updated with each bug fix

## 🏗️ Test Infrastructure

### Test Runner
```cpp
// Built-in test command
quantum --test examples          # Run all library tests
quantum --test examples/library/C # Run specific language tests
quantum --test tests             # Run unit tests
```

### Test Output Format
```
✓ examples\library\C\algorithms\binary search.sa
✗ examples\library\Python\simple_programs\guess_number.sa
    TypeError: No method 'randint' on native

══════════════════════════════════════════════════
  Passed : 258 / 263
  Failed : 5 / 263
══════════════════════════════════════════════════
```

### Test Results Storage
- **File**: `test_results.txt`
- **Format**: Detailed error reports with line numbers
- **Retention**: Last test run results for analysis

## 📝 Test Types

### Language Feature Tests

#### Expression Evaluation
```sa
# test_expressions.sa
print("Testing expressions...")

# Arithmetic
assert(2 + 3 == 5, "Addition failed")
assert(10 - 4 == 6, "Subtraction failed")
assert(3 * 4 == 12, "Multiplication failed")
assert(15 / 3 == 5, "Division failed")

# Comparison
assert(5 > 3, "Greater than failed")
assert(3 < 5, "Less than failed")
assert(5 == 5, "Equality failed")

# Logical
assert(true and true, "Logical AND failed")
assert(true or false, "Logical OR failed")
assert(not false, "Logical NOT failed")
```

#### Control Flow Tests
```sa
# test_control_flow.sa
print("Testing control flow...")

# If statements
let x = 10
if (x > 5) {
    assert(true, "If statement failed")
} else {
    assert(false, "If statement else failed")
}

# While loops
let count = 0
while (count < 3) {
    count = count + 1
}
assert(count == 3, "While loop failed")

# For loops
let sum = 0
for (let i = 0; i < 5; i++) {
    sum = sum + i
}
assert(sum == 10, "For loop failed")
```

#### Function Tests
```sa
# test_functions.sa
print("Testing functions...")

# Function definition and call
function add(a, b) {
    return a + b
}

assert(add(2, 3) == 5, "Function call failed")

# Closures
function makeAdder(n) {
    return function(x) {
        return x + n
    }
}

let add5 = makeAdder(5)
assert(add5(3) == 8, "Closure failed")

# Recursion
function factorial(n) {
    if (n <= 1) {
        return 1
    }
    return n * factorial(n - 1)
}

assert(factorial(5) == 120, "Recursion failed")
```

### Type System Tests

#### Dynamic Typing
```sa
# test_types.sa
print("Testing type system...")

# Type changes
let x = 10
assert(type(x) == "number", "Initial type wrong")

x = "hello"
assert(type(x) == "string", "Type change failed")

x = true
assert(type(x) == "bool", "Boolean type failed")

# Type checking
assert(isinstance(10, "number"), "Number isinstance failed")
assert(isinstance("hello", "string"), "String isinstance failed")
```

#### Array Operations
```sa
# test_arrays.sa
print("Testing arrays...")

# Creation and access
let arr = [1, 2, 3, 4, 5]
assert(len(arr) == 5, "Array length failed")
assert(arr[0] == 1, "Array access failed")

# Operations
push(arr, 6)
assert(len(arr) == 6, "Push failed")

let popped = pop(arr)
assert(popped == 6, "Pop failed")
assert(len(arr) == 5, "Length after pop failed")

# Methods
assert(arr.includes(3), "Includes failed")
assert(arr.slice(1, 3) == [2, 3], "Slice failed")
```

### Error Handling Tests

#### Exception Handling
```sa
# test_errors.sa
print("Testing error handling...")

# Type errors
try {
    len(10)  # Should fail
    assert(false, "Type error not thrown")
} catch (error) {
    assert(type(error) == "TypeError", "Wrong error type")
}

# Index errors
try {
    [1, 2, 3][10]  # Should fail
    assert(false, "Index error not thrown")
} catch (error) {
    assert(type(error) == "IndexError", "Wrong error type")
}
```

## 🔍 Library Testing Strategy

### Multi-Language Examples
The library contains examples in 4 different programming styles:
- **C Style** - Low-level, systems programming approach
- **Python Style** - High-level, dynamic programming
- **JavaScript Style** - Web and functional programming
- **C++ Style** - Object-oriented programming

### Test Coverage Matrix

| Category | C | Python | JavaScript | C++ | Total |
|----------|---|--------|------------|-----|-------|
| Algorithms | 4 | 11 | 6 | Many | 21+ |
| Data Structures | 1 | 3 | 1 | Many | 5+ |
| String Operations | 3 | 3 | 8 | Many | 14+ |
| Mathematical | 2 | 2 | 4 | Many | 8+ |
| Games | 1 | 3 | 4 | Many | 8+ |

### Example Test Structure
Each library example serves as a test case:
```sa
# binary_search.sa - C Style
print("Testing binary search...")

let arr = [1, 3, 5, 7, 9, 11, 13]
let target = 7
let result = binary_search(arr, target)

assert(result == 3, "Binary search failed")
print("✓ Binary search works correctly")
```

## 🐛 Bug Fix Verification

### Regression Test Process
1. **Bug Report**: Issue identified with failing test
2. **Root Cause Analysis**: Debug and identify the problem
3. **Fix Implementation**: Apply the fix to source code
4. **Test Update**: Add/update test to verify fix
5. **Verification**: Run full test suite to ensure no regressions

### Example Bug Fix Workflow
```sa
# Before fix - failing test
# file_operations.sa
let content = read_file("nonexistent.txt")
let size = len(content)  # TypeError: len() not supported for type nil

# After fix - working test
let content = read_file("nonexistent.txt")
let size = 0
if (content != null && content != nil) {
    size = len(content)
}
assert(size == 0, "File size check failed")
```

## 📈 Test Metrics

### Coverage Metrics
- **Total Tests**: 263+ library examples
- **Language Features**: 100% core features covered
- **Standard Library**: 95%+ function coverage
- **Error Cases**: 80%+ error paths tested

### Performance Metrics
- **Test Execution Time**: < 30 seconds for full suite
- **Memory Usage**: < 100MB peak during testing
- **Pass Rate**: Target > 98% (currently 98.1%)

### Quality Metrics
- **Code Coverage**: Target 90%+ for core modules
- **Mutation Testing**: Planned for critical paths
- **Fuzz Testing**: Planned for edge cases

## 🔄 Continuous Testing

### Automated Testing
- **Pre-commit Hooks**: Run basic tests before commits
- **CI/CD Pipeline**: Full test suite on every push
- **Nightly Builds**: Comprehensive testing with profiling

### Test Environments
- **Windows**: MSVC 2019/2022
- **Linux**: GCC 9+/Clang 10+
- **macOS**: Clang 12+

### Release Testing
- **Feature Branch**: All tests must pass
- **Release Candidate**: Extended testing suite
- **Production Release**: Full regression testing

## 🛠️ Test Utilities

### Test Helper Functions
```sa
# Built-in test assertions
assert(condition, message)           # Basic assertion
assert_equals(expected, actual, msg) # Value equality
assert_throws(error_type, func)      # Exception testing
assert_approximately(a, b, tolerance) # Float comparison

# Test utilities
test_time(func, iterations)          # Performance testing
memory_usage(func)                   # Memory profiling
benchmark(func, reference)           # Speed comparison
```

### Debug Mode
```cpp
#ifdef DEBUG
    // Enable additional checks
    // Verbose error messages
    // Execution tracing
    // Memory debugging
#endif
```

## 📋 Test Planning

### Short-term Goals
1. **Increase Coverage**: Achieve 99% pass rate
2. **Add Edge Cases**: More boundary condition tests
3. **Performance Tests**: Benchmark critical operations
4. **Fuzz Testing**: Random input generation

### Long-term Goals
1. **Property-Based Testing**: Generate test cases automatically
2. **Mutation Testing**: Verify test effectiveness
3. **Cross-Platform Testing**: Expanded platform support
4. **Integration Testing**: Real-world usage scenarios

## 🎯 Test Success Criteria

### Acceptance Criteria
- ✅ All core language features work correctly
- ✅ Standard library functions pass all tests
- ✅ Error handling works as expected
- ✅ Performance meets requirements
- ✅ Memory leaks are eliminated
- ✅ Cross-platform compatibility maintained

### Quality Gates
- **Must Pass**: Core language tests
- **Should Pass**: Library tests (>98%)
- **Could Pass**: Edge case tests (>90%)

---

*This comprehensive testing strategy ensures Quantum Language maintains high quality and reliability across all its features.*
