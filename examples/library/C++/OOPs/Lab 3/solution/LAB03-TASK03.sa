#include <iostream>
using namespace std;
void diagonal(int *ptr, int row, int col)
{
    cout << "Transverse diagonal  of garden is : ";
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j <= i; j++)
        {
            cout << *(ptr + (i - j) * (col) + j) << " ";
        }
    }

    for (int i = 1; i < col; i++)
    {
        int n = i;

        for (int j = row - 1; j > (i - 1); j--)
        {
            cout << *(ptr + (j) * (col) + n) << " ";
            n++;
        }
    }

    cout << endl;
}
int main()
{
    int row, col;
    cout << "Enter number of rows in garden : ";
    cin >> row;
    cout << "Enter number of columns in garden: ";
    cin >> col;
    if (col < row)
    {
        cout << "Number of columns should be greater than  number of rows." << endl;
        return 0;
    }
    int *ptr = new int[row * col];
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            cout << "Enter element for row " << i + 1 << " and column " << j + 1 << ": ";
            cin >> *(ptr + i * col + j);
        }
    }
    diagonal(ptr, row, col);
    delete[] ptr;
    return 0;
}