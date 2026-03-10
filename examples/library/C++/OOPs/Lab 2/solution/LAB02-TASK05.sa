#include <iostream>
using namespace std;

int main()
{
    int n;
    cout << "Enter size: ";
    cin >> n;

    int* arr = new int[n];
    for (int i = 0; i < n; i++)
        cin >> arr[i];

    for (int i = 0; i < n; i++)
    {
        for (int j = i + 1; j < n; )
        {
            if (arr[i] == arr[j])
            {
                for (int k = j; k < n - 1; k++)
                    arr[k] = arr[k + 1];
                n--;
            }
            else
                j++;
        }
    }

    cout << "After removing duplicates:\n";
    for (int i = 0; i < n; i++)
        cout << arr[i] << " ";

    delete[] arr;
    return 0;
}