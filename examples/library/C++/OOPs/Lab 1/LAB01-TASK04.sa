// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<iostream>
using namespace std;

int* resetToZero(int* p);


int main()
{
	int a;
	cout << "Enter a: ";
	cin >> a;
	int *ptr = &a;
	int *newptr =resetToZero(ptr);

	cout << "newptr= " << *newptr << endl;



	system("pause");
	return 0;
}

int* resetToZero(int* p){
	*p = 0;
	return p;
}