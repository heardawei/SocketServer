#include <vector>
#include <time.h>
#include <stdlib.h>

static std::vector<std::string> vs;

int main(void)
{
	srand(time(NULL));
	for (int i = 0; i < 40; i++)
	{
		int r = rand() % 10 + 10;
		while (r--)
		{
			vs.push_back(std::string("hello world"));
			printf("push\n");
		}
		for (auto itr = vs.begin(); itr != vs.end(); )
		{
			if (vs.size() > 100)
			{
				itr = vs.erase(itr);
				printf("erase\n");
				continue;
			}

			int r = rand() % 2;
			if (r)
			{
				itr = vs.erase(itr);
				printf("erase\n");
				continue;
			}
		}
	}
	getchar();
	return 0;
}