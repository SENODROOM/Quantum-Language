#include <iostream>
#include <iomanip>
using namespace std;

class HeroMissionDate
{
private:
	int day;
	int month;
	int year;

public:
	HeroMissionDate()
	{
		day = 1;
		month = 1;
		year = 1926;
		cout << "Default constructor called!" << endl;
	}

	HeroMissionDate(int d, int m, int y)
	{
		day = d;
		month = m;
		year = y;
		cout << "Overloaded constructor called!" << endl;
	}

	~HeroMissionDate()
	{
		cout << "Saving Townsville ends... Destructor called!" << endl;
	}

	void Print() const
	{
		cout << setw(2) << day << "/" << setw(2) << month << "/" << setw(4) << year << endl;
	}

	void Input()
	{
		cout << "Enter day: ";
		cin >> day;
		cout << "Enter month: ";
		cin >> month;
		cout << "Enter year: ";
		cin >> year;
	}

	void SetYear(int y)
	{
		year = y;
	}
	int GetDay() const
	{
		return day;
	}
	int GetMonth() const
	{
		return month;
	}
	int Compare(const HeroMissionDate &otherDate) const
	{
		if (year > otherDate.year)
		{
			return 1;
		}
		if (year < otherDate.year)
		{
			return -1;
		}
		if (month > otherDate.month)
		{
			return 1;
		}
		if (month < otherDate.month)
		{
			return -1;
		}
		if (day > otherDate.day)
		{
			return 1;
		}
		if (day < otherDate.day)
		{
			return -1;
		}
		return 0;
	}
	HeroMissionDate IncrementMonth() const
	{
		int nd = day;
		int nm = month + 1;
		int ny = year;
		if (nm > 12)
		{
			nm = 1;
			ny += 1;
		}
		return HeroMissionDate(nd, nm, ny);
	}
};

int main()
{

	HeroMissionDate d1;
	cout << "Default date: ";
	d1.Print();

	HeroMissionDate d2(14, 9, 2026);
	cout << "Overloaded date: ";
	d2.Print();
	{
		HeroMissionDate temp(31, 12, 2025);
		cout << "Temporary: ";
		temp.Print();
	}

	HeroMissionDate d3;
	cout << "Input a date:\n";
	d3.Input();
	cout << "You entered: ";
	d3.Print();

	d3.SetYear(2030);
	cout << "After SetYear(2030): ";
	d3.Print();
	cout << "Day: " << d3.GetDay() << ", Month: " << d3.GetMonth() << endl;

	int cmp = d2.Compare(d3);
	if (cmp == 1)
	{
		cout << "d2 is later than d3" << endl;
	}
	else if (cmp == -1)
	{
		cout << "d2 is earlier than d3" << endl;
	}
	else
	{
		cout << "d2 and d3 are the same date" << endl;
	}

	HeroMissionDate d4 = d2.IncrementMonth();
	cout << "d2 incremented month -> ";
	d4.Print();
	cout << "d2 remains -> ";
	d2.Print();
	return 0;
}
