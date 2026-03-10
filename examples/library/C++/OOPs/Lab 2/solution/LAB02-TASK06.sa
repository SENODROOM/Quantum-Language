#include <iostream>
using namespace std;

int main()
{
    int size;
    cout << "Enter initial size: ";
    cin >> size;

    int* arr = new int[size];
    int count = 0;

    while (true)
    {
        int id;
        cin >> id;

        if (id == -1)
            break;

        if (count == size)
        {
            int* newArr = new int[size * 2];
            for (int i = 0; i < size; i++)
                newArr[i] = arr[i];

            delete[] arr;
            arr = newArr;
            size *= 2;
        }

        arr[count++] = id;
    }

    cout << "Jellyfish IDs:\n";
    for (int i = 0; i < count; i++)
        cout << arr[i] << " ";

    delete[] arr;
    return 0;
}