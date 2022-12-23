#pragma once
#include "UndoAction.h"
#include <QtCore/QString>

class PathsDataModel;

class UndoSetUnits : public UndoAction
{
public:
	UndoSetUnits(const QString &units, PathsDataModel &model) : model_(model) {
		units_ = units;
	}

	void apply() override;

private:
	PathsDataModel &model_;
	QString units_;
};

