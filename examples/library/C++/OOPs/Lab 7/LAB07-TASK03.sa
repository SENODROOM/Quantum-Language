// ConsoleApplication4.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
using namespace std;
class Beyblade{
private:
	int beybladeId;
	float powerLevel;
public:
	Beyblade(){
		beybladeId = 0;
		powerLevel = 0;
	}

	Beyblade(int id,float power){
		beybladeId = id;
		powerLevel = power;
	}

	Beyblade(Beyblade &other){
		beybladeId = other.beybladeId;
		powerLevel = other.powerLevel;
	}

	void setter(int valueID, float valuelevel){
		beybladeId = valueID;
		powerLevel = valuelevel;
	}

	int getID(){
		return beybladeId;
	}

	int getLvl(){
		return powerLevel;
	}

	void display(){
		cout << "beybladeID: " << beybladeId<<endl;
		cout << "powerlevel: " << powerLevel<<endl;
	}


};

int main(){
	int num;
	cout << "Enter the number of Beyblades: ";
	cin >> num;
	Beyblade* test1 = new Beyblade[num];

	for (int i = 0; i < num; i++)
	{
		int id;
		float lvl;
		cout << "Enter the id of Bayblade: ";
		cin >> id;
		cout << "Enter the lvl of Bayblade: ";
		cin >> lvl;
		test1[i].setter(id, lvl);
	}

	for (int i = 0; i < num; i++)
	{
		test1[i].display();
	}


	system("pause");
	return 0;
}

