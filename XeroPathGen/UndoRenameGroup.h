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

	const QString& newName() const {
		return newname_;
	}

	const QString& oldName() const {
		return oldname_;
	}

private:
	QString newname_;
	QString oldname_;
	PathsDataModel& model_;
};

