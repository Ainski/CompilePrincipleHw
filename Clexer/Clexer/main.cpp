#include<iostream>
#include<regex>
using namespace std;

int main()
{
	regex re("\\d*");

	string test = "";

	if (regex_match(test, re))
		cout << "Æ¥Åä³É¹¦\n";
	else
		cout << "Æ¥ÅäÊ§°Ü\n";
	return 0;
}