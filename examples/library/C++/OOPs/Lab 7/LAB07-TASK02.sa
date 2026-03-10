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
	Beyblade test1;
	Beyblade* ptrtest2 = new Beyblade(12,12.56);

	test1.display();

	ptrtest2->display();
	delete ptrtest2;
	ptrtest2 = nullptr;
	system("pause");
	return 0;
}

