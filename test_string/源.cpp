#include <string>
#include <iostream>

int main(void)
{

	/* wrong ! raise an exception */
	std::string s(nullptr);

	std::cout << s << std::endl;

	return 0;
}