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
#include "QuadraticSolver.h"
#include <cmath>

std::vector<double> QuadraticSolver::solve(double a, double b, double c) {
	std::vector<double> result;
	double tmp = b * b - 4 * a * c;

	if (tmp == 0.0) {
		result.push_back(-b / (2 * a));
	}
	else if (tmp > 0.0) {
		result.push_back((-b + std::sqrt(tmp)) / (2 * a));
		result.push_back((-b - std::sqrt(tmp)) / (2 * a));

		if (result[0] < result[1]) {
			//
			// Swap the result, the biggest should always be first
			//
			double tmp2 = result[0];
			result[0] = result[1];
			result[1] = tmp2;
		}
	}
	return result;
}
