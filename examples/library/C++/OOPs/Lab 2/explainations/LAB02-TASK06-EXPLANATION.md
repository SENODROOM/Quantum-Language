# LAB01 - TASK 06: Dynamic Array with Auto-Resize (Jellyfish IDs)
## Line-by-Line Deep Explanation

---

## 📌 What Does This Program Do?
This program collects jellyfish ID numbers from the user one by one. If the array runs out of space, it **automatically doubles in size** by allocating a larger array and copying existing data into it. The user types `-1` to stop entering IDs. This simulates how dynamic data structures like `std::vector` work internally.

---

## 🔧 Headers and Namespace

```cpp
#include <iostream>
using namespace std;
```
- Standard I/O setup.

---

## 🔧 `main()` Function

### Step 1: Initial Setup

```cpp
int size;
cout << "Enter initial size: ";
cin >> size;

int* arr = new int[size];
int count = 0;
```
- Ask for the **starting capacity** of the array.
- `new int[size]` — Dynamically allocate the initial array on the heap.
- `count` — Tracks how many IDs have been stored so far. Starts at 0.
- Note the difference: `size` = how many slots exist, `count` = how many are filled.

---

### Step 2: Input Loop

```cpp
while (true)
{
    int id;
    cin >> id;

    if (id == -1)
        break;
```
- `while (true)` — An **infinite loop** that only exits when explicitly broken.
- Read an ID from the user.
- `if (id == -1) break;` — `-1` is the **sentinel value** (a special value that signals "stop"). When the user enters -1, the loop exits immediately with `break`.

---

### Step 3: Check if Array is Full

```cpp
    if (count == size)
    {
        int* newArr = new int[size * 2];
        for (int i = 0; i < size; i++)
            newArr[i] = arr[i];

        delete[] arr;
        arr = newArr;
        size *= 2;
    }
```
- `if (count == size)` — The array is **completely full** (every slot is occupied).
- **Resize process:**
  1. `int* newArr = new int[size * 2];` — Allocate a **new array twice as big**.
  2. `for (int i = 0; i < size; i++) newArr[i] = arr[i];` — **Copy all existing data** from the old array to the new one.
  3. `delete[] arr;` — **Free the old array's memory** to prevent a memory leak.
  4. `arr = newArr;` — Make `arr` point to the new larger array. Now all future operations use the bigger array.
  5. `size *= 2;` — Update `size` to reflect the new capacity.

> 💡 **Why double the size?** Doubling is a common strategy (used by `std::vector` too). It ensures that over many insertions, the average cost of resizing is low — this is called **amortized O(1)** insertion time.

---

### Step 4: Store the ID

```cpp
    arr[count++] = id;
```
- Store the entered ID at the current `count` position.
- `count++` — **Post-increment**: first use the current value of `count` as the index, then increment it.
  - So if `count` was 3: store at `arr[3]`, then `count` becomes 4.
- This is a compact one-liner equivalent to:
  ```cpp
  arr[count] = id;
  count++;
  ```

---

### Step 5: Display Results

```cpp
cout << "Jellyfish IDs:\n";
for (int i = 0; i < count; i++)
    cout << arr[i] << " ";
```
- Print all stored IDs using `count` (not `size`) as the loop bound, since `size` might be larger than the number of actual entries after resizing.

---

### Step 6: Free Memory

```cpp
delete[] arr;
return 0;
```
- Free the final array (which might have been resized multiple times).
- We only need one `delete[]` here because the old arrays were already freed inside the loop each time a resize happened.

---

## 🎯 Resize Walkthrough

Initial size = 2. User enters: `10, 20, 30, 40, -1`

| Step | count | size | Action |
|------|-------|------|--------|
| Enter 10 | 0→1 | 2 | Store 10 at arr[0] |
| Enter 20 | 1→2 | 2 | Store 20 at arr[1] |
| Enter 30 | count==size! | 2→4 | Resize to 4. Store 30 at arr[2] |
| Enter 40 | 3→4 | 4 | Store 40 at arr[3] |
| Enter -1 | — | — | Break out of loop |
| Output | — | — | `10 20 30 40` |

---

## 🧠 Key Concepts Used

| Concept | Where Used |
|---|---|
| Dynamic array resizing | Doubling when `count == size` |
| Sentinel value | `-1` to stop input |
| `while(true)` with `break` | Infinite loop with manual exit |
| Copy-then-delete pattern | Migrate data to new array, free old one |
| Post-increment `count++` | Store then advance index in one line |
| `size` vs `count` | Capacity vs actual used elements |

---

## 🔗 Real-World Connection

This is exactly how `std::vector` works in C++ under the hood:
- It starts with a small capacity.
- When full, it allocates a larger block (typically doubles), copies everything over, and frees the old block.
- You never have to think about this when using `vector` — but this task shows you the mechanics behind it!
