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

class QuinticHermiteSpline
{
public:
	QuinticHermiteSpline(double v0, double v1, double dv0, double dv1, double ddv0, double ddv1);

	double eval(double t);
	double derivative(double t);
	double derivative2(double t);
	double derivative3(double t);

	double v0() { return v0_; }
	double v1() { return v1_; }
	double dv0() { return dv0_; }
	double dv1() { return dv1_; }
	double ddv0() { return ddv0_; }
	double ddv1() { return ddv1_; }

	void ddv0(double v) { ddv0_ = v; compute(); }
	void ddv1(double v) { ddv1_ = v; compute(); }
	double a() { return a_; }
	double b() { return b_; }
	double c() { return c_; }
	double d() { return d_; }
	double e() { return e_; }
	double f() { return f_; }

private:
	void compute();

private:
	double a_, b_, c_, d_, e_, f_;
	double v0_, v1_, dv0_, dv1_, ddv0_, ddv1_;
};

