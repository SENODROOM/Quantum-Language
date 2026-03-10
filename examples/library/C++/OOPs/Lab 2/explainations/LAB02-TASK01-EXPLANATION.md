# LAB01 - TASK 01: Average Calculator
## Line-by-Line Deep Explanation

---

## 📌 What Does This Program Do?
This program asks the user to enter a list of floating-point numbers, then calculates and prints their **average (mean)**. It uses dynamic memory to store the numbers and a separate function to compute the average.

---

## 🔧 Headers and Namespace

```cpp
#include <iostream>
using namespace std;
```
- `#include <iostream>` — Enables `cout` and `cin` for printing and reading input.
- `using namespace std;` — Lets us write `cout` and `cin` directly instead of `std::cout` and `std::cin`.

---

## 🔧 Function Declaration (Prototype)

```cpp
float average(const float* arr, int size);
```
- This is a **function prototype** — it tells the compiler that a function called `average` exists, even though its actual code is written below `main()`.
- `float` — The function returns a floating-point number (the average).
- `const float* arr` — A pointer to an array of floats. The `const` keyword means this function **promises not to modify** the array values.
- `int size` — The number of elements in the array.

> 💡 **Why declare before `main()`?** C++ reads code top-to-bottom. If `main()` calls `average()` before the compiler has seen its definition, it will throw an error. A prototype fixes this by informing the compiler in advance.

---

## 🔧 `main()` Function

```cpp
int n;
cout << "Enter number of elements: ";
cin >> n;
```
- Declare an integer `n` to hold how many numbers the user wants to average.
- Ask the user and read the input.

```cpp
float* arr = new float[n];
```
- **Dynamically allocate** an array of `n` floats on the heap.
- We use `new` because `n` is only known at runtime (the user decides the size). A regular stack array like `float arr[n]` is not guaranteed to work for runtime sizes in standard C++.
- `float*` — `arr` is a pointer to the first element of this new array.

```cpp
for (int i = 0; i < n; i++)
{
    cout << "Enter value " << i + 1 << ": ";
    cin >> arr[i];
}
```
- Loop from `0` to `n-1`, prompting the user for each value.
- `arr[i]` — Access the `i`th element of the array (same as `*(arr + i)`).
- `i + 1` in the prompt makes it user-friendly (shows 1, 2, 3... instead of 0, 1, 2...).

```cpp
cout << "Average = " << average(arr, n) << endl;
```
- Call the `average` function, passing the array pointer and size.
- The returned float is directly printed. No need to store it in a variable first.

```cpp
delete[] arr;
return 0;
```
- `delete[] arr` — **Free the heap memory** we allocated with `new float[n]`. The `[]` is required because we allocated an array, not just a single element.
- `return 0` — Program finishes successfully.

---

## 🔧 Function: `average(const float* arr, int size)`

```cpp
float average(const float* arr, int size)
{
    float sum = 0;
    for (int i = 0; i < size; i++)
        sum += arr[i];

    return sum / size;
}
```
- `float sum = 0;` — Initialize a running total to zero.
- The loop goes through every element and adds it to `sum`.
  - `sum += arr[i]` is shorthand for `sum = sum + arr[i]`.
- `return sum / size;` — Divide total by count to get the **mean (average)**.
  - Since `sum` is a `float`, the division is **floating-point division** (not integer division), so you get decimal results.

---

## 🧠 Key Concepts Used

| Concept | Where Used |
|---|---|
| Function prototype | `float average(...)` declared before `main()` |
| Dynamic memory (`new`/`delete[]`) | Allocating `arr` at runtime |
| `const` pointer parameter | Prevents accidental modification of array |
| Float arithmetic | Division in `average()` returns decimals |
| Array access via pointer | `arr[i]` |
