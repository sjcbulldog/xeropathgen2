#pragma once
#include "UndoAction.h"
#include <QtCore/QString>

class PathsDataModel;

class UndoAddGroup : public UndoAction
{
public:
	UndoAddGroup(const QString& grname, PathsDataModel& model) : model_(model) {
		grname_ = grname;
	}

	void apply() override;

private:
	PathsDataModel& model_;
	QString grname_;
};

