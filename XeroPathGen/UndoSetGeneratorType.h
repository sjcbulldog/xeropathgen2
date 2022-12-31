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
#include "GeneratorType.h"

class PathsDataModel;

class UndoSetGeneratorType : public UndoAction
{
public:
	UndoSetGeneratorType(GeneratorType gentype, PathsDataModel &model) : model_(model) {
		gentype_ = gentype;
	}

	void apply() override;

private:
	PathsDataModel& model_;
	GeneratorType gentype_;
};
