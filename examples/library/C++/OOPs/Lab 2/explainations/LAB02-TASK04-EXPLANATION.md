# LAB01 - TASK 04: Cumulative Power (Running Sum)
## Line-by-Line Deep Explanation

---

## 📌 What Does This Program Do?
This program reads a list of snack power values and computes the **cumulative (running) sum** — at each position, it shows the total power collected so far up to that point. Two separate dynamic arrays are used: one for the original values and one for the cumulative totals.

**Example:** Input `[2, 3, 5, 1]` → Output `[2, 5, 10, 11]`

---

## 🔧 Headers and Namespace

```cpp
#include <iostream>
using namespace std;
```
- Standard I/O setup.

---

## 🔧 `main()` Function

### Step 1: Get Input Size

```cpp
int n;
cout << "Enter number of snacks: ";
cin >> n;
```
- Ask how many snack values will be entered.

---

### Step 2: Allocate Two Arrays

```cpp
float* snacks = new float[n];
float* cumulative = new float[n];
```
- `snacks` — Stores the **original input values** (each snack's power).
- `cumulative` — Stores the **running total** at each index.
- Both are dynamically allocated with `new` since `n` is only known at runtime.
- Both are `float` arrays to support decimal values.

---

### Step 3: Read Snack Values

```cpp
for (int i = 0; i < n; i++)
    cin >> snacks[i];
```
- Fill the `snacks` array with user input. No individual prompts — values are entered all at once.

---

### Step 4: Compute Cumulative Sum

```cpp
cumulative[0] = snacks[0];
```
- The first cumulative value is simply the first snack's value (nothing to add to yet).
- This line **must** be done separately before the loop because the loop references `cumulative[i - 1]`, and index `-1` would be out of bounds.

```cpp
for (int i = 1; i < n; i++) {
    cumulative[i] = cumulative[i - 1] + snacks[i];
}
```
- Start from index `1` (we already handled index `0`).
- Each position's cumulative value = the **previous cumulative value** + the **current snack value**.
- `cumulative[i - 1]` carries forward the running total — we don't need to re-add all previous values each time.

**Trace for `[2.0, 3.0, 5.0, 1.0]`:**
```
cumulative[0] = 2.0
cumulative[1] = 2.0 + 3.0 = 5.0
cumulative[2] = 5.0 + 5.0 = 10.0
cumulative[3] = 10.0 + 1.0 = 11.0
```

---

### Step 5: Display Results

```cpp
cout << "Cumulative Power:\n";
for (int i = 0; i < n; i++)
    cout << cumulative[i] << " ";
```
- Print all cumulative values separated by spaces.

---

### Step 6: Free Memory

```cpp
delete[] snacks;
delete[] cumulative;
return 0;
```
- Both dynamically allocated arrays must be freed separately.
- **Important:** We always `delete[]` arrays (not just `delete`), because we allocated them with `new[]`.
- `return 0` ends the program cleanly.

---

## 🧠 Key Concepts Used

| Concept | Where Used |
|---|---|
| Two separate dynamic arrays | `snacks` and `cumulative` |
| Running sum / prefix sum | `cumulative[i] = cumulative[i-1] + snacks[i]` |
| Handling index `0` separately | Avoids out-of-bounds `i - 1` access |
| Multiple `delete[]` calls | One for each `new[]` allocation |
| Float arrays | Supports decimal power values |

---

## 🎯 Why Two Arrays?

We could theoretically overwrite `snacks` with cumulative values (using one array), but using **two separate arrays** is cleaner and preserves the original data. If you ever needed to show the original values alongside the cumulative ones, you'd still have them in `snacks`.
