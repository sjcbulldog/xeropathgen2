#pragma once

#include <QtCore/QString>
#include <QtCore/QPair>
#include <QtCore/QVector>

enum class GeneratorType
{
	CheesyPoofs,
	ErrorCodeXeroSwerve
};

struct GeneratorDescriptor
{
	QString jsonKey_;
	QString desc_;
	GeneratorType type_;

	GeneratorDescriptor(GeneratorType type, const QString& key, const QString& desc) {
		type_ = type;
		jsonKey_ = key;
		desc_ = desc;
	}
};

inline QVector<GeneratorDescriptor> getGeneratorTypes()
{
	QVector<GeneratorDescriptor> ret;

	ret.push_back(GeneratorDescriptor(GeneratorType::CheesyPoofs, "cheesy", "Cheesy Poofs Tank Drive Generator"));
	ret.push_back(GeneratorDescriptor(GeneratorType::ErrorCodeXeroSwerve, "errorcodexeroswerve", "Error Code Xero Swerve Drive Generator"));

	return ret;
}
