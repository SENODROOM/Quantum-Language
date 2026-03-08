# LAB03 - TASK 04: Mario's Item Chest (2D Jagged Char Array)
## Line-by-Line Deep Explanation

---

## 📌 What Does This Program Do?
Instead of storing item codes (numbers), this program stores the **actual names** of Mario's items as characters. Each item name can be a different length, so we use a **jagged array** — a 2D array where each row has a different number of columns. No `string` type is allowed; everything is done with raw `char` pointers.

---

## 🔧 Header and Namespace

```cpp
#include <iostream>
using namespace std;
```
- Standard includes for I/O operations.

---

## 🔧 `main()` Function — Step by Step

### Step 1: Temporary Buffer

```cpp
char *ptr = new char[100];
```
- Allocates a **temporary buffer** of 100 characters on the heap.
- This is used as a scratch pad to read each item name character by character before we know its length.
- Think of it as a temporary notepad Mario uses before copying the name to a proper storage location.

---

### Step 2: Get Number of Items

```cpp
int size;
cout << "Enter total items: ";
cin >> size;
cin.ignore();
```
- Ask the user how many items Mario wants to store.
- `cin.ignore();` — This is **very important**. After `cin >> size`, the Enter key press (`\n`) stays in the input buffer. If we don't ignore it, the next character-by-character read will immediately see `\n` and think the name is empty. `cin.ignore()` discards that leftover newline.

---

### Step 3: Allocate Storage Arrays

```cpp
int *ptr1 = new int[size];
char **names = new char *[size];
```
- `int *ptr1 = new int[size];` — An array to store the **length** of each item name. `ptr1[i]` holds how many characters item `i` has.
- `char **names = new char *[size];` — This is a **pointer to a pointer** (double pointer). It creates an array of `char*` pointers. Each `names[i]` will eventually point to a dynamically allocated char array holding one item name.
  - This is what makes it a **jagged array**: each `names[i]` can point to an array of different size.

---

### Step 4: Read Each Item Name

```cpp
for (int i = 0; i < size; i++)
{
    int n = 0;
    cout << "Enter name of item " << i + 1 << " : ";
    do
    {
        cin.get(*(ptr + n));
        if (*(ptr + n) == '\n')
            break;
        n++;
    } while (true);
```
- For each item, we read it **one character at a time** into the temporary buffer `ptr`.
- `int n = 0;` — Reset the character counter for each new item.
- `cin.get(*(ptr + n));` — Reads **one character** from input and stores it at position `n` in the buffer.
  - `*(ptr + n)` is pointer arithmetic for `ptr[n]`.
  - `cin.get()` reads everything including spaces (unlike `cin >>` which skips spaces).
- `if (*(ptr + n) == '\n') break;` — When we hit Enter (newline `\n`), the name is complete, so we stop the loop.
- `n++` — Move to the next position in the buffer.
- The `do-while(true)` is an **infinite loop** that only breaks when we hit `\n`.

After this loop, `n` holds the **number of characters** in the name (not counting the newline).

---

### Step 5: Store Length and Copy Name

```cpp
    ptr1[i] = n;

    names[i] = new char[n];
    for (int j = 0; j < n; j++)
    {
        names[i][j] = ptr[j];
    }
}
```
- `ptr1[i] = n;` — Save the length of this item's name.
- `names[i] = new char[n];` — Dynamically allocate **exactly** `n` characters for this item's name (no wasted space!). This is why it's a jagged array — each row is a different size.
  - ⚠️ **Note:** The task instructions say to allocate `n+1` for the null terminator `'\0'`, but this code allocates just `n`. It works for printing but technically isn't a proper null-terminated C-string.
- The inner `for` loop copies each character from the temporary buffer `ptr` into `names[i]`.

---

### Step 6: Display All Item Names

```cpp
for (int i = 0; i < size; i++)
{
    cout << "Name of item " << i + 1 << " is : ";
    for (int j = 0; j < ptr1[i]; j++)
    {
        cout << names[i][j];
    }
    cout << endl;
}
```
- For each item `i`, loop through `ptr1[i]` characters and print them one by one.
- `names[i][j]` accesses the `j`th character of the `i`th item name.
- `cout << endl;` moves to the next line after printing each name.

---

### Step 7: Free All Memory

```cpp
for (int i = 0; i < size; i++)
{
    delete[] names[i];
}
delete[] names;
delete[] ptr1;
delete[] ptr;
```
- **Order matters** when freeing memory!
- First, free each individual name array: `delete[] names[i];` — frees the memory each row points to.
- Then free the array of pointers: `delete[] names;` — frees the `char**` array itself.
- Then `delete[] ptr1;` — frees the length-tracking array.
- Finally `delete[] ptr;` — frees the temporary input buffer.
- If we did `delete[] names` before freeing `names[i]`, we'd lose the pointers and cause a **memory leak**.

```cpp
return 0;
```
- Program ends successfully.

---

## 🧠 Key Concepts Used

| Concept | Where Used |
|---|---|
| Double pointer (`char**`) | `names` — array of char arrays |
| Jagged array | Each `names[i]` has different size |
| Character-by-character input | `cin.get()` loop |
| `cin.ignore()` | Clearing leftover newline from buffer |
| Dynamic allocation per string | `new char[n]` for each item |
| Multi-level memory deallocation | Delete inner arrays before outer |

---

## 🎯 Memory Layout Visualization

For 3 items: "Star", "Mushroom", "Fire"

```
names (char**):
  names[0] → ['S','t','a','r']             (4 chars)
  names[1] → ['M','u','s','h','r','o','o','m']  (8 chars)
  names[2] → ['F','i','r','e']             (4 chars)

ptr1: [4, 8, 4]   (stores lengths)
```

Each row has a **different length** — that's what makes it "jagged"!
