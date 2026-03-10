// ConsoleApplication4.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
using namespace std;

class Beyblade{
private:
	int beybladeId;

public:

	float powerLevel;
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
	float Beyblade::* ptrtest = &Beyblade::powerLevel;

	Beyblade b1;
	b1.*ptrtest = 90.5;

	Beyblade* bp = &b1;
	bp->*ptrtest = 120.0;

	b1.display();

	system("pause");
	return 0;
}

