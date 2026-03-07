// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<iostream>
using namespace std;

int* referenceBooster(int &p);


int main()
{
	int a;
	cout << "Enter a: ";
	cin >> a; 

	cout << "ptr= " << referenceBooster(a) << endl;
	cout << "a= " << a << endl;


	system("pause");
	return 0;
}

int* referenceBooster(int &p){
	p += 10;
	int *ptr = &p;
	return ptr;
}