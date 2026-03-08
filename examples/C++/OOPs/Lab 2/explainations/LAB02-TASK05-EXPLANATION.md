# LAB01 - TASK 05: Remove Duplicates
## Line-by-Line Deep Explanation

---

## 📌 What Does This Program Do?
This program reads an array of integers from the user and **removes all duplicate values**, keeping only the first occurrence of each number. It does this **in-place** — no second array is used. The array effectively shrinks as duplicates are found and removed.

**Example:** Input `[3, 1, 3, 2, 1, 4]` → Output `[3, 1, 2, 4]`

---

## 🔧 Headers and Namespace

```cpp
#include <iostream>
using namespace std;
```
- Standard I/O setup.

---

## 🔧 `main()` Function

### Step 1: Input

```cpp
int n;
cout << "Enter size: ";
cin >> n;

int* arr = new int[n];
for (int i = 0; i < n; i++)
    cin >> arr[i];
```
- Ask for the array size, dynamically allocate it, and fill it with user-entered integers.

---

### Step 2: Removing Duplicates (The Core Logic)

```cpp
for (int i = 0; i < n; i++)
{
    for (int j = i + 1; j < n; )
    {
```
- **Outer loop** (`i`) — Picks each element one by one as the "reference" element to check against.
- **Inner loop** (`j`) — Starts from `i + 1` and scans the rest of the array for duplicates of `arr[i]`.
- Notice `j` has **no increment** in the `for` statement (`for (int j = i + 1; j < n; )`). This is intentional — `j` is only incremented in the `else` branch, not when a duplicate is removed (explained below).

```cpp
        if (arr[i] == arr[j])
        {
            for (int k = j; k < n - 1; k++)
                arr[k] = arr[k + 1];
            n--;
        }
```
- **Duplicate found!** `arr[j]` is the same as `arr[i]`.
- **Shift everything left** starting from position `j`:
  - `arr[k] = arr[k + 1]` copies the next element into the current position, effectively erasing `arr[j]` by overwriting it.
  - The loop runs from `j` to `n - 2` (we go up to `n - 1` but access `k + 1`, so stop at `n - 2`).
- `n--` — **Shrink the logical size** of the array by 1. We don't free or reallocate memory — we just treat the last slot as no longer part of the array.
- After removal, `j` is **NOT incremented**. Why? Because after shifting, a new element has moved into position `j`. We need to check it too — it might also be a duplicate of `arr[i]`.

```cpp
        else
            j++;
```
- **No duplicate** at this position — move `j` forward to check the next element.
- `j` only advances when no removal happened.

---

### 🎯 Trace Example

Array: `[3, 1, 3, 2, 1]`, n = 5

**i=0 (arr[0]=3):**
- j=1: 1 ≠ 3 → j++ → j=2
- j=2: 3 == 3 → remove arr[2], shift left → `[3, 1, 2, 1, _]`, n=4. j stays at 2
- j=2: 2 ≠ 3 → j++ → j=3
- j=3: 1 ≠ 3 → j++ → j=4 → inner loop ends

**i=1 (arr[1]=1):**
- j=2: 2 ≠ 1 → j++ → j=3
- j=3: 1 == 1 → remove arr[3], shift left → `[3, 1, 2, _]`, n=3. j stays at 3
- j=3 ≥ n=3 → inner loop ends

**i=2 (arr[2]=2):**
- j=3 ≥ n=3 → inner loop doesn't run

Final: `[3, 1, 2]` ✅

---

### Step 3: Display and Clean Up

```cpp
cout << "After removing duplicates:\n";
for (int i = 0; i < n; i++)
    cout << arr[i] << " ";

delete[] arr;
return 0;
```
- Print elements from index `0` to the updated `n - 1`.
- `delete[] arr` frees the originally allocated memory (the array is the same size in memory, but we only printed `n` elements after shrinking the logical count).

---

## 🧠 Key Concepts Used

| Concept | Where Used |
|---|---|
| In-place removal | No second array; elements shifted left |
| Manual left-shift | Inner `k` loop overwrites the duplicate |
| Logical shrinking | `n--` reduces effective array size |
| Controlled `j` increment | Only advance `j` when no removal occurred |
| Dynamic memory | `new int[n]` / `delete[] arr` |

---

## ⚠️ Why Not Always Increment `j`?

This is the trickiest part. After removing `arr[j]` and shifting everything left, the element that was at `j+1` is now at `j`. If we incremented `j`, we'd **skip checking it**. By keeping `j` the same after a removal, we ensure every element gets checked.
