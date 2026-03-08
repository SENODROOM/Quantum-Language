#include <iostream>
using namespace std;

int main()
{
    int n;
    cout << "Enter number of snacks: ";
    cin >> n;

    float* snacks = new float[n];
    float* cumulative = new float[n];

    for (int i = 0; i < n; i++)
        cin >> snacks[i];

    cumulative[0] = snacks[0];
    for (int i = 1; i < n; i++) {
        cumulative[i] = cumulative[i - 1] + snacks[i];
    }

    cout << "Cumulative Power:\n";
    for (int i = 0; i < n; i++)
        cout << cumulative[i] << " ";

    delete[] snacks;
    delete[] cumulative;
    return 0;
}