#pragma once
#include "UndoAction.h"
#include <QtCore/QString>

class PathsDataModel;

class UndoRenamePath : public UndoAction
{
public:
	UndoRenamePath(const QString& grname, const QString& oldname, const QString& newname, PathsDataModel& model) : model_(model) {
		grname_ = grname;
		newname_ = newname;
		oldname_ = oldname;
	}

	void apply() override;

private:
	QString grname_;
	QString newname_;
	QString oldname_;
	PathsDataModel& model_;
};

