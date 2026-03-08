#include <iostream>
using namespace std;

void bubbleSort(float* arr, int n);
int main()
{
    int n;
    cout << "Enter number of moods: ";
    cin >> n;

    float* moods = new float[n];

    for (int i = 0; i < n; i++)
        cin >> moods[i];

    bubbleSort(moods, n);

    cout << "Sorted moods:\n";
    for (int i = 0; i < n; i++)
        cout << moods[i] << " ";

    delete[] moods;
    return 0;
}

void bubbleSort(float* arr, int n)
{
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]);
            }
        }
    }
}