# LAB03 - TASK 05: Super Mario's Smart Pointer Adventure
## Line-by-Line Deep Explanation

---

## 📌 What Does This Program Do?
This program introduces **smart pointers** — a modern C++ way to manage memory automatically. Instead of manually calling `new` and `delete`, smart pointers handle memory cleanup on their own. Three types are demonstrated:
- `unique_ptr` — Only ONE owner. Can't be shared.
- `shared_ptr` — Multiple owners. Tracks how many owners exist.
- `weak_ptr` — Observes a `shared_ptr` without owning it.

Item codes: **1 = Mushroom, 2 = Star, 3 = Fireball**

---

## 🔧 Headers

```cpp
#include <iostream>
#include <memory>
using namespace std;
```
- `#include <memory>` — This header is required to use `unique_ptr`, `shared_ptr`, and `weak_ptr`.

---

## 🔧 Function: `add(unique_ptr<int[]> &ptr, int &size, int &m)`

```cpp
void add(unique_ptr<int[]> &ptr, int &size, int &m)
```
- Adds an item to Mario's inventory.
- `unique_ptr<int[]> &ptr` — A **reference** to a `unique_ptr` that manages an integer array.
  - `unique_ptr<int[]>` means it owns an array of integers.
  - We pass by reference (`&`) because `unique_ptr` **cannot be copied** — only moved. Passing by reference avoids the need to copy or move it.
- `int &size` — Reference to the inventory's max capacity.
- `int &m` — Reference to the current item count.

```cpp
if (m < size)
{
    cout << "Enter item code (1=Mushroom, 2=Star, 3=Fireball): ";
    cin >> ptr[m];
    m++;
}
```
- `ptr[m]` — With `unique_ptr<int[]>`, you can use `[]` just like a regular array. This is one of the advantages of smart pointers — they mimic regular pointer syntax.
- Stores the item code and increments the count.

```cpp
else
{
    cout << "Inventory is full!" << endl;
}
```
- Can't add more than `size` items.

---

## 🔧 Function: `remove(int &m)`

```cpp
void remove(int &m)
```
- Same logic as Task 1. Decrements `m` to "remove" the last item (doesn't actually erase the value, just ignores it).

```cpp
if (m == 0) cout << "No item found" << endl;
else { m--; cout << "Deleted....." << endl; }
```

---

## 🔧 Function: `display(unique_ptr<int[]> &ptr, int &m)`

```cpp
void display(unique_ptr<int[]> &ptr, int &m)
```
- Displays all items in the inventory with their names (not just codes).

```cpp
for (int i = 0; i < m; i++)
{
    cout << "Item " << i + 1 << " is ";
    if (ptr[i] == 1) cout << "Mushroom";
    else if (ptr[i] == 2) cout << "Star";
    else if (ptr[i] == 3) cout << "Fireball";
    cout << endl;
}
```
- `ptr[i]` reads each element using array syntax on the smart pointer.
- `if-else` chain converts the number code to a readable name.

---

## 🔧 Function: `sharedDemo()`

This function demonstrates **`shared_ptr`** — shared ownership.

```cpp
void sharedDemo()
{
    cout << "\nShared Ownership Demo" << endl;
```

```cpp
    shared_ptr<int> mushroom = make_shared<int>(1);
    shared_ptr<int> star = make_shared<int>(2);
```
- `make_shared<int>(1)` — Creates a `shared_ptr` that manages one integer with value `1`.
- `make_shared` is preferred over `new` because it's safer and more efficient (allocates the object and control block in one memory operation).
- Both `mushroom` and `star` are the **sole owners** of their respective values.

```cpp
    cout << "Mushroom use count: " << mushroom.use_count() << endl;
    cout << "Star use count: " << star.use_count() << endl;
```
- `.use_count()` returns how many `shared_ptr` objects currently share ownership of the managed object.
- At this point, both counts are **1** (only one owner each).

```cpp
    shared_ptr<int> backpack = mushroom;
    shared_ptr<int> powerupList = mushroom;
```
- We **copy** `mushroom` into two new `shared_ptr` variables.
- Unlike `unique_ptr`, `shared_ptr` **CAN be copied**. Each copy shares ownership.
- The mushroom now has **3 owners**: `mushroom`, `backpack`, `powerupList`.

```cpp
    cout << "After sharing Mushroom:" << endl;
    cout << "Mushroom use count: " << mushroom.use_count() << endl;
    cout << "Star use count: " << star.use_count() << endl;
```
- **Mushroom count** is now **3** (3 shared_ptrs point to it).
- **Star count** is still **1** (nothing shared it).
- When ALL shared_ptrs go out of scope, the memory is automatically freed. No `delete` needed!

---

## 🔧 Function: `weakDemo()`

This function demonstrates **`weak_ptr`** — observing without owning.

```cpp
void weakDemo()
{
    cout << "\nWeak Ownership Demo" << endl;

    weak_ptr<int> weakItem;
```
- A `weak_ptr` is declared but not yet pointing to anything.
- `weak_ptr` does NOT own the memory — it can only observe it.

```cpp
    {
        shared_ptr<int> fireball = make_shared<int>(3);
        cout << "Fireball use count: " << fireball.use_count() << endl;
```
- Open a new **inner scope** (block) with `{`.
- Create a `shared_ptr` to a Fireball (value = 3). Use count = 1.

```cpp
        weakItem = fireball;
        cout << "After creating weak_ptr, Fireball use count: " << fireball.use_count() << endl;
```
- Assign `fireball` to `weakItem`.
- **Key point:** Assigning to a `weak_ptr` does **NOT** increase the `use_count`. It remains **1**.
- `weak_ptr` watches the object but doesn't claim ownership.

```cpp
        if (auto locked = weakItem.lock())
        {
            cout << "Weak pointer locked, value: " << *locked << endl;
        }
```
- `weakItem.lock()` — Tries to get a `shared_ptr` from the `weak_ptr`. This temporarily becomes an owner.
- If the object still exists, `lock()` returns a valid `shared_ptr`.
- `auto locked` — `auto` automatically deduces the type (it becomes `shared_ptr<int>`).
- Inside the `if`, `*locked` dereferences to get the value (3 = Fireball).

```cpp
        cout << "Fireball going out of scope..." << endl;
    }
```
- The `}` closes the inner scope. `fireball` (the only `shared_ptr`) goes out of scope and is destroyed.
- Since there are no more `shared_ptr` owners (use count drops to 0), the Fireball memory is **automatically freed**.

```cpp
    cout << "After scope ended:" << endl;
    if (auto locked = weakItem.lock())
    {
        cout << "Weak pointer locked" << endl;
    }
    else
    {
        cout << "Weak pointer cannot be locked (item deleted)" << endl;
    }
}
```
- After the inner scope ends, we try to `lock()` the `weak_ptr` again.
- Since the `shared_ptr` that owned the Fireball was destroyed, the memory is gone.
- `weakItem.lock()` now returns an **empty (null) shared_ptr** — meaning the object no longer exists.
- The `else` branch executes: "Weak pointer cannot be locked (item deleted)".
- This shows that `weak_ptr` safely detects when the object is gone — no crash, no dangling pointer!

---

## 🔧 `main()` Function

```cpp
int size;
cout << "Enter size of Mario's Inventory: ";
cin >> size;
unique_ptr<int[]> ptr = make_unique<int[]>(size);
```
- `make_unique<int[]>(size)` — Creates a `unique_ptr` managing an integer array of `size` elements.
- This is the **modern, safe** equivalent of `int *ptr = new int[size];`.
- You **never need to call `delete[]`** — the `unique_ptr` does it automatically when it goes out of scope.

```cpp
int m = 0;
int choice;
do {
    // Show menu...
    cin >> choice;
    if (choice == 1) add(ptr, size, m);
    else if (choice == 2) remove(m);
    else if (choice == 3) display(ptr, m);
    else if (choice == 4) sharedDemo();
    else if (choice == 5) weakDemo();
    else break;
} while (choice != 6);
```
- Same menu structure as Task 1.
- Options 4 and 5 trigger the `shared_ptr` and `weak_ptr` demo functions.

```cpp
return 0;
```
- When `main()` ends, `ptr` (the `unique_ptr`) automatically destroys the array — no manual `delete[]` needed!

---

## 🧠 Smart Pointer Comparison Table

| Feature | `unique_ptr` | `shared_ptr` | `weak_ptr` |
|---|---|---|---|
| Ownership | One owner only | Multiple owners | No ownership |
| Can be copied? | ❌ No | ✅ Yes | ✅ Yes (from shared_ptr) |
| Tracks owners? | No | Yes (use_count) | Observes shared_ptr |
| Auto-deletes? | Yes (when owner dies) | Yes (when count = 0) | Never (doesn't own) |
| Can access value? | Yes | Yes | Only via `.lock()` |
| Use case | Exclusive resource | Shared resource | Avoiding circular refs |

---

## 🎯 Why Smart Pointers?

With **raw pointers** (Task 1 style):
```cpp
int *ptr = new int[size];
// ... if you forget this, memory leak!
delete[] ptr;
```

With **smart pointers** (Task 5 style):
```cpp
unique_ptr<int[]> ptr = make_unique<int[]>(size);
// Memory is AUTOMATICALLY freed when ptr goes out of scope
// No delete[] needed!
```

Smart pointers make C++ **safer** and **easier** to manage memory correctly.
