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

	const QString& groupName() const {
		return grname_;
	}

	const QString& newName() const {
		return newname_;
	}

	const QString& oldName() const {
		return oldname_;
	}

private:
	QString grname_;
	QString newname_;
	QString oldname_;
	PathsDataModel& model_;
};

