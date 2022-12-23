#pragma once

#include "UndoAction.h"
#include <QtCore/QString>
#include <memory>

class RobotPath;

class UndoChangePathName : public UndoAction
{
public:
	UndoChangePathName(const QString& name, std::shared_ptr<RobotPath> path) {
		path_ = path;
		name_ = name;
	}

	void apply() override;

private:
	std::shared_ptr<RobotPath> path_;
	QString name_;
};

