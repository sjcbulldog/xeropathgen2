#pragma once

#include "UnitConverter.h"
#include <limits>

class PathParameters
{
public:
	PathParameters() {
		start_velocity_ = 0.0;
		end_velocity_ = 0.0;
		max_velocity_ = 1e5;
		max_accel_ = 1e5;
	}

	PathParameters(double vel) {
		start_velocity_ = 0.0;
		end_velocity_ = 0.0;
		max_velocity_ = vel;
		max_accel_ = 1e5;
	}

	PathParameters(double vel, double accel) {
		start_velocity_ = 0.0;
		end_velocity_ = 0.0;
		max_velocity_ = vel;
		max_accel_ = accel;
	}

	PathParameters(double start, double end, double vel, double accel) {
		start_velocity_ = start;
		end_velocity_ = end;
		max_velocity_ = vel;
		max_accel_ = accel;
	}

	double startVelocity() const {
		return start_velocity_;
	}

	double endVelocity() const {
		return end_velocity_;
	}

	double maxVelocity() const {
		return max_velocity_;
	}

	double maxAccel() const {
		return max_accel_;
	}

	void convert(const QString& from, const QString& to) {
		if (from != to) {
			start_velocity_ = UnitConverter::convert(start_velocity_, from, to);
			end_velocity_ = UnitConverter::convert(end_velocity_, from, to);
			max_velocity_ = UnitConverter::convert(max_velocity_, from, to);
			max_accel_ = UnitConverter::convert(max_accel_, from, to);
		}
	}

private:
	double start_velocity_;
	double end_velocity_;
	double max_velocity_;
	double max_accel_;
};

