# LAB01 - TASK 02: Bubble Sort (Mood Sorter)
## Line-by-Line Deep Explanation

---

## 📌 What Does This Program Do?
This program reads a list of floating-point "mood" values from the user, sorts them from **smallest to largest** using the **Bubble Sort** algorithm, and then prints the sorted result.

---

## 🔧 Headers and Namespace

```cpp
#include <iostream>
using namespace std;
```
- Standard setup for input/output. `using namespace std;` avoids needing `std::` prefix.

---

## 🔧 Function Prototype

```cpp
void bubbleSort(float* arr, int n);
```
- Declares the `bubbleSort` function before `main()` so the compiler knows it exists.
- `void` — This function doesn't return anything; it sorts the array **in-place** (modifies the original).
- `float* arr` — Pointer to the array to be sorted.
- `int n` — Number of elements.

---

## 🔧 `main()` Function

```cpp
int n;
cout << "Enter number of moods: ";
cin >> n;
```
- Ask how many mood values the user wants to sort.

```cpp
float* moods = new float[n];
```
- Dynamically allocate an array of `n` floats to hold the mood values.

```cpp
for (int i = 0; i < n; i++)
    cin >> moods[i];
```
- Read each mood value into the array. Notice there's no `cout` prompt here — values are entered one after another (can be space-separated or on separate lines).

```cpp
bubbleSort(moods, n);
```
- Call the sort function. Since arrays decay to pointers when passed to functions, `moods` passes the **address of the first element**. Any changes inside `bubbleSort` affect the original array.

```cpp
cout << "Sorted moods:\n";
for (int i = 0; i < n; i++)
    cout << moods[i] << " ";
```
- Print each sorted value separated by spaces.

```cpp
delete[] moods;
return 0;
```
- Free the heap memory and exit.

---

## 🔧 Function: `bubbleSort(float* arr, int n)`

Bubble Sort works by **repeatedly comparing adjacent pairs** and swapping them if they're in the wrong order. After each full pass, the **largest unsorted element "bubbles up"** to its correct position at the end.

```cpp
void bubbleSort(float* arr, int n)
{
    for (int i = 0; i < n - 1; i++) {
```
- **Outer loop** runs `n - 1` times (one pass per element, minus one because the last element sorts itself).
- After each pass `i`, the last `i` elements are already in their correct place, so we don't need to check them again.

```cpp
        for (int j = 0; j < n - i - 1; j++) {
```
- **Inner loop** compares adjacent elements.
- `n - i - 1` — Each pass reduces the range because the largest element of that pass has already moved to its final position. This avoids redundant comparisons.

```cpp
            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]);
            }
```
- If the current element is **greater than** the next one, swap them.
- `swap()` is a built-in C++ function (from `<algorithm>`, but also available via `<iostream>` indirectly) that exchanges two values efficiently.
- This ensures smaller values gradually move left and larger ones move right.

---

## 🎯 Bubble Sort Visualization

For array: `[3.5, 1.2, 4.1, 2.0]`

**Pass 1 (i=0):**
- Compare 3.5 & 1.2 → swap → `[1.2, 3.5, 4.1, 2.0]`
- Compare 3.5 & 4.1 → no swap
- Compare 4.1 & 2.0 → swap → `[1.2, 3.5, 2.0, 4.1]` ← 4.1 is in place ✅

**Pass 2 (i=1):**
- Compare 1.2 & 3.5 → no swap
- Compare 3.5 & 2.0 → swap → `[1.2, 2.0, 3.5, 4.1]` ← 3.5, 4.1 in place ✅

**Pass 3 (i=2):**
- Compare 1.2 & 2.0 → no swap → `[1.2, 2.0, 3.5, 4.1]` ✅ Done!

---

## 🧠 Key Concepts Used

| Concept | Where Used |
|---|---|
| Dynamic memory | `new float[n]` / `delete[] moods` |
| Pass-by-pointer | Array modified inside `bubbleSort` |
| Bubble Sort algorithm | Nested loops with adjacent swaps |
| `swap()` function | Exchanging two array elements |
| `void` return type | Function sorts in-place, no return needed |
