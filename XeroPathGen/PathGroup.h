//
// Copyright 2022 Jack W. Griffin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.
//
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

	void insertPath(std::shared_ptr<RobotPath> path, int index) {
		paths_.insert(index, path);
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

	int getPathIndexByName(const QString& name) const {
		auto it = std::find_if(paths_.begin(), paths_.end(), [&name](const std::shared_ptr<RobotPath> p) { return p->name() == name; });
		return std::distance(paths_.begin(), it);
	}

	const QList<std::shared_ptr<RobotPath>>& paths() const {
		return paths_;
	}

private:
	QString name_;
	QList<std::shared_ptr<RobotPath>> paths_;
};

