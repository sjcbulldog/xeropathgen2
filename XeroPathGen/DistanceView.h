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

	double getPosition(int index) const {
		return distances_[index];
	}

	Pose2dWithRotation operator[](double dist) const;
	Pose2dWithRotation operator[](int index) const;
	int size() const {
		return points_.size();
	}

private:
	QVector<double> distances_;
	QVector<Pose2dWithRotation> points_;
};

