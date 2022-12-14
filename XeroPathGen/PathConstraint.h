#pragma once

#include "MinMaxAcceleration.h"
#include "Pose2dWithTrajectory.h"
#include <QtCore/QJsonObject>

class PathConstraint
{
public:
	PathConstraint() {
	}

	virtual ~PathConstraint() {
	}

	virtual double getMaxVelocity(const Pose2dWithTrajectory& state) = 0;
	virtual MinMaxAcceleration getMinMacAccel(const Pose2dWithTrajectory& state, double velocity) = 0;
	virtual void convert(const QString& from, const QString &to) = 0;
	virtual QJsonObject toJSON() const = 0;
};

