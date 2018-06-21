
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main(void)
{
	char path[1024] = { 0 };
	snprintf(path, sizeof(path), ".\\aaa.txt");
	remove(path);
	FILE *f = fopen(path, "wb");

	char c = 0x99;
	const int sz = 71;
	fseek(f, sz - 1, SEEK_SET);
	fwrite(&c, 1, 1, f);
	fseek(f, 0, SEEK_SET);
	fwrite(&c, 1, 1, f);
	fclose(f);

	getchar();
	return 0;
}