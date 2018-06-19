#include <iostream>
#include <string>
#include <map>

using namespace std;

class A
{
public:
	virtual void print()
	{
		cout << "I'm A" << endl;
	}
};

class B :public A
{
public:
	virtual void print()
	{
		cout << "I'm B" << endl;
	}
};

int main(void)
{
	map<int, A *> m;
	B b1;
	m[1] = &b1;

	m[1]->print();

	getchar();
	return 0;
}