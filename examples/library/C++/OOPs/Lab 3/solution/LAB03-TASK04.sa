#include <iostream>
using namespace std;
int main()
{

    char *ptr = new char[100];
    int size;
    cout << "Enter total items: ";
    cin >> size;
    cin.ignore();
    int *ptr1 = new int[size];
    char **names = new char *[size];

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

        ptr1[i] = n;

        names[i] = new char[n];
        for (int j = 0; j < n; j++)
        {
            names[i][j] = ptr[j];
        }
    }

    for (int i = 0; i < size; i++)
    {
        cout << "Name of item " << i + 1 << " is : ";
        for (int j = 0; j < ptr1[i]; j++)
        {
            cout << names[i][j];
        }
        cout << endl;
    }

    for (int i = 0; i < size; i++)
    {
        delete[] names[i];
    }
    delete[] names;
    delete[] ptr1;
    delete[] ptr;

    return 0;
}