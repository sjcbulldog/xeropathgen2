#pragma once

#include <QtCore/qstring.h>

class ICsv
{
public:
	virtual double getField(const QString& field) const = 0;
};
