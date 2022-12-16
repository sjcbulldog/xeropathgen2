#pragma once

#include "Pose2dWithTrajectory.h"
#include <QtCore/QVector>
#include <QtCore/QString>

class PathTrajectory
{
public:
	PathTrajectory(const QString &name, const QVector<Pose2dWithTrajectory>& pts) {
		name_ = name;
		points_ = pts;
	}
	typedef QVector<Pose2dWithTrajectory>::iterator iterator;
	typedef QVector<Pose2dWithTrajectory>::const_iterator const_iterator;

	iterator begin() {
		return points_.begin();
	}

	iterator end() {
		return points_.end();
	}

	const_iterator begin() const {
		return points_.begin();
	}

	const_iterator end() const {
		return points_.end();
	}

	size_t size() const {
		return points_.size();
	}

	const Pose2dWithTrajectory& operator[](size_t index) const {
		return points_[index];
	}

	Pose2dWithTrajectory& operator[](size_t index) {
		return points_[index];
	}

	const QString& name() const {
		return name_;
	}

	size_t getIndex(double time);

	bool getTimeForDistance(double dist, double& time);

	double getEndTime() const {
		if (points_.size() == 0)
			return 0.0;

		return points_[points_.size() - 1].time();
	}

	double getDistance() const {
		if (points_.size() == 0)
			return 0.0;

		return points_[points_.size() - 1].position();
	}

private:
	QString name_;
	QVector<Pose2dWithTrajectory> points_;
};