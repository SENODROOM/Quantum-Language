#include <iostream>
using namespace std;

float average(const float* arr, int size);
int main()
{
    int n;
    cout << "Enter number of elements: ";
    cin >> n;

    float* arr = new float[n];

    for (int i = 0; i < n; i++)
    {
        cout << "Enter value " << i + 1 << ": ";
        cin >> arr[i];
    }

    cout << "Average = " << average(arr, n) << endl;

    delete[] arr;
    return 0;
}

float average(const float* arr, int size)
{
    float sum = 0;
    for (int i = 0; i < size; i++)
        sum += arr[i];

    return sum / size;
}