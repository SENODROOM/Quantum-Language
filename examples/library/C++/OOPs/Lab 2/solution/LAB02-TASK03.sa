#include <iostream>
using namespace std;

int length(const char* str)
{
    int len = 0;
    while (str[len] != '\0')
        len++;
    return len;
}

void concat(const char* src, char* dest)
{
    int srcLen = length(src);
    int destLen = length(dest);

    for (int i = destLen; i >= 0; i--) {
        dest[i + srcLen + 1] = dest[i];
    }

    for (int i = 0; i < srcLen; i++) {
        dest[i] = src[i];
    }

    dest[srcLen] = ' ';

    cout << dest << endl;
}

int main()
{
    char src[] = "SpongeBob";
    char dest[50] = "SquarePants";

    concat(src, dest);
    return 0;
}