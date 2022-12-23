#pragma once
#include "UndoAction.h"
#include <QtCore/QString>

class PathGroup;

class UndoRenameGroup : public UndoAction
{
public:
	UndoRenameGroup(const QString &newname, const QString& oldname, PathsDataModel& model) : model_(model) {
		newname_ = newname;
		oldname_ = oldname;
	}

	void apply();

private:
	QString newname_;
	QString oldname_;
	PathsDataModel& model_;
};

