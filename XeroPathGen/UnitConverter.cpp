//
// Copyright 2022 Jack W. Griffin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.
//
#include "UnitConverter.h"
#include <stdexcept>
#include <algorithm>

UnitConverter::conversion UnitConverter::convert_[] =
{
	{ "ft", "in", 12.0, UnitConverter::UnitType::Length},
	{ "in", "cm", 2.54, UnitConverter::UnitType::Length},
	{ "in", "m", 0.0254, UnitConverter::UnitType::Length},
	{ "m", "ft", 3.28084, UnitConverter::UnitType::Length},
	{ "ft", "cm", 30.48, UnitConverter::UnitType::Length},
	{ "m", "cm", 100.0, UnitConverter::UnitType::Length},

	{ "lbs", "kg", 0.453592, UnitConverter::UnitType::Weight},
};

QString UnitConverter::normalizeUnits(const QString& units)
{
	QString ret = units;

	if (units == "meters")
		ret = "m";
	else if (units == "feet")
		ret = "ft";
	else if (units == "foot")
		ret = "ft";
	else if (units == "inches")
		ret = "in";

	return ret;
}

bool UnitConverter::findConversion(const QString& from, const QString& to, double& conversion)
{
	bool ret = false;

	QString fromnorm = normalizeUnits(from);
	QString tonorm = normalizeUnits(to);

	if (fromnorm == tonorm)
	{
		conversion = 1.0;
		ret = true;
	}
	else
	{
		for (int i = 0; i < sizeof(convert_) / sizeof(convert_[0]); i++)
		{
			if (fromnorm == convert_[i].from && tonorm == convert_[i].to)
			{
				conversion = convert_[i].conversion;
				ret = true;
				break;
			}

			if (tonorm == convert_[i].from && fromnorm == convert_[i].to)
			{
				conversion = 1.0 / convert_[i].conversion;
				ret = true;
				break;
			}
		}
	}

	return ret;
}

double UnitConverter::convert(double value, const QString& from, const QString& to)
{
	double conv;

	if (from == to)
		return value;

	if (!findConversion(from, to, conv))
	{
		QString msg = "no conversion from '";
		msg += from;
		msg += "' to '";
		msg += to;
		msg += "'";
		throw std::runtime_error(msg.toStdString());
	}

	return value * conv;
}

float UnitConverter::convert(float value, const QString& from, const QString& to)
{
	double conv;

	if (!findConversion(from, to, conv))
	{
		QString msg = "no conversion from '";
		msg += from;
		msg += "' to '";
		msg += to;
		msg += "'";
		throw std::runtime_error(msg.toStdString());
	}

	return static_cast<float>(value * conv);
}

bool UnitConverter::hasConversion(const QString& from, const QString& to)
{
	double conv;
	return findConversion(from, to, conv);
}

QList<QString> UnitConverter::getAllLengthUnits()
{
	QList<QString> result;

	for (int i = 0; i < sizeof(convert_) / sizeof(convert_[0]); i++)
	{
		if (convert_[i].type_ == UnitType::Length)
		{
			if (std::find(result.begin(), result.end(), convert_[i].to) == result.end())
				result.push_back(convert_[i].to);
			if (std::find(result.begin(), result.end(), convert_[i].from) == result.end())
				result.push_back(convert_[i].from);
		}
	}

	return result;
}

QList<QString> UnitConverter::getAllWeightUnits()
{
	QList<QString> result;

	for (int i = 0; i < sizeof(convert_) / sizeof(convert_[0]); i++)
	{
		if (convert_[i].type_ == UnitType::Weight)
		{
			if (std::find(result.begin(), result.end(), convert_[i].to) == result.end())
				result.push_back(convert_[i].to);
			if (std::find(result.begin(), result.end(), convert_[i].from) == result.end())
				result.push_back(convert_[i].from);
		}
	}

	return result;
}
