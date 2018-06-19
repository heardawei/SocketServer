#include <iostream>
//#include <locale.h>
#include <string>
#include <stdlib.h>

using namespace std;

string wstr_2_str(const wstring &ws)
{
	// curLocale="C"
//	string curLocale = setlocale(LC_ALL, NULL);
//	setlocale(LC_ALL, "chs");

	size_t convertedChars = 0;

	const wchar_t *wcs = ws.c_str();
	size_t dByteNum = sizeof(wchar_t) * ws.size() + 1;
	cout << "ws.size():" << ws.size() << endl;

	char* dest = new char[dByteNum];
	wcstombs_s(&convertedChars, dest, dByteNum, wcs, _TRUNCATE);
	cout << "convertedChars:" << convertedChars << endl;

	string result = dest;

	delete[] dest;

//	setlocale(LC_ALL, curLocale.c_str());

	return result;
}

wstring str_2_wstr(const string &s)
{
	// curLocale="C"
//	string curLocale = setlocale(LC_ALL, NULL);
//	setlocale(LC_ALL, "chs");

	size_t convertedChars = 0;
	const char* source = s.c_str();
	size_t charNum = sizeof(char) * s.size() + 1;
	cout << "s.size():" << s.size() << endl;

	wchar_t* dest = new wchar_t[charNum];
	mbstowcs_s(&convertedChars, dest, charNum, source, _TRUNCATE);
	cout << "str_2_wstr_convertedChars:" << convertedChars << endl; //6

	wstring result = dest;

	delete[] dest;

//	setlocale(LC_ALL, curLocale.c_str());

	return result;
}

int main()
{
	const wchar_t *wstr = L"ABCDEFG";
	const char *str = "ABCDEFG";

	string str_obj = wstr_2_str(wstr);
	cout << "wide_str [" << wstr << "] trans to -> str [" << str_obj << "]" << endl;
//	cout << str_obj << endl;

//	setlocale(LC_ALL, "chs");

	wstring wstr_obj = str_2_wstr(str);
	cout << "str [" << str << "] trans to -> wide_str [" << wstr_obj.data() << "]" << endl;
//	wcout << wstr_obj << endl;
	getchar();
	return 0;
}