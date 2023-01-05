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

#include <vector>

/// \file

// \brief This class is not instantiated, but rather contains static methods to solve quadratics
class QuadraticSolver {
public:
	QuadraticSolver() = delete;
	~QuadraticSolver() = delete;

	/// \brief Solve the quadratic equation given
	/// The parameters of the quadratic are given by a, b, and c.  All real
	/// roots of the quadratic are returns.  Imaginary roots are not.  The
	/// quadratic is represented in the form:<br> 
	// f(x) = a * x * x + b * x + c
	/// \param a parameter of the quadratic
	/// \param b parameter of the quadratic
	/// \param c parameter of the quadratic
	/// \returns the roots of the quadratic
	static std::vector<double> solve(double a, double b, double c);
};
