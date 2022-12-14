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

#include "QuinticHermiteSpline.h"
#include "Translation2d.h"
#include "Pose2d.h"
#include <memory>
#include <vector>

class SplinePair
{
public:
	SplinePair(const Pose2d &p0, const Pose2d &p1);
	SplinePair(const QuinticHermiteSpline& x, const QuinticHermiteSpline& y);
	virtual ~SplinePair();

	QuinticHermiteSpline& getX() {
		return *x_;
	}

	QuinticHermiteSpline& getY() {
		return *y_;
	}

	double x0() { return x_->v0(); }
	double x1() { return x_->v0(); }
	double dx0() { return x_->dv0(); }
	double dx1() { return x_->dv1(); }
	double ddx0() { return x_->ddv0(); }
	double ddx1() { return x_->ddv1(); }

	double y0() { return y_->v0(); }
	double y1() { return y_->v0(); }
	double dy0() { return y_->dv0(); }
	double dy1() { return y_->dv1(); }
	double ddy0() { return y_->ddv0(); }
	double ddy1() { return y_->ddv1(); }

	void ddxy0(double x, double y) {
		x_->ddv0(x);
		y_->ddv0(y);
	}

	void ddxy1(double x, double y) {
		x_->ddv1(x);
		y_->ddv1(y);
	}

	Translation2d evalPosition(double t);
	Rotation2d evalHeading(double t);
	Pose2d evalPose(double t) { return Pose2d(evalPosition(t), evalHeading(t)); }

	double getCurvature(double t);
	double getDCurvature(double t);
	double getDCurvature2(double t);

	Pose2d getStartPose();
	Pose2d getEndPose();

	double sumDCurvature2() {
		double dt = 1.0 / kSamples;
		double sum = 0;
		for (double t = 0; t < 1.0; t += dt) {
			sum += (dt * getDCurvature2(t));
		}
		return sum;
	}

	bool hasStep() const {
		return has_step_;
	}

	double step() const {
		return step_;
	}

	void setStep(double v) {
		step_ = v;
		has_step_ = true;
	}

	void clearStep() {
		has_step_ = false;
		step_ = 0.1;
	}

private:
	double dx(double t) {
		return x_->derivative(t);
	}

	double ddx(double t) {
		return x_->derivative2(t);
	}

	double dddx(double t) {
		return x_->derivative3(t);
	}

	double dy(double t) {
		return y_->derivative(t);
	}

	double ddy(double t) {
		return y_->derivative2(t);
	}

	double dddy(double t) {
		return y_->derivative3(t);
	}

private:
	static constexpr int kSamples = 100;

private:
	QuinticHermiteSpline* x_;
	QuinticHermiteSpline* y_;
	bool has_step_;
	double step_;
};

