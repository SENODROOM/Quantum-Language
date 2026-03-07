// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<iostream>
using namespace std;


int main()
{
	// DANGLING POINTER:
	int* a = new int(100);
	int* b = a;
	delete a; // a is now dangling pointer
	// cout <<"*a: "<< *a <<endl;
	cout << "a: " << a << endl;
	a = nullptr; // a is now no longer dangling pointer
	cout << a << endl;

	// MEMORY LEAK:
	int* c = new int(200);
	delete c; // to avoid memory leak must delete the variable value
	c = new int(300);
	cout << *c<<endl;
	system("pause");
	return 0;
}
