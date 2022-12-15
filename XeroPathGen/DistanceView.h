#pragma once


#include "Pose2dWithRotation.h"
#include <QtCore/QVector>

class DistanceView
{
public:
	DistanceView(const QVector<Pose2dWithRotation>& points, double delta);
	double length() const {
		return distances_.back();
	}

	double getPosition(size_t index) const {
		return distances_[index];
	}

	Pose2dWithRotation operator[](double dist) const;
	Pose2dWithRotation operator[](size_t index) const;
	size_t size() const {
		return points_.size();
	}

private:
	std::vector<double> distances_;
	std::vector<Pose2dWithRotation> points_;
};

