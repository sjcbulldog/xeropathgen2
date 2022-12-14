#pragma once

#include <string>

class ICsv
{
public:
	virtual double getField(const std::string& field) const = 0;
};
