#include "BasicRules.h"

BasicRules::BasicRules(const string& re, const string& RuleName)
{
	this->re = regex(re);
	this->RuleName = RuleName;
}
bool BasicRules::isMatch(const string& input) const
{
	return regex_match(input,re);
}
