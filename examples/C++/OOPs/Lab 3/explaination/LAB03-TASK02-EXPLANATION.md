# LAB03 - TASK 02: Mario's Power Grid (2D Dynamic Square Matrix)
## Line-by-Line Deep Explanation

---

## 📌 What Does This Program Do?
This program creates a **2D grid (matrix)** where Mario can store power levels. The user picks the number of rows and columns, fills in the grid, and the program displays the original grid AND its **transpose** (rows and columns flipped). Memory is managed dynamically using `new` and `delete`.

---

## 🔧 Header and Namespace

```cpp
#include <iostream>
using namespace std;
```
- Standard setup: enables input/output and saves us from writing `std::` everywhere.

---

## 🔧 Function: `input(int *ptr, int row, int col)`

```cpp
void input(int *ptr, int row, int col)
```
- This function **fills the 2D grid** with values entered by the user.
- `int *ptr` — A pointer to the start of our 1D array (which represents a 2D grid in memory).
- `int row` — Number of rows.
- `int col` — Number of columns.

> 💡 **Key Idea:** A 2D array is stored as a 1D array in memory. To access element at row `i` and column `j`, the formula is:
> **`ptr[i * col + j]`** or **`*(ptr + i * col + j)`**

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
- **Outer loop** (`i`) goes through each row.
- **Inner loop** (`j`) goes through each column in that row.
- `*(ptr + i * col + j)` — This is how we access position `[i][j]` in a 1D array pretending to be 2D.
  - `i * col` skips over `i` complete rows.
  - `+ j` moves to the correct column.
  - `*` dereferences (gets the actual memory location) to store the input.

---

## 🔧 Function: `display(int *ptr, int row, int col)`

```cpp
void display(int *ptr, int row, int col)
```
- This function **prints the 2D grid** in a readable matrix format.

```cpp
cout << "The elements of the array are: " << endl;
for (int i = 0; i < row; i++)
{
    for (int j = 0; j < col; j++)
    {
        cout << *(ptr + i * col + j) << " ";
    }
    cout << endl;
}
```
- Same nested loop logic as `input()`, but now we **read** and print values instead of writing them.
- `cout << endl;` after the inner loop moves to a new line after each row — this creates the grid appearance.

**Example output for a 3x3 grid:**
```
1 2 3 
4 5 6 
7 8 9 
```

---

## 🔧 Function: `transpose(int *ptr, int row, int col)`

```cpp
void transpose(int *ptr, int row, int col)
```
- This function prints the **transpose** of the matrix.
- Transpose means: **rows become columns and columns become rows**.
- Original `[i][j]` becomes `[j][i]` in the transpose.

```cpp
cout << "The transpose of the array is: " << endl;
for (int i = 0; i < col; i++)
{
    for (int j = 0; j < row; j++)
    {
        cout << *(ptr + j * col + i) << " ";
    }
    cout << endl;
}
```
- The **outer loop** now goes up to `col` (instead of `row`) because after transposing, the number of rows equals the original column count.
- The **inner loop** goes up to `row`.
- `*(ptr + j * col + i)` — Notice `i` and `j` are swapped compared to display:
  - `j * col + i` accesses column `i` of row `j` — that's exactly what transpose does.

**Example: Transpose of the above 3x3:**
```
1 4 7 
2 5 8 
3 6 9 
```

---

## 🔧 `main()` Function

```cpp
int row, col;
cout << "Enter size of row and col: ";
cin >> row >> col;
```
- Ask the user for the number of rows and columns.
- `cin >> row >> col;` reads both values in one line (user enters: `3 3` for a 3x3 grid).

```cpp
int *ptr = new int[row * col];
```
- **Dynamically allocate** a 1D array with `row * col` total elements.
- For a 3x3 grid: allocates 9 integers in a continuous block of memory.
- This simulates a 2D array using a 1D block.

```cpp
input(ptr, row, col);
```
- Call the input function to fill the grid.

```cpp
display(ptr, row, col);
```
- Print the original grid.

```cpp
transpose(ptr, row, col);
```
- Print the transposed grid.

```cpp
delete[] ptr;
```
- **Free the memory** we allocated with `new`. The `[]` is important because we allocated an array, not just one element.

```cpp
return 0;
```
- Program ends successfully.

---

## 🧠 Key Concepts Used

| Concept | Where Used |
|---|---|
| Dynamic 2D array (as 1D) | `new int[row * col]` |
| Pointer arithmetic for 2D | `*(ptr + i * col + j)` |
| Matrix transpose logic | Swapping `i` and `j` in loop |
| Memory deallocation | `delete[] ptr` |
| Nested loops | Input, display, and transpose |

---

## 🎯 Quick Memory Layout Visualization

For a **2x3 matrix** (2 rows, 3 cols):
```
Logical view:        Memory (1D array):
[1, 2, 3]    →      [1, 2, 3, 4, 5, 6]
[4, 5, 6]           index: 0  1  2  3  4  5

Access [1][2] = *(ptr + 1*3 + 2) = *(ptr + 5) = 6 ✅
```
