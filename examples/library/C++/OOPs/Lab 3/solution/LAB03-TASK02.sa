#include <iostream>
using namespace std;
void input(int *ptr, int row, int col)
{

    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            cout << "Enter element for row " << i + 1 << " and column " << j + 1 << ": ";
            cin >> *(ptr + i * col + j);
        }
    }
}
void display(int *ptr, int row, int col)
{
    cout << "The elements of the array are: " << endl;
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            cout << *(ptr + i * col + j) << " ";
        }
        cout << endl;
    }
}
void transpose(int *ptr, int row, int col)
{
    cout << "The transpose of the array is: " << endl;
    for (int i = 0; i < col; i++)
    {
        for (int j = 0; j < row; j++)
        {
            cout << *(ptr + j * col + i) << " ";
        }
        cout << endl;
    }
}
int main()
{
    int row, col;
    cout << "Enter size of row and col: ";
    cin >> row >> col;
    int *ptr = new int[row * col];
    input(ptr, row, col);
    display(ptr, row, col);
    transpose(ptr, row, col);
    delete[] ptr;
    return 0;
}