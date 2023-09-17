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
#include "CentripetalConstraint.h"
#include "UndoChangeCentripetalForceConstraint.h"

std::shared_ptr<PathConstraint> CentripetalConstraint::clone(std::shared_ptr<RobotPath> tpath)
{
	if (tpath == nullptr) {
		tpath = path();

	}
	auto ret = std::make_shared<CentripetalConstraint>(tpath, maxcen_);
	return ret;
}

void CentripetalConstraint::setMaxCenForce(double c, bool undoentry) {
	if (undoentry) {
		path()->beforeConstraintChanged(std::make_shared<UndoChangeCentripetalForceConstraint>(maxcen_, shared_from_this()));
	}
	maxcen_ = c;
	path()->afterConstraintChanged();
}