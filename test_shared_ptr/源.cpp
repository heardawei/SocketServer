#include <memory>
#include <map>
#include <iostream>

class A
{
public:
	A() { std::cout << "A" << std::endl; }
	A(int b) { std::cout << "A(" << b << ")" << std::endl; }

	std::map<int, std::shared_ptr<A>> m;

};

class B : public A
{
public:
	B() : A() { std::cout << "B" << std::endl; }
	B(int b) : A(b) { std::cout << "B(" << b << ")" << std::endl; }
	void assign()
	{
		std::shared_ptr<B> ptr(this);
	}
};

int main(void)
{
	std::map<int, std::shared_ptr<A>> m;
	std::cout << "-----------------------" << std::endl;
	A *b1 = new B(3);
	std::cout << "-----------------------" << std::endl;
	std::shared_ptr<A> ptr(b1);
	std::cout << "-----------------------" << std::endl;
	m[2] = ptr;
	std::cout << "-----------------------" << std::endl;

	getchar();
	return 0;
}