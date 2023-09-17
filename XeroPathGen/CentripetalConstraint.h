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

#include "UnitConverter.h"
#include "PathConstraint.h"
#include "RobotPath.h"

#include <QtCore/QString>

class CentripetalConstraint : public PathConstraint, public std::enable_shared_from_this<CentripetalConstraint>
{
public:
	CentripetalConstraint(std::shared_ptr<RobotPath> path, double maxcen) : PathConstraint(path) {
		//
		// The units of force are newtons which are mass * length / time / time.  The value
		// calculations are done by converting everything to meters and kilograms, computing the constrainted
		// velocity with will be in meters / second, and then convering the velocity back to the units
		// of the path
		//
		maxcen_ = maxcen;
	}

	virtual std::shared_ptr<PathConstraint> clone(std::shared_ptr<RobotPath> path);

	double maxCenForce() const {
		return maxcen_;
	}

	void setMaxCenForce(double c, bool undoentry = true);

	double getMaxVelocity(const Pose2dWithTrajectory& state, std::shared_ptr<RobotParams> robot) override {
		//
		// The conastraint value is in Newtons.  To compute a velocity, we need to convert the units to the
		// length and mass units being used by the user.
		//

		if (std::abs(state.curvature()) < 0.0001) {
			//
			// If there is not curvature, there is not centripal force and therefore there is no
			// limit on the velocity 
			//
			return std::numeric_limits<double>::max();
		}

		//
		// Get the weight and convert to kgrams
		//
		double weight = UnitConverter::convert(robot->getRobotWeight(), robot->getWeightUnits(), "kg");

		//
		// Get the radius of the curvature in meters
		//
		double radius = UnitConverter::convert(1.0 / state.curvature(), path()->units(), "m");

		//
		// Compute the velocity in meters per second
		//
		double vel = std::sqrt(std::abs(maxcen_ * radius / weight));

		//
		// Convert the velocity back to the units of the path
		//
		vel = UnitConverter::convert(vel, "m", path()->units());

		return vel;
	}

	MinMaxAcceleration getMinMaxAccel(const Pose2dWithTrajectory& state, double velocity, std::shared_ptr<RobotParams> robot) override {
		(void)state;
		(void)velocity;

		MinMaxAcceleration acc;
		return acc;
	}

	void convert(const QString& from, const QString& to) override {
	}

	QString toString() const {
		QString ret;

		ret = "Centripetal force cannot exceed " + QString::number(maxcen_) + " Newtons";
		return ret;
	}

	QJsonObject toJSON() const override {
		QJsonObject obj;

		obj.insert("type", "centripetal");
		obj.insert("maxcen", maxcen_);

		return obj;
	}

	static std::shared_ptr<PathConstraint> fromJSON(std::shared_ptr<RobotPath> path, const QJsonObject& obj, QString &msg) {
		QString type = RobotPath::getStringParam(obj, "type", msg);
		if (msg.length() > 0 || type != "centripetal") {
			return nullptr;
		}

		double maxcen = RobotPath::getDoubleParam(obj, "maxcen", msg);
		if (msg.length() > 0) {
			return nullptr;
		}

		return std::make_shared<CentripetalConstraint>(path, maxcen);
	}

private:
	double maxcen_;
};

