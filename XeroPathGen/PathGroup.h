#pragma once

#include "RobotPath.h"
#include <QtCore/QString>
#include <QtCore/QList>
#include <memory>

class PathGroup
{
public:
	PathGroup(const QString& name) {
		name_ = name;
	}

	const QString& name() const {
		return name_;
	}

	void setName(const QString& name) {
		name_ = name;
	}

	QStringList pathNames() const {
		QStringList names;

		for (auto path : paths_) {
			names.push_back(path->name());
		}

		return names;
	}

	void addPath(std::shared_ptr<RobotPath> path) {
		paths_.push_back(path);
	}

	void deletePath(const QString& name) {
		auto it = std::find_if(paths_.begin(), paths_.end(), [&name](const std::shared_ptr<RobotPath> p) { return p->name() == name; });
		if (it != paths_.end()) {
			paths_.erase(it);
		}
	}

	std::shared_ptr<RobotPath> getPathByName(const QString& name) const {
		auto it = std::find_if(paths_.begin(), paths_.end(), [&name](const std::shared_ptr<RobotPath> p) { return p->name() == name; });
		return (it == paths_.end()) ? nullptr : *it;
	}

	QList<std::shared_ptr<RobotPath>>& paths() {
		return paths_;
	}

private:
	QString name_;
	QList<std::shared_ptr<RobotPath>> paths_;
};

