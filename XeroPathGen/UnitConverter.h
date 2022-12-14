#pragma once

#include <QtCore/QString>
#include <QtCore/QList>

class UnitConverter
{
public:
	UnitConverter() = delete;
	~UnitConverter() = delete;

	/// \brief convert a double value from one set of units to another
	/// \param value the value to convert
	/// \param from the units to convert from
	/// \param to the units to convert to
	/// \returns value converted to new units
	static double convert(double value, const QString& from, const QString& to);

	/// \brief convert a double value from one set of units to another
	/// \param value the value to convert
	/// \param from the units to convert from
	/// \param to the units to convert to
	/// \return value converted to new units
	static float convert(float value, const QString& from, const QString& to);

	/// \brief returns information about whether a conversion is possible
	/// \param from the units to convert from
	/// \param to the units to convert to
	/// \returns true if the conversion is possible, false otherwise
	static bool hasConversion(const QString& from, const QString& to);

	/// \brief returns all units the converter can process
	/// \returns a list of all units the converter can process
	static QList<QString> getAllLengthUnits();


	/// \brief returns all units the converter can process
	/// \returns a list of all units the converter can process
	static QList<QString> getAllWeightUnits();

private:
	static QString normalizeUnits(const QString& units);

private:
	enum class UnitType
	{
		Length,
		Weight
	};

private:
	struct conversion
	{
		const char* from;
		const char* to;
		double conversion;
		UnitType type_;
	};

	static bool findConversion(const QString& from, const QString& to, double& conversion);
	static struct conversion convert_[];
};

