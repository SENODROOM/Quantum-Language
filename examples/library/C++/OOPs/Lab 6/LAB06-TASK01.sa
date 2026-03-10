// ConsoleApplication3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
using namespace std;

class TownsvilleGuardian
{
private:
	string heroName;
	int energyLevel;
	int villainsDefeated;
	string superPower;

public:
	TownsvilleGuardian()
	{
		heroName = "New Recruit";
		energyLevel = 60;
		villainsDefeated = 0;
		superPower = "Chemical X Burst";

		cout << "Default constructor called!" << endl;
	}
	TownsvilleGuardian(string name, int energy, string power)
	{
		heroName = name;
		energyLevel = (energy >= 0 && energy <= 100) ? energy : 60;
		villainsDefeated = 0;
		superPower = power;

		cout << "Overloaded constructor called!" << endl;
	}
	~TownsvilleGuardian()
	{
		cout << "Guardian signing off... Townsville is safe!" << endl;
	}
	string getHeroName() const { return heroName; }
	int getEnergyLevel() const { return energyLevel; }
	int getVillainsDefeated() const { return villainsDefeated; }
	void setEnergyLevel(int energy)
	{
		if (energy >= 0 && energy <= 100)
			energyLevel = energy;
	}
	void defeatVillain() { villainsDefeated += 1; }
	void displayStatus() const
	{
		cout << "Hero: " << heroName << endl;
		cout << "Energy Level: " << energyLevel << endl;
		cout << "Villains Defeated: " << villainsDefeated << endl;
		cout << "Super Power: " << superPower << endl;
	}
	void battle(int villainStrength)
	{
		if (energyLevel > villainStrength)
		{
			cout << "Villain defeated!" << endl;
			defeatVillain();
			energyLevel -= 10;
			if (energyLevel < 0)
				energyLevel = 0;
		}
		else
		{
			cout << "Retreat! Call Professor Utonium!" << endl;
		}
	}
	void recharge()
	{
		energyLevel += 15;
		if (energyLevel > 100)
			energyLevel = 100;
	}
	int compareEnergy(const TownsvilleGuardian &otherGuardian) const
	{
		if (energyLevel > otherGuardian.energyLevel)
			return 1;
		if (energyLevel < otherGuardian.energyLevel)
			return -1;
		return 0;
	}
	TownsvilleGuardian nextBattleReady() const
	{
		int newEnergy = energyLevel + 5;
		if (newEnergy > 100)
			newEnergy = 100;
		return TownsvilleGuardian(heroName, newEnergy, superPower);
	}
};

int main()
{
	TownsvilleGuardian g1;
	TownsvilleGuardian g2("Blossom", 85, "Ice Breath");
	cout << "Default Guardian Status:\n";
	g1.displayStatus();
	cout << "Overloaded Guardian Status:\n";
	g2.displayStatus();
	cout << "Accessor test: " << g2.getHeroName() << ", " << g2.getEnergyLevel() << ", " << g2.getVillainsDefeated() << endl;
	g2.setEnergyLevel(95);
	cout << "After setEnergyLevel(95): " << g2.getEnergyLevel() << endl;
	g2.defeatVillain();
	cout << "After defeatVillain(): " << g2.getVillainsDefeated() << endl;
	g2.battle(80);
	cout << "After battle(80): " << g2.getEnergyLevel() << ", Villains Defeated: " << g2.getVillainsDefeated() << endl;
	g2.battle(100);
	g2.recharge();
	cout << "After recharge(): " << g2.getEnergyLevel() << endl;
	int cmp = g2.compareEnergy(g1);
	if (cmp == 1)
	{
		cout << "g2 has more energy than g1" << endl;
	}
	else if (cmp == -1)
	{
		cout << "g2 has less energy than g1" << endl;
	}
	else
	{
		cout << "g2 and g1 have equal energy" << endl;
	}
	TownsvilleGuardian g3 = g2.nextBattleReady();
	cout << "Next battle ready guardian (g3):\n";
	g3.displayStatus();
	system("pause");
	return 0;
}
