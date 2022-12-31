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

#include "PathConstraint.h"
#include "UnitConverter.h"
#include "RobotPath.h"
#include "Pose2dWithTrajectory.h"
#include "UndoDistanceVelocityConstraintChange.h"
#include <memory>

class DistanceVelocityConstraint : public PathConstraint, public std::enable_shared_from_this<DistanceVelocityConstraint>
{
public:
	DistanceVelocityConstraint(std::shared_ptr<RobotPath> path, double after, double before, double velocity);
	virtual ~DistanceVelocityConstraint();

	void update(double after, double before, double velocity) {
		path()->beforeConstraintChanged(std::make_shared<UndoDistanceVelocityConstraintChange>(velocity_, after_distance_, before_distance_, shared_from_this()));
		after_distance_ = after;
		before_distance_ = before;
		velocity_ = velocity;
		path()->afterConstraintChanged();
	}

	double getMaxVelocity(const Pose2dWithTrajectory& state, std::shared_ptr<RobotParams> robot) override ;
	MinMaxAcceleration getMinMaxAccel(const Pose2dWithTrajectory& state, double velocity, std::shared_ptr<RobotParams> robot) override ;

	void convert(const QString& from, const QString& to) override {
		after_distance_ = UnitConverter::convert(after_distance_, from, to);
		before_distance_ = UnitConverter::convert(before_distance_, from, to);
		velocity_ = UnitConverter::convert(velocity_, from, to);
	}

	double getBefore() const {
		return before_distance_;
	}

	void setBefore(double d, bool undoentry) {
		if (undoentry) {
			path()->beforePathChanged(std::make_shared< UndoDistanceVelocityConstraintChange>(velocity_, after_distance_, before_distance_, shared_from_this()));
		}
		before_distance_ = d;
		path()->afterConstraintChanged();
	}

	double getAfter() const {
		return after_distance_;
	}

	void setAfter(double d, bool undoentry) {
		if (undoentry) {
			path()->beforePathChanged(std::make_shared< UndoDistanceVelocityConstraintChange>(velocity_, after_distance_, before_distance_, shared_from_this()));
		}
		after_distance_ = d;
		path()->afterConstraintChanged();
	}

	double getVelocity() const {
		return velocity_;
	}

	void setVelocity(double d, bool undoentry) {
		if (undoentry) {
			path()->beforePathChanged(std::make_shared< UndoDistanceVelocityConstraintChange>(velocity_, after_distance_, before_distance_, shared_from_this()));
		}
		velocity_ = d;
		path()->afterConstraintChanged();
	}

	QString toString() const override {
		QString ret;

		ret = "Limit velocity to " + QString::number(velocity_) + " " + path()->units() + "/s";
		ret += " between " + QString::number(after_distance_) + " " + path()->units();
		ret += " and " + QString::number(before_distance_) + " " + path()->units();

		return ret;
	}

	QJsonObject toJSON() const override {
		QJsonObject obj;

		obj.insert("type", "distancevelocity");
		obj.insert("before", before_distance_);
		obj.insert("after", after_distance_);
		obj.insert("velocity", velocity_);

		return obj;
	}

	static std::shared_ptr<PathConstraint> fromJSON(std::shared_ptr<RobotPath> path, const QJsonObject& obj, QString& msg) {

		QString type = RobotPath::getStringParam(obj, "type", msg);
		if (msg.length() > 0 || type != "distancevelocity") {
			return nullptr;
		}

		double after = RobotPath::getDoubleParam(obj, "after", msg);
		if (msg.length() > 0) {
			return nullptr;
		}

		double before = RobotPath::getDoubleParam(obj, "before", msg);
		if (msg.length() > 0) {
			return nullptr;
		}
		double velocity = RobotPath::getDoubleParam(obj, "velocity", msg);
		if (msg.length() > 0) {
			return nullptr;
		}


		return std::make_shared<DistanceVelocityConstraint>(path, after, before, velocity);
	}

private:
	double after_distance_;
	double before_distance_;
	double velocity_;
};

