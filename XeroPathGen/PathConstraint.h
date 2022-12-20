#pragma once

#include "MinMaxAcceleration.h"
#include "Pose2dWithTrajectory.h"
#include "RobotParams.h"
#include <QtCore/QJsonObject>
#include <memory>

class RobotPath;

class PathConstraint
{
public:
	PathConstraint(std::shared_ptr<RobotPath> path) {
		path_ = path;
	}

	virtual ~PathConstraint() {
	}

	virtual double getMaxVelocity(const Pose2dWithTrajectory& state, std::shared_ptr<RobotParams> robot) = 0;
	virtual MinMaxAcceleration getMinMaxAccel(const Pose2dWithTrajectory& state, double velocity, std::shared_ptr<RobotParams> robot) = 0;
	virtual void convert(const QString& from, const QString &to) = 0;
	virtual QJsonObject toJSON() const = 0;
	virtual QString toString() const = 0;

	std::shared_ptr<RobotPath> path() const {
		return path_;
	}

private:
	std::shared_ptr<RobotPath> path_;
};

