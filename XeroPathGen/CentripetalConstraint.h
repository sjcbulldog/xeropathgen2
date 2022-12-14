#pragma once

#include "UnitConverter.h"
#include "PathConstraint.h"
#include "RobotPath.h"
#include <QtCore/QString>

class CentripetalConstraint : public PathConstraint
{
public:
	CentripetalConstraint(double maxcen, double weight, const QString& lenunits, const QString& wtunits) {
		weight_ = weight;
		maxcen_ = maxcen;
		length_units_ = lenunits;
		weight_units_ = wtunits;
	}

	virtual double getMaxVelocity(const Pose2dWithTrajectory& state) {
		double radius = 1 / state.curvature();
		double radius_m = UnitConverter::convert(radius, length_units_, "m");
		double vel = std::sqrt(std::abs(maxcen_ * radius_m / weight_));
		double velunits = UnitConverter::convert(vel, "m", length_units_);

		return velunits;
	}

	virtual MinMaxAcceleration getMinMacAccel(const Pose2dWithTrajectory& state, double velocity) {
		(void)state;
		(void)velocity;

		MinMaxAcceleration acc;
		return acc;
	}

	virtual void convert(const std::string& from, const std::string& to) {
	}

	virtual QJsonObject toJSON() const {
		QJsonObject obj;

		obj.insert("type", "centripetal");
		obj.insert("maxcen", maxcen_);
		obj.insert("weight", weight_);
		obj.insert("length-units", length_units_);
		obj.insert("weight-units", weight_units_);

		return obj;
	}

	static std::shared_ptr<PathConstraint> fromJSON(const QJsonObject& obj, QString &msg) {
		QString msg;

		QString type = RobotPath::getStringParam(obj, "type", msg);
		if (msg.length() > 0 || type != "centripetal") {
			return nullptr;
		}

		double maxcen = RobotPath::getDoubleParam(obj, "maxcen", msg);
		if (msg.length() > 0) {
			return nullptr;
		}

		double weight = RobotPath::getDoubleParam(obj, "weight", msg);
		if (msg.length() > 0) {
			return nullptr;
		}


		QString lenunits = RobotPath::getStringParam(obj, "length-units", msg);
		if (msg.length() > 0) {
			return nullptr;
		}

		QString wtunits = RobotPath::getStringParam(obj, "weight-units", msg);
		if (msg.length() > 0) {
			return nullptr;
		}

		return std::make_shared<CentripetalConstraint>(maxcen, weight, lenunits, wtunits);
	}

private:
	double weight_;
	double maxcen_;
	QString length_units_;
	QString weight_units_;
};

