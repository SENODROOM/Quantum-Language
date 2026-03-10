// ConsoleApplication3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
using namespace std;
class MysteryCase
{
private:
	int caseID;
	float rewardAmount;
public:
	MysteryCase(): caseID(2),rewardAmount(2.3f){
		cout << "Default Object is called \n";
	}
	MysteryCase(int id,float amount) : caseID(2), rewardAmount(2.3f){
		cout << "Parameterized Object is called \n";
	}
	MysteryCase(const MysteryCase& p) : caseID(p.caseID), rewardAmount(p.rewardAmount){
		cout << "Copy Object is called \n";
	}

	void display(){
		cout << "caseID: " << caseID << " reawrdAmount: " << rewardAmount<<endl;
	}
	~MysteryCase(){
		cout << "Destructor is called \n";
	}
};

int main(){
	MysteryCase case1;
	case1.display();

	MysteryCase case2(23,2.3f);
	case2.display();
	{
	MysteryCase case3(case2);
	case3.display();
	}

	system("pause");
	return 0;
}

