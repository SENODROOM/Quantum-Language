// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<iostream>
using namespace std;


int main()
{
	int a,b,c;
	cout << "Enter a: ";
	cin >> a; 
	cout << "Enter b: ";
	cin >> b;
	cout << "Enter c: ";
	cin >> c;

	const int* pointer1 = &a;
	int* const pointer2 = &b;
	const int* const pointer3 = &c;

	// *pointer1 = 25;   // invalid
	pointer1 = pointer2; // valid
	cout << pointer1 << endl;

	*pointer2 = 25;   // invalid
	// *pointer2 = pointer3; // valid
	cout << *pointer2 << endl;

	// *pointer3 = 25;   // invalid
	// pointer3 = pointer2; // valid

	system("pause");
	return 0;
}
