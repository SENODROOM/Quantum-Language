# LAB03 - TASK 03: Super Mario Diagonal Coin Quest
## Line-by-Line Deep Explanation

---

## 📌 What Does This Program Do?
Mario wants to collect coins from a 2D grid by moving **diagonally** (top-left to bottom-right). This program takes a grid as input and prints all values in diagonal order — starting from the top-left corner, covering every diagonal strip of the matrix.

**Example (4×5 grid with values 1-20):**
```
 1   2   3   4   5
 6   7   8   9  10
11  12  13  14  15
16  17  18  19  20
```
**Diagonal output:** `1 6 2 11 7 3 16 12 8 4 17 13 9 5 18 14 10 19 15 20`

---

## 🔧 Header and Namespace

```cpp
#include <iostream>
using namespace std;
```
- Brings in the input/output library and avoids needing `std::` prefix.

---

## 🔧 Function: `diagonal(int *ptr, int row, int col)`

```cpp
void diagonal(int *ptr, int row, int col)
```
- This function **traverses and prints the grid diagonally**.
- `int *ptr` — Pointer to the 1D array that stores the 2D grid.
- `int row` — Number of rows.
- `int col` — Number of columns.

### 🔍 Understanding the Diagonal Pattern

Each diagonal goes **top-right to bottom-left** (↘ direction but printed together). There are two groups of diagonals:

1. **Diagonals starting from the first COLUMN** (rows 0, 1, 2, ... row-1)
2. **Diagonals starting from the first ROW** (columns 1, 2, ... col-1)

---

### Part 1: Diagonals starting from the first column

```cpp
cout << "Transverse diagonal  of garden is : ";
for (int i = 0; i < row; i++)
{
    for (int j = 0; j <= i; j++)
    {
        cout << *(ptr + (i - j) * (col) + j) << " ";
    }
}
```

- **Outer loop** (`i`) goes from 0 to `row-1`. Each value of `i` represents a new diagonal.
- **Inner loop** (`j`) goes from 0 to `i`. Each step moves one cell diagonally (down one row, right one column).
- `*(ptr + (i - j) * col + j)`:
  - `(i - j)` = the current **row** index in the grid.
  - `j` = the current **column** index.
  - Together: `row_index * col + col_index` = the 1D position of `[row][col]`.

**Visualization for i=2 (3rd diagonal):**
```
j=0: access [2][0] = 11
j=1: access [1][1] = 7
j=2: access [0][2] = 3
→ prints: 11 7 3
```

---

### Part 2: Diagonals starting from the first row (after column 0)

```cpp
for (int i = 1; i < col; i++)
{
    int n = i;

    for (int j = row - 1; j > (i - 1); j--)
    {
        cout << *(ptr + (j) * (col) + n) << " ";
        n++;
    }
}
```

- **Outer loop** (`i`) starts from column 1 (column 0 was already covered in Part 1).
- `int n = i;` — `n` tracks the current column, starting at `i` and increasing as we go down the diagonal.
- **Inner loop** (`j`) starts from the **last row** and goes upward. This is because we're printing each diagonal from bottom to top so the order matches the expected output.
  - `j > (i - 1)` stops when we'd go out of bounds.
- `*(ptr + j * col + n)`:
  - `j` = current row.
  - `n` = current column (increments each step going right).

**Visualization for i=1 (diagonal starting at row 0, col 1):**
```
j=3, n=1: access [3][1] = 17
j=2, n=2: access [2][2] = 13
j=1, n=3: access [1][3] = 9
j=0, n=4: access [0][4] = 5
→ prints: 17 13 9 5
```

---

## 🔧 `main()` Function

```cpp
int row, col;
cout << "Enter number of rows in garden : ";
cin >> row;
cout << "Enter number of columns in garden: ";
cin >> col;
```
- Ask the user for grid dimensions separately.

```cpp
if (col < row)
{
    cout << "Number of columns should be greater than  number of rows." << endl;
    return 0;
}
```
- **Validation check**: The diagonal algorithm in this code only works correctly when `col >= row`.
- If this condition fails, exit early with an error message. `return 0` exits `main()`.

```cpp
int *ptr = new int[row * col];
```
- Dynamically allocate a 1D array to hold the entire 2D grid.

```cpp
for (int i = 0; i < row; i++)
{
    for (int j = 0; j < col; j++)
    {
        cout << "Enter element for row " << i + 1 << " and column " << j + 1 << ": ";
        cin >> *(ptr + i * col + j);
    }
}
```
- Nested loop to fill the grid row by row.
- `*(ptr + i * col + j)` — pointer arithmetic to access `[i][j]` in the 1D array.

```cpp
diagonal(ptr, row, col);
```
- Call the diagonal function to print elements in diagonal order.

```cpp
delete[] ptr;
```
- Free the dynamically allocated memory to prevent memory leaks.

```cpp
return 0;
```
- Program ends successfully.

---

## 🧠 Key Concepts Used

| Concept | Where Used |
|---|---|
| Dynamic 2D as 1D array | `new int[row * col]` |
| Pointer arithmetic | `*(ptr + i * col + j)` |
| Diagonal traversal logic | Two-loop structure in `diagonal()` |
| Input validation | `col < row` check |
| Memory management | `delete[] ptr` |

---

## 🎯 Diagonal Traversal Summary

For a 4×5 grid, the diagonals visited are:
```
Diagonal 1: [0][0]                    → 1
Diagonal 2: [1][0], [0][1]            → 6, 2
Diagonal 3: [2][0], [1][1], [0][2]   → 11, 7, 3
Diagonal 4: [3][0], [2][1], [1][2], [0][3]  → 16, 12, 8, 4
Diagonal 5: [3][1], [2][2], [1][3], [0][4]  → 17, 13, 9, 5
Diagonal 6: [3][2], [2][3], [1][4]   → 18, 14, 10
Diagonal 7: [3][3], [2][4]           → 19, 15
Diagonal 8: [3][4]                   → 20
```
