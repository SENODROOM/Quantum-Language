# LAB03 - TASK 01: Super Mario's Inventory Challenge
## Line-by-Line Deep Explanation

---

## 📌 What Does This Program Do?
This program simulates Mario's inventory bag. You can **add items**, **remove items**, **display** what's in the bag, **shrink** the bag (make it smaller), or **regrow** it (make it bigger). All of this is done using **dynamic memory** (memory allocated at runtime using `new`).

---

## 🔧 Header and Namespace

```cpp
#include <iostream>
using namespace std;
```
- `#include <iostream>` — Brings in the library so we can use `cout` (print) and `cin` (input).
- `using namespace std;` — Lets us write `cout` instead of `std::cout` every time.

---

## 🔧 Function: `add(int *ptr, int &size, int &m)`

```cpp
void add(int *ptr, int &size, int &m)
```
- This function **adds a new item** to Mario's inventory.
- `int *ptr` — A pointer to the array (the inventory bag).
- `int &size` — A **reference** to the total capacity of the bag. Using `&` means changes here affect the original variable.
- `int &m` — A **reference** to how many items are currently in the bag.

```cpp
if (m < size)
```
- Checks if there's still **space** in the bag (current items `m` must be less than total `size`).

```cpp
cout << "Enter number: ";
cin >> *(ptr + m);
```
- `*(ptr + m)` means: go to memory position `m` steps from the start of the array, and store the input there.
- This is **pointer arithmetic** — instead of `ptr[m]`, we write `*(ptr + m)`. Both do the same thing.

```cpp
m++;
```
- Increase the item count by 1 because we just added something.

```cpp
else
{
    cout << "Inventory is full!" << endl;
}
```
- If there's no space, just tell Mario the bag is full.

---

## 🔧 Function: `remove(int &m)`

```cpp
void remove(int &m)
```
- This function **removes the last item** from the inventory.
- It only takes `m` (item count) because we don't actually delete the value — we just pretend it's not there by decreasing the count.

```cpp
if (m == 0)
{
    cout << "No item found" << endl;
}
```
- If there are 0 items, there's nothing to remove.

```cpp
else
{
    m--;
    cout << "Deleted....." << endl;
}
```
- Otherwise, decrease `m` by 1. The last item is now "ignored" even though its value still sits in memory.

---

## 🔧 Function: `display(int *ptr, int &m)`

```cpp
void display(int *ptr, int &m)
```
- This function **shows all items** in Mario's inventory.

```cpp
if (m == 0)
{
    cout << "No item found" << endl;
}
```
- Nothing to show if bag is empty.

```cpp
cout << "Marios's Inventory" << endl;
for (int i = 0; i < m; i++)
{
    cout << "Value " << i + 1 << " is " << *(ptr + i) << endl;
}
```
- Loop from index `0` to `m-1` and print each item.
- `*(ptr + i)` reads the value at position `i` in the array.

---

## 🔧 Function: `shrink(int *&ptr, int &size, int &m)`

```cpp
void shrink(int *&ptr, int &size, int &m)
```
- This function **reduces the size** of the inventory array.
- `int *&ptr` — A **reference to a pointer**. This means we can change what `ptr` points to inside this function, and it will affect the original pointer in `main()`.

```cpp
int newsize;
cout << "Enter the new size: ";
cin >> newsize;
```
- Ask user for the new (smaller) size.

```cpp
int *temp = new int[newsize];
```
- Create a **brand new array** with the new smaller size.

```cpp
for (int i = 0; i < newsize; i++)
{
    *(temp + i) = *(ptr + i);
    cout << *(temp + i) << " ";
}
```
- Copy items from the old array into the new smaller array, only up to `newsize`.

```cpp
delete[] ptr;
```
- **Free the old array's memory** to avoid memory leaks.

```cpp
ptr = temp;
```
- Make `ptr` point to the new smaller array.

```cpp
size = newsize;
m = (m > newsize) ? newsize : m;
```
- Update the size variable.
- `(m > newsize) ? newsize : m` — This is a ternary (shorthand if-else). If we had more items than the new size, cap `m` at `newsize`. Otherwise keep `m` as it is.

---

## 🔧 Function: `regrow(int *&ptr, int &size, int &m)`

```cpp
void regrow(int *&ptr, int &size, int &m)
```
- This function **increases the size** of the inventory. Note: In this implementation, it doesn't actually reallocate — it just shows existing items after optionally adding more using the existing array. This is a simplified version.

```cpp
int newsize;
cout << "Enter the new size: ";
cin >> newsize;
```
- Ask for the new bigger size.

```cpp
if (newsize > size)
{
    int n;
    do
    {
        cout << "You want to add more item:(0 for yes and 1 for no) ";
        cin >> n;
        if (n == 0)
        {
            add(ptr, newsize, m);
        }
    } while (n != 1);
}
```
- If the new size is larger, ask Mario if he wants to add more items.
- This is a `do-while` loop — it runs **at least once**, and keeps asking until the user says no (enters `1`).
- Calls `add()` with `newsize` so items can fill up to the new capacity.

```cpp
cout << "Regrow Inventory: ";
for (int i = 0; i < m; i++)
{
    cout << *(ptr + i) << " ";
}
```
- Display all current items after the regrow.

---

## 🔧 `main()` Function

```cpp
int size;
cout << "Enter size of Mariao's Inventory: ";
cin >> size;
int *ptr = new int[size];
```
- Ask for initial bag size, then **dynamically allocate** an integer array of that size on the heap.
- `new int[size]` reserves memory at runtime (not compile time).

```cpp
int m = 0;
```
- `m` tracks how many items are currently in the bag. Starts at 0 (empty).

```cpp
int choice;
do {
    // show menu
    cin >> choice;
    if (choice == 1) add(ptr, size, m);
    else if (choice == 2) remove(m);
    else if (choice == 3) display(ptr, m);
    else if (choice == 4) shrink(ptr, size, m);
    else if (choice == 5) regrow(ptr, size, m);
    else break;
} while (choice != 6);
```
- A `do-while` menu loop that keeps showing options until the user chooses 6 (Exit).
- Each choice calls the appropriate function.

```cpp
ptr = nullptr;
delete[] ptr;
```
- ⚠️ **Bug here!** The code sets `ptr = nullptr` BEFORE calling `delete[] ptr`. Deleting `nullptr` does nothing (it's safe but pointless). The correct order should be `delete[] ptr;` first, then `ptr = nullptr;`.

```cpp
system("pause");
return 0;
```
- `system("pause")` is a Windows command that pauses the console so you can see the output.
- `return 0` ends the program successfully.

---

## 🧠 Key Concepts Used
| Concept | Where Used |
|---|---|
| Dynamic memory (`new`/`delete`) | Creating and freeing the array |
| Pointer arithmetic (`*(ptr + i)`) | Accessing array elements |
| Pass by reference (`&`) | Modifying variables in calling function |
| Reference to pointer (`*&`) | Changing what a pointer points to |
| Ternary operator (`? :`) | Capping item count in shrink |
| `do-while` loop | Menu and regrow logic |
