#pragma once

#include <QtCore/QString>
#include <QtCore/QPair>
#include <QtCore/QVector>

enum class GeneratorType
{
	None,
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

inline GeneratorType keyToType(const QString& key)
{
	for (const auto& desc : getGeneratorTypes())
	{
		if (desc.jsonKey_ == key)
		{
			return desc.type_;
		}
	}

	return GeneratorType::None;
}

inline QString typeToKey(GeneratorType type)
{
	for (const auto& desc : getGeneratorTypes())
	{
		if (desc.type_ == type)
		{
			return desc.jsonKey_;
		}
	}

	return "";
}
