# Standard Library Documentation

## 📚 Overview

Quantum Language provides a comprehensive standard library with built-in functions and methods that cover I/O operations, mathematical computations, string manipulation, data structure operations, and cybersecurity utilities.

## 🔤 String Functions

### Core String Operations
```sa
len(string)              # Returns string length
str(value)               # Converts value to string
concat(str1, str2, ...)  # Concatenates strings
```

### String Methods
```sa
"hello".len()            # 5
"hello".upper()          # "HELLO"
"hello".lower()          # "hello"
"hello".capitalize()     # "Hello"
"hello".trim()           # "hello"
"hello".ltrim()          # "hello"
"hello".rtrim()          # "hello"

# Substring operations
"hello".slice(1, 4)      # "ell"
"hello".substring(1, 4)  # "ell"
"hello".substr(1, 3)     # "ell"

# Search operations
"hello".index("e")       # 1
"hello".find("e")        # 1
"hello".rfind("l")       # 3
"hello".contains("e")    # true
"hello".startswith("he") # true
"hello".endswith("lo")   # true

# Character operations
"hello".charAt(1)        # "e"
"hello".charCodeAt(1)    # 101
"hello".at(1)            # "e"

# Case operations
"hello".toLowerCase()    # "hello"
"hello".toUpperCase()    # "HELLO"

# Padding operations
"5".padStart(3, "0")     # "005"
"5".padEnd(3, "0")       # "500"

# Splitting and joining
"a,b,c".split(",")       # ["a", "b", "c"]
["a", "b", "c"].join(",") # "a,b,c"
```

### Advanced String Operations
```sa
# Pattern matching
"hello".match(pattern)   # Returns match object
"hello".search(regex)     # Returns first match position
"hello".replace(old, new) # Replace all occurrences
"hello".replaceAll(old, new) # Replace all occurrences

# Unicode operations
"café".normalize()       # Unicode normalization
"hello".codePoints()     # Array of code points
"hello".fromCodePoints(codes) # String from code points

# Encoding operations
"hello".encode("utf8")   # Get bytes
bytes.decode("utf8")     # Convert bytes to string
```

## 🔢 Mathematical Functions

### Basic Arithmetic
```sa
abs(x)                   # Absolute value
sign(x)                  # Sign of number (-1, 0, 1)
ceil(x)                  # Round up
floor(x)                 # Round down
round(x)                 # Round to nearest
trunc(x)                 # Truncate decimal part
```

### Power and Logarithmic
```sa
pow(x, y)                # x raised to power y
sqrt(x)                  # Square root
cbrt(x)                  # Cube root
exp(x)                   # e raised to power x
log(x)                   # Natural logarithm
log10(x)                 # Base-10 logarithm
log2(x)                  # Base-2 logarithm
```

### Trigonometric
```sa
sin(x)                   # Sine
cos(x)                   # Cosine
tan(x)                   # Tangent
asin(x)                  # Arc sine
acos(x)                  # Arc cosine
atan(x)                  # Arc tangent
atan2(y, x)              # Arc tangent of y/x
sinh(x)                  # Hyperbolic sine
cosh(x)                  # Hyperbolic cosine
tanh(x)                  # Hyperbolic tangent
```

### Constants
```sa
PI                       # 3.141592653589793
E                        # 2.718281828459045
TAU                      # 6.283185307179586
INFINITY                 # Positive infinity
NaN                      # Not a Number
```

### Random Numbers
```sa
random()                 # Random float [0, 1)
randint(min, max)        # Random integer [min, max]
randchoice(array)        # Random element from array
shuffle(array)           # Shuffle array in place
```

### Statistics
```sa
min(values)              # Minimum value
max(values)              # Maximum value
sum(values)              # Sum of values
mean(values)             # Arithmetic mean
median(values)           # Median value
mode(values)             # Most frequent value
stddev(values)           # Standard deviation
variance(values)         # Variance
```

## 📊 Array Functions

### Basic Operations
```sa
len(array)               # Array length
push(array, item)        # Add item to end
pop(array)               # Remove from end
shift(array)             # Remove from beginning
unshift(array, item)     # Add to beginning
```

### Array Methods
```sa
[1, 2, 3].slice(1, 3)    # [2, 3]
[1, 2, 3].splice(1, 2)   # Remove elements
[1, 2, 3].concat([4, 5]) # [1, 2, 3, 4, 5]
[1, 2, 3].join(",")      # "1,2,3"
[1, 2, 3].reverse()      # [3, 2, 1]
[1, 2, 3].sort()         # Sort array
```

### Searching and Filtering
```sa
[1, 2, 3].indexOf(2)     # 1
[1, 2, 3].lastIndexOf(2) # 1
[1, 2, 3].includes(2)    # true
[1, 2, 3].find(x => x > 1) # 2
[1, 2, 3].filter(x => x > 1) # [2, 3]
[1, 2, 3].map(x => x * 2) # [2, 4, 6]
```

### Reduction and Transformation
```sa
[1, 2, 3].reduce((a, b) => a + b) # 6
[1, 2, 3].every(x => x > 0) # true
[1, 2, 3].some(x => x > 2) # true
[1, 2, 3].fill(0)         # [0, 0, 0]
```

## 🗝️ Dictionary (Object) Functions

### Basic Operations
```sa
len(dict)                # Number of keys
keys(dict)               # Array of keys
values(dict)             # Array of values
items(dict)              # Array of [key, value] pairs
```

### Dictionary Methods
```sa
obj.has(key)             # Check if key exists
obj.get(key, default)    # Get value with default
obj.set(key, value)      # Set value
obj.delete(key)          # Remove key
obj.clear()              # Remove all keys
obj.copy()               # Create copy
```

### Merging and Updating
```sa
obj1.merge(obj2)         # Merge objects
obj1.update(obj2)        # Update in place
obj.fromKeys(keys, value) # Create from keys
```

## 📁 File I/O Functions

### File Operations
```sa
read_file(filename)      # Read file contents
write_file(filename, content, append=false) # Write to file
file_exists(filename)   # Check if file exists
file_size(filename)     # Get file size
delete_file(filename)   # Delete file
copy_file(src, dst)     # Copy file
move_file(src, dst)     # Move file
```

### Directory Operations
```sa
mkdir(path)              # Create directory
rmdir(path)              # Remove directory
list_dir(path)           # List directory contents
is_dir(path)             # Check if path is directory
is_file(path)            # Check if path is file
```

### Path Operations
```sa
path_join(parts...)      # Join path components
path_basename(path)      # Get filename
path_dirname(path)       # Get directory
path_extname(path)       # Get extension
path_abs(path)           # Get absolute path
```

## 🌐 Network Functions (Planned)

### Basic Network Operations
```sa
# These are reserved for future implementation
scan(target, options)    # Network scanning
connect(host, port)      # Network connection
send(data, target)       # Send data
receive(buffer_size)     # Receive data
```

## 🔐 Cryptographic Functions (Planned)

### Encryption and Decryption
```sa
# These are reserved for future implementation
encrypt(data, algorithm, key)    # Encrypt data
decrypt(data, algorithm, key)    # Decrypt data
hash(data, algorithm)            # Generate hash
generate_key(algorithm, size)    # Generate encryption key
sign(data, private_key)          # Digital signature
verify(data, signature, public_key) # Verify signature
```

### Supported Algorithms (Planned)
- **AES-256** - Symmetric encryption
- **RSA** - Asymmetric encryption
- **SHA-256** - Hashing
- **HMAC** - Message authentication
- **ECDSA** - Digital signatures

## 🎯 Utility Functions

### Type Conversion
```sa
parseInt(string)         # Parse to integer
parseFloat(string)       # Parse to float
type(value)              # Get type name
isinstance(value, type)  # Check instance type
```

### Time and Date
```sa
time()                   # Current timestamp
date()                   # Current date string
datetime()               # Current datetime string
sleep(seconds)           # Sleep for specified time
```

### System Operations
```sa
exit(code)               # Exit program
env(var_name)            # Get environment variable
set_env(var_name, value) # Set environment variable
args()                   # Command line arguments
```

### Debugging
```sa
print(...)               # Output to console
debug(...)               # Debug output (conditional)
assert(condition, message) # Assertion check
trace(expression)        # Trace expression evaluation
```

## 🔧 Built-in Constants

### Language Constants
```sa
true, false              # Boolean values
nil, null, undefined    # Null values
__version__              # Language version
__file__                 # Current file path
__line__                 # Current line number
```

### Platform Constants
```sa
OS_NAME                 # Operating system name
ARCH                    # System architecture
PATH_SEPARATOR          # Path separator character
```

## 📋 Error Handling Functions

### Exception Handling
```sa
try {
    # Code that might throw
} catch (error) {
    # Handle error
} finally {
    # Cleanup code
}

throw(message)          # Throw exception
rethrow()              # Re-throw current exception
```

### Error Types
```sa
ParseError              # Syntax errors
RuntimeError            # Runtime errors
TypeError               # Type errors
ValueError              # Value errors
IndexError              # Index errors
KeyError                # Key errors
```

## 🎨 Formatting Functions

### String Formatting
```sa
format(template, args...) # String formatting
printf(template, args...) # Formatted output
sprintf(template, args...) # Formatted string
```

### Number Formatting
```sa
number_format(number, decimals, decimal_point, thousands_sep)
currency_format(number, currency, locale)
percent_format(number, decimals)
```

## 🔍 Reflection Functions

### Introspection
```sa
has_property(obj, prop)  # Check property existence
get_property(obj, prop)  # Get property descriptor
set_property(obj, prop, descriptor) # Set property
delete_property(obj, prop) # Delete property
```

### Function Information
```sa
func_name(function)     # Get function name
func_params(function)   # Get parameter names
func_arity(function)    # Get parameter count
```

---

*This standard library provides comprehensive functionality for Quantum Language programs, with planned expansions for cybersecurity features.*
