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

class PathsDataModel;
class PathGroup;

class UndoDeleteGroup : public UndoAction
{
public:
	UndoDeleteGroup(PathGroup* gr, int index, PathsDataModel& model) : model_(model) {
		group_ = gr;
		index_ = index;
	}

	void apply();

	PathGroup* group() {
		return group_;
	}
	
	int index() {
		return index_;
	}

private:
	PathsDataModel& model_;
	PathGroup* group_;
	int index_;
};

