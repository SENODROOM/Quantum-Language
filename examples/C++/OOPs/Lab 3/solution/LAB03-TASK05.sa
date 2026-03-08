#include <iostream>
#include <memory>
using namespace std;

void add(unique_ptr<int[]> &ptr, int &size, int &m)
{
    if (m < size)
    {
        cout << "Enter item code (1=Mushroom, 2=Star, 3=Fireball): ";
        cin >> ptr[m];
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

void display(unique_ptr<int[]> &ptr, int &m)
{
    if (m == 0)
    {
        cout << "No item found" << endl;
    }
    else
    {
        cout << "Mario's Inventory" << endl;
        for (int i = 0; i < m; i++)
        {
            cout << "Item " << i + 1 << " is ";
            if (ptr[i] == 1)
                cout << "Mushroom";
            else if (ptr[i] == 2)
                cout << "Star";
            else if (ptr[i] == 3)
                cout << "Fireball";
            cout << endl;
        }
    }
}

void sharedDemo()
{
    cout << "\nShared Ownership Demo" << endl;

    shared_ptr<int> mushroom = make_shared<int>(1);
    shared_ptr<int> star = make_shared<int>(2);

    cout << "Mushroom use count: " << mushroom.use_count() << endl;
    cout << "Star use count: " << star.use_count() << endl;

    shared_ptr<int> backpack = mushroom;
    shared_ptr<int> powerupList = mushroom;

    cout << "After sharing Mushroom:" << endl;
    cout << "Mushroom use count: " << mushroom.use_count() << endl;
    cout << "Star use count: " << star.use_count() << endl;
}

void weakDemo()
{
    cout << "\nWeak Ownership Demo" << endl;

    weak_ptr<int> weakItem;

    {
        shared_ptr<int> fireball = make_shared<int>(3);
        cout << "Fireball use count: " << fireball.use_count() << endl;

        weakItem = fireball;
        cout << "After creating weak_ptr, Fireball use count: " << fireball.use_count() << endl;

        if (auto locked = weakItem.lock())
        {
            cout << "Weak pointer locked, value: " << *locked << endl;
        }

        cout << "Fireball going out of scope..." << endl;
    }

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

int main()
{
    int size;
    cout << "Enter size of Mario's Inventory: ";
    cin >> size;
    unique_ptr<int[]> ptr = make_unique<int[]>(size);
    int m = 0;
    int choice;
    do
    {
        cout << "\n1. Add item\n2. Remove item\n3. Display inventory\n4. Shared ownership demo\n5. Weak ownership demo\n6. Exit" << endl;
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
            sharedDemo();
        }
        else if (choice == 5)
        {
            weakDemo();
        }
        else
        {
            break;
        }
    } while (choice != 6);

    return 0;
}