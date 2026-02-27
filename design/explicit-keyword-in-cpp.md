# `explicit` in C++ (Simple Guide)

`explicit` means: "Do not convert automatically. I want conversion to be written clearly."

## 1) Main idea with one example

### Without `explicit`

```cpp
class Age {
public:
    Age(int years) : years_(years) {}

private:
    int years_;
};

void printAge(Age a) {}

int main() {
    printAge(25); // 25 is automatically converted to Age(25)
}
```

### With `explicit`

```cpp
class Age {
public:
    explicit Age(int years) : years_(years) {}

private:
    int years_;
};

void printAge(Age a) {}

int main() {
    // printAge(25);      // Error
    printAge(Age(25));    // OK, conversion is clear
}
```

## 2) Why this is useful

- Prevents accidental conversions
- Makes code easier to read
- Reduces subtle bugs

## 3) Another real-world style example

```cpp
class FilePath {
public:
    explicit FilePath(const std::string& path) : path_(path) {}

private:
    std::string path_;
};

void openFile(FilePath path) {}

int main() {
    // openFile(\"data.txt\");              // Error
    openFile(FilePath(\"data.txt\"));       // OK
}
```

## 4) `explicit` with conversion operators

```cpp
class Switch {
public:
    explicit operator bool() const { return true; }
};

int main() {
    Switch s;

    if (s) {               // OK
    }

    // int x = s;          // Error: no implicit conversion to int
}
```

## 5) Rule to remember

For constructors that can be called with one argument, use `explicit` by default.
