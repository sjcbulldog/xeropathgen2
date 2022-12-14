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
#include "TrapezoidalProfile.h"
#include "QuadraticSolver.h"
#include <QtCore/QString>
#include <cmath>
#include <iostream>
#include <cassert>

TrapezoidalProfile::TrapezoidalProfile(double maxaccel, double maxdecel, double maxvel) {
	max_accel_ = maxaccel;
	max_decel_ = maxdecel;
	max_velocity_ = maxvel;
	isneg_ = false;
}

TrapezoidalProfile::~TrapezoidalProfile() {
}

double TrapezoidalProfile::getAccel(double t) const {
	double ret = 0.0;

	if (t < 0.0)
		ret = max_accel_;
	else if (t < ta_)
		ret = max_accel_;
	else if (t < ta_ + tc_)
		ret = 0.0;
	else if (t < ta_ + tc_ + td_)
		ret = max_decel_;

	return isneg_ ? -ret : ret;
}

double TrapezoidalProfile::getVelocity(double t) const {
	double ret;
	if (t < 0.0) {
		ret = start_velocity_;
	}
	else if (t < ta_) {
		ret = start_velocity_ + t * max_accel_;
	}
	else if (t < ta_ + tc_) {
		ret = actual_max_velocity_;
	}
	else if (t < ta_ + tc_ + td_) {
		double dt = (t - ta_ - tc_);
		ret = actual_max_velocity_ + dt * max_decel_;
	}
	else {
		ret = end_velocity_;
	}

	return isneg_ ? -ret : ret;
}

double TrapezoidalProfile::getDistance(double t) const {
	double ret;

	if (t < 0.0) {
		ret = 0.0;
	}
	else if (t < ta_) {
		ret = start_velocity_ * t + 0.5 * t * t * max_accel_;
	}
	else if (t < ta_ + tc_) {
		ret = start_velocity_ * ta_ + 0.5 * ta_ * ta_ * max_accel_;
		ret += (t - ta_) * actual_max_velocity_;
	}
	else if (t < ta_ + tc_ + td_) {
		double dt = t - ta_ - tc_;
		ret = start_velocity_ * ta_ + 0.5 * ta_ * ta_ * max_accel_;
		ret += tc_ * actual_max_velocity_;
		ret += actual_max_velocity_ * dt + 0.5 * dt * dt * max_decel_;
	}
	else {
		ret = distance_;
	}

	return isneg_ ? -ret : ret;
}

double TrapezoidalProfile::pickRoot(const std::vector<double>& roots) const {
	//
	// We want the smallest root that is greater than or equal to zero
	//
	assert(roots.size() != 0);
	int i = roots.size() - 1;
	while (i != 0) {
		if (roots[i] >= 0.0)
			return roots[i];
		i--;
	}

	return roots[0];
}

double TrapezoidalProfile::getTimeForDistance(double dist) const {
	double ret;
	double sign = isneg_ ? -1.0 : 1.0;
	std::vector<double> roots;

	if (isneg_)
		dist = -dist;

	if (dist < sign * getDistance(ta_)) {
		roots = QuadraticSolver::solve(0.5 * max_accel_, start_velocity_, -dist);
		ret = pickRoot(roots);
	}
	else if (dist < sign * getDistance(ta_ + tc_)) {
		dist -= sign * getDistance(ta_);
		ret = ta_ + dist / actual_max_velocity_;
	}
	else if (dist < sign * getDistance(ta_ + tc_ + td_)) {
		dist -= sign * getDistance(ta_ + tc_);
		roots = QuadraticSolver::solve(0.5 * max_decel_, actual_max_velocity_, -dist);
		ret = pickRoot(roots) + ta_ + tc_;
	}
	else {
		ret = ta_ + tc_ + td_;
	}

	return ret;
}

bool TrapezoidalProfile::update(double dist, double start_velocity, double end_velocity) {
	if (std::abs(dist) > 0.0 && (max_velocity_ == 0.0 || max_accel_ == 0.0))
		return false;

	start_velocity_ = std::fabs(start_velocity);
	end_velocity_ = std::fabs(end_velocity);

	isneg_ = (dist < 0);
	distance_ = std::fabs(dist);
	ta_ = (max_velocity_ - start_velocity_) / max_accel_;
	td_ = (end_velocity_ - max_velocity_) / max_decel_;
	double da = start_velocity * ta_ + 0.5 * max_accel_ * ta_ * ta_;
	double dd = max_velocity_ * td_ + 0.5 * max_decel_ * td_ * td_;
	tc_ = (distance_ - da - dd) / max_velocity_;
	type_ = "trapezoid";

	if (td_ < 0.0 || da + dd > distance_) {
		//
		// We don't have time to get to the cruising velocity
		//
		double num = (2.0 * distance_ * max_accel_ * max_decel_ + max_decel_ * start_velocity_ * start_velocity_ - max_accel_ * end_velocity_ * end_velocity_) / (max_decel_ - max_accel_);
		bool decel_only = false;
		if (num < 0)
			decel_only = true;
		else
			actual_max_velocity_ = std::sqrt(num);


		if (decel_only || actual_max_velocity_ < start_velocity_) {
			double realdist;

			// 
			// Just decelerate down to the end
			//
			ta_ = 0;
			tc_ = 0;
			td_ = (end_velocity - start_velocity_) / max_decel_;
			actual_max_velocity_ = start_velocity_;
			realdist = 0.5 * max_decel_ * td_ * td_ + start_velocity * td_;
			if (realdist > dist)
				throw std::runtime_error("there is no solution to the trapezoidal profile parameters given");
			type_ = "line";
		}
		else {
			//
			// Can't get to max velocity but can accelerate some
			// before decelerating
			//
			actual_max_velocity_ = std::sqrt(num);
			ta_ = (actual_max_velocity_ - start_velocity_) / max_accel_;
			td_ = (end_velocity_ - actual_max_velocity_) / max_decel_;
			tc_ = 0;
			type_ = "pyramid";
		}
	}
	else {
		//
		// Ok, now figure out the crusing time
		//
		actual_max_velocity_ = max_velocity_;
		tc_ = (distance_ - da - dd) / max_velocity_;
	}

	return true;
}

QString TrapezoidalProfile::toString() {
	QString ret = "[" + type_;
	ret += ", sv " + QString::number(start_velocity_, 'f', 2);
	ret += ", mv " + QString::number(actual_max_velocity_, 'f', 2);
	ret += ", ev " + QString::number(end_velocity_, 'f', 2);
	ret += ", ta " + QString::number(ta_, 'f', 2);
	ret += ", tc " + QString::number(tc_, 'f', 2);
	ret += ", td " + QString::number(td_, 'f', 2);
	ret += "]";
	return ret;
}
