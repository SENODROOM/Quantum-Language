# Quantum Language Specification

## 📖 Language Overview

Quantum Language is a dynamically-typed, multi-syntax scripting language that combines elements from Python, JavaScript, and C++. It supports functional, object-oriented, and procedural programming paradigms.

## 🔤 Lexical Structure

### File Extension
- **`.sa`** - Quantum Language source files

### Character Set
- Unicode support (UTF-8 encoding)
- Case-sensitive language
- Whitespace significant in some contexts (indentation optional)

### Keywords
```
# Control Flow
if, else, while, for, in, break, continue, return

# Declarations
let, var, const, function, class, import

# Types & Values
true, false, nil, null, undefined

# Logical Operators
and, or, not

# Cybersecurity (reserved)
scan, payload, encrypt, decrypt, hash
```

### Identifiers
- **Pattern**: `[a-zA-Z_][a-zA-Z0-9_]*`
- **Case-sensitive**: `variable` ≠ `Variable`
- **Reserved words**: Cannot use keywords as identifiers

### Comments
```sa
# Single line comment (Python/Shell style)

/*
 Multi-line comment
 (C/JavaScript style)
*/

/**
 Documentation comment
 (JSDoc style)
*/
```

### Literals

#### Number Literals
```sa
42           # Integer
3.14159      # Float
1.23e-4      # Scientific notation
0xFF         # Hexadecimal
0b1010       # Binary
0o52         # Octal
```

#### String Literals
```sa
"Hello World"          # Double quotes
'Single quotes'         # Single quotes
`Template literals`     # Backticks (template strings)
"Line 1\nLine 2"        # Escape sequences
"Raw string \n"         # Raw strings (r"")
```

#### Boolean & Null Literals
```sa
true, false            # Boolean
nil, null, undefined   # Null/undefined values
```

## 🏗️ Data Types

### Primitive Types
- **`number`** - 64-bit floating point (IEEE 754)
- **`string`** - Unicode text sequences
- **`bool`** - True/false values
- **`nil`** - Null/undefined value

### Composite Types
- **`array`** - Ordered list of values (dynamic size)
- **`dict`** - Key-value pairs (string keys)
- **`function`** - First-class functions
- **`class`** - Object templates
- **`instance`** - Class instances

### Advanced Types
- **`pointer`** - C-style memory references
- **`native`** - Built-in function references

## 📐 Syntax Variants

### Variable Declarations
```sa
# Python style
x = 10
name = "Quantum"

# JavaScript style
let x = 10
var name = "Quantum"
const PI = 3.14159

# C++ style
int x = 10          # Type annotation (optional)
string name = "Quantum"
```

### Function Declarations
```sa
# Function statement
function add(a, b) {
    return a + b
}

# Function expression
let add = function(a, b) {
    return a + b
}

# Arrow function (JavaScript)
let add = (a, b) => a + b

# Lambda (Python)
add = lambda a, b: a + b
```

### Control Flow
```sa
# If statement
if (condition) {
    # code
} else if (other) {
    # code
} else {
    # code
}

# While loop
while (condition) {
    # code
}

# For loop
for (let i = 0; i < 10; i++) {
    # code
}

# For-each loop
for (item in array) {
    # code
}
```

## 🔧 Operators

### Arithmetic Operators
```
+   Addition
-   Subtraction
*   Multiplication
/   Division
%   Modulo
**  Exponentiation
```

### Comparison Operators
```
==  Equal
!=  Not equal
=== Strict equal
!== Strict not equal
<   Less than
>   Greater than
<=  Less than or equal
>=  Greater than or equal
```

### Logical Operators
```
and Logical AND
or  Logical OR
not Logical NOT
&&  Bitwise AND (also logical)
||  Bitwise OR (also logical)
!   Bitwise NOT (also logical)
```

### Assignment Operators
```
=   Simple assignment
+=  Addition assignment
-=  Subtraction assignment
*=  Multiplication assignment
/=  Division assignment
%=  Modulo assignment
```

### Special Operators
```
->  Arrow operator (C++ pointers)
&   Address of
*   Dereference
.   Member access
[]  Array/object access
```

## 🎯 Expressions

### Primary Expressions
```sa
42                    # Number literal
"hello"              # String literal
true                 # Boolean literal
nil                  # Null literal
variable_name        # Variable reference
```

### Unary Expressions
```sa
-x                    # Negation
not x                # Logical NOT
!x                   # Bitwise NOT
++x, x++             # Increment/decrement
&x                   # Address of
*x                   # Dereference
```

### Binary Expressions
```sa
x + y                # Addition
x - y                # Subtraction
x * y                # Multiplication
x / y                # Division
x % y                # Modulo
x ** y               # Exponentiation
x and y              # Logical AND
x or y               # Logical OR
x == y               # Equality
x != y               # Inequality
x < y                # Less than
```

### Function Calls
```sa
func()               # No arguments
func(arg1, arg2)     # Positional arguments
obj.method()         # Method call
array[index]         # Index access
obj.property         # Property access
```

### Array & Object Literals
```sa
[]                   # Empty array
[1, 2, 3]           # Array with elements
{}                   # Empty object
{key: value}         # Object with property
```

## 🔄 Statements

### Declaration Statements
```sa
let x = 10           # Variable declaration
const PI = 3.14      # Constant declaration
function f() {}      # Function declaration
class C {}          # Class declaration
```

### Control Statements
```sa
if (condition) {}    # Conditional
while (condition) {} # Loop
for (init; cond; inc) {} # For loop
break               # Exit loop
continue            # Skip iteration
return value        # Return from function
```

### Expression Statements
```sa
x = 10              # Assignment
func()              # Function call
print("hello")      # Built-in function
```

## 🏗️ Functions

### Function Definition
```sa
function name(param1, param2) {
    # Function body
    return result
}

# With type annotations (optional)
function add(number a, number b) -> number {
    return a + b
}
```

### Parameters
```sa
# Regular parameters
function func(a, b, c) {}

# Default parameters
function func(a = 10, b = "default") {}

# Rest parameters (variadic)
function func(...args) {}

# Parameter destructuring
function func({x, y}) {}
```

### Closures & Lambdas
```sa
# Arrow function
let add = (a, b) => a + b

# Function expression
let multiply = function(a, b) { return a * b }

# Closure
function makeAdder(n) {
    return function(x) { return x + n }
}
```

## 🏛️ Object-Oriented Programming

### Class Definition
```sa
class Person {
    # Constructor
    constructor(name, age) {
        this.name = name
        this.age = age
    }
    
    # Method
    greet() {
        return "Hello, I'm " + this.name
    }
    
    # Static method
    static createAdult(name) {
        return new Person(name, 18)
    }
}
```

### Inheritance
```sa
class Student extends Person {
    constructor(name, age, grade) {
        super(name, age)  # Call parent constructor
        this.grade = grade
    }
    
    study() {
        return this.name + " is studying"
    }
}
```

### Object Creation
```sa
let person = new Person("Alice", 25)
let student = new Student("Bob", 20, "A")

# Property access
person.name
student.grade

# Method calls
person.greet()
student.study()
```

## 🌊 Advanced Features

### Array Comprehensions
```sa
# List comprehension
squares = [x * x for x in range(10)]
evens = [x for x in range(20) if x % 2 == 0]

# Dictionary comprehension
squares_dict = {x: x * x for x in range(5)}
```

### Slicing
```sa
arr = [0, 1, 2, 3, 4, 5]

# Python-style slicing
arr[1:4]      # [1, 2, 3]
arr[:3]       # [0, 1, 2]
arr[2:]       # [2, 3, 4, 5]
arr[::2]      # [0, 2, 4]
arr[::-1]     # [5, 4, 3, 2, 1, 0]
```

### Template Strings
```sa
name = "World"
message = `Hello, ${name}!`
result = `2 + 2 = ${2 + 2}`
```

### Destructuring
```sa
# Array destructuring
let [a, b, c] = [1, 2, 3]

# Object destructuring
let {name, age} = person
let {name: n, age: a} = person  # With renaming
```

## 🔌 Built-in Functions

### I/O Functions
```sa
print(arg1, arg2, ...)    # Output to console
input(prompt)             # Get user input
read_file(filename)       # Read file contents
write_file(filename, content)  # Write to file
```

### Type Functions
```sa
type(value)               # Get type name
str(value)                # Convert to string
parseInt(string)          # Parse integer
parseFloat(string)        # Parse float
```

### Array Functions
```sa
len(array)                # Get length
push(array, item)         # Add item to end
pop(array)                # Remove from end
slice(array, start, end)  # Get slice
```

### Mathematical Functions
```sa
abs(x)                    # Absolute value
sqrt(x)                   # Square root
pow(x, y)                 # Power
sin(x), cos(x), tan(x)    # Trigonometric
log(x), exp(x)            # Logarithmic
random()                  # Random number
```

## 🛡️ Cybersecurity Features (Reserved)

These keywords are reserved for future cybersecurity functionality:

```sa
# Network operations (planned)
scan(target, options)     # Network scanning
connect(host, port)       # Network connection

# Cryptographic operations (planned)
encrypt(data, algorithm)  # Encryption
decrypt(data, key)        # Decryption
hash(data, algorithm)     # Hashing
generate_key(algorithm)   # Key generation

# Payload operations (planned)
payload(type, options)     # Exploit payload creation
execute(payload)          # Payload execution
```

---

*This specification defines the complete syntax and semantics of Quantum Language v1.0.0.*
