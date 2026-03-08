# LAB01 - TASK 03: String Concatenation (SpongeBob SquarePants)
## Line-by-Line Deep Explanation

---

## 📌 What Does This Program Do?
This program **prepends** one string onto another without using any built-in string functions like `strlen` or `strcat`. It manually finds string lengths and moves characters around to combine "SpongeBob" and "SquarePants" into "SpongeBob SquarePants".

---

## 🔧 Headers and Namespace

```cpp
#include <iostream>
using namespace std;
```
- Standard setup for I/O operations.

---

## 🔧 Function: `length(const char* str)`

```cpp
int length(const char* str)
{
    int len = 0;
    while (str[len] != '\0')
        len++;
    return len;
}
```
- This is a **manual replacement for `strlen()`** — counts how many characters are in a C-style string.
- `const char* str` — A pointer to a character array (C-string). `const` means we won't modify it.
- `str[len] != '\0'` — Every C-string ends with a null terminator `'\0'` (a special character with value 0 that marks the end). The loop keeps going until it hits this character.
- Each iteration increments `len`.
- Returns the count of characters (not including the `'\0'`).

**Example:** For `"SpongeBob"` → returns `9`.

---

## 🔧 Function: `concat(const char* src, char* dest)`

This function **prepends `src` to the front of `dest`**, adding a space between them. So `src = "SpongeBob"` and `dest = "SquarePants"` becomes `dest = "SpongeBob SquarePants"`.

```cpp
void concat(const char* src, char* dest)
{
    int srcLen = length(src);
    int destLen = length(dest);
```
- Get the length of both strings using our custom `length()` function.
- `srcLen = 9` (SpongeBob), `destLen = 11` (SquarePants).

```cpp
    for (int i = destLen; i >= 0; i--) {
        dest[i + srcLen + 1] = dest[i];
    }
```
- **Shift `dest` to the right** to make room for `src` + a space at the front.
- We shift every character (including the `'\0'`) rightward by `srcLen + 1` positions (`+1` for the space).
- We go **backwards** (`i = destLen` down to `0`) to avoid overwriting characters before they've been moved.
  - If we went forward, we'd overwrite characters we still need to move.

**Before shift:** `dest = ['S','q','u','a','r','e','P','a','n','t','s','\0']`
**After shift:** positions 10–22 now hold SquarePants + `'\0'`, positions 0–9 are "free".

```cpp
    for (int i = 0; i < srcLen; i++) {
        dest[i] = src[i];
    }
```
- Copy each character of `src` into the now-empty front of `dest`.
- After this: `dest = ['S','p','o','n','g','e','B','o','b', ?, 'S','q',...,'s','\0']`

```cpp
    dest[srcLen] = ' ';
```
- Place a space character at position `srcLen` (right after "SpongeBob" and before "SquarePants").
- Now `dest = "SpongeBob SquarePants"`.

```cpp
    cout << dest << endl;
}
```
- Print the final concatenated string.

---

## 🔧 `main()` Function

```cpp
char src[] = "SpongeBob";
char dest[50] = "SquarePants";
```
- `char src[]` — A character array initialized with "SpongeBob". The compiler automatically sizes it to 10 (9 chars + `'\0'`).
- `char dest[50]` — A character array with **50 slots** initialized with "SquarePants". We need the extra space because `concat` will write more characters into it (SpongeBob + space + SquarePants = 21 chars + `'\0'`). Without enough space, writing past the array would be undefined behavior (memory corruption).

```cpp
concat(src, dest);
return 0;
```
- Call `concat` and let it do the work. Program ends after printing.

---

## ⚠️ Important Notes

- `dest` must be large enough to hold the combined result. If `dest[50]` were too small, the program would write into memory it doesn't own — a **buffer overflow** bug.
- The backward-shifting loop (`i = destLen` down to `0`) is essential. Going forward would corrupt data.
- We include `'\0'` in the shift (`i >= 0` includes index `destLen` which holds `'\0'`). This ensures the final string is still properly null-terminated.

---

## 🧠 Key Concepts Used

| Concept | Where Used |
|---|---|
| C-style strings (char arrays) | `src[]`, `dest[]` |
| Null terminator `'\0'` | End-of-string detection in `length()` |
| Manual string length | Custom `length()` function |
| Backward shifting | Making room in `dest` without overwriting |
| String prepending logic | `concat()` function |
| Buffer sizing | `dest[50]` must fit both strings |
