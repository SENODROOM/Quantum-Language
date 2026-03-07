// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<iostream>
using namespace std;

void cubeByPtr(int* p);
void cubeByRef(int& r);


int main()
{
	int a,b;
	cout << "Enter a: ";
	cin >> a; 
	cout << "Enter b: ";
	cin >> b;

	int *ptr = &a;

	cubeByPtr(ptr);
	cubeByRef(b);

	cout << "a= " << a << endl;
	cout << "b= " << b << endl;


	system("pause");
	return 0;
}

void cubeByPtr(int* p){
	*p = (*p)*(*p)*(*p);
}
void cubeByRef(int& r){
	r = r*r*r;
}