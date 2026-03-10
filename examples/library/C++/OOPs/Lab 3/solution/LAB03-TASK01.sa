#include <iostream>
using namespace std;

void add(int *ptr, int &size, int &m)
{
    if (m < size)
    {
        cout << "Enter number: ";
        cin >> *(ptr + m);
        m++;
    }
    else
    {
        cout << "Inventory is full!" << endl;
    }
}

void remove(int &m)
{
    if (m == 0)
    {
        cout << "No item found" << endl;
    }
    else
    {
        m--;
        cout << "Deleted....." << endl;
    }
}

void display(int *ptr, int &m)
{
    if (m == 0)
    {
        cout << "No item found" << endl;
    }
    else
    {
        cout << "Marios's Inventory" << endl;
        for (int i = 0; i < m; i++)
        {
            cout << "Value " << i + 1 << " is " << *(ptr + i) << endl;
        }
    }
}

void shrink(int *&ptr, int &size, int &m)
{
    int newsize;
    cout << "Enter the new size: ";
    cin >> newsize;
    int *temp = new int[newsize];
    cout << "Shrinked array: ";
    for (int i = 0; i < newsize; i++)
    {
        *(temp + i) = *(ptr + i);
        cout << *(temp + i) << " ";
    }
    cout << endl;
    delete[] ptr;
    ptr = temp;
    size = newsize;
    m = (m > newsize) ? newsize : m;
}

void regrow(int *&ptr, int &size, int &m)
{
    int newsize;
    cout << "Enter the new size: ";
    cin >> newsize;
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
    cout << "Regrow Inventory: ";
    for (int i = 0; i < m; i++)
    {
        cout << *(ptr + i) << " ";
    }

    cout << endl;
}

int main()
{
    int size;
    cout << "Enter size of Mariao's Inventory: ";
    cin >> size;
    int *ptr = new int[size];
    int m = 0;
    int choice;
    do
    {
        cout << "1. Add item (add an integer to the array if space is available) \n2. Remove item(remove the last item if the bag is not empty)\n3. Display inventory(show all items)\n4. Shrink inventory (reduce array size, preserving existing items if possible)\n5. Regrow inventory (increase array size, allowing for more items)\n6. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;
        if (choice == 1)
        {
            add(ptr, size, m);
        }
        else if (choice == 2)
        {
            remove(m);
        }
        else if (choice == 3)
        {
            display(ptr, m);
        }
        else if (choice == 4)
        {
            shrink(ptr, size, m);
        }
        else if (choice == 5)
        {
            regrow(ptr, size, m);
        }
        else
        {
            break;
        }
    } while (choice != 6);
    ptr = nullptr;
    delete[] ptr;
    system("pause");
    return 0;
}