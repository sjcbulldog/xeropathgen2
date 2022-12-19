#pragma once

#include "UnitConverter.h"
#include <QtCore/QString>
#include <cassert>

class RobotParams
{
public:
	enum class DriveType : int
	{
		TankDrive,
		SwerveDrive,
		Invalid
	};

	static constexpr const char* VersionTag = "_version";
	static constexpr const char* NameTag = "name";
	static constexpr const char* RobotLengthTag = "robotlength";
	static constexpr const char* RobotWidthTag = "robotwidth";
	static constexpr const char* RobotWeightTag = "weight";
	static constexpr const char* EffectiveLengthTag = "efflength";
	static constexpr const char* EffectiveWidthTag = "effwidth";
	static constexpr const char* MaxVelocityTag = "maxvelocity";
	static constexpr const char* MaxAccelerationTag = "maxacceleration";
	static constexpr const char* MaxJerkTag = "maxjerk";
	static constexpr const char* MaxCentripetalTag = "maxcentripetal";
	static constexpr const char* TimeStepTag = "timestep";
	static constexpr const char* DriveTypeTag = "drivetype";
	static constexpr const char* LengthUnitsTag = "lengthunits";
	static constexpr const char* WeightUnitsTag = "weightunits";

public:
	static constexpr double DefaultLength = 30.0;
	static constexpr double DefaultWidth = 24.0;
	static constexpr double DefaultWeight = 180.0;
	static constexpr double DefaultMaxVelocity = 180.0;
	static constexpr double DefaultMaxAcceleration = 180.0;
	static constexpr double DefaultMaxJerk = 1800.0;
	static constexpr double DefaultCentripetal = 100000.0;
	static constexpr double DefaultTimestep = 0.02;
	static constexpr DriveType DefaultDriveType = DriveType::TankDrive;
	static constexpr const char* DefaultLengthUnits = "m";
	static constexpr const char* DefaultWeightUnits = "lbs";

public:
	RobotParams(const QString& name) {
		name_ = name;
		drivetype_ = DefaultDriveType;
		rlength_ = DefaultLength;
		rwidth_ = DefaultWidth;
		elength_ = DefaultLength;
		ewidth_ = DefaultWidth;
		max_velocity_ = DefaultMaxVelocity;
		max_acceleration_ = DefaultMaxAcceleration;
		max_jerk_ = DefaultMaxJerk;
		length_units_ = "in";
		weight_units_ = "lbs";
		timestep_ = 0.02;
		max_centripetal_force_ = 1100;
	}

	virtual ~RobotParams() {
	}

	static std::vector<DriveType> getDriveTypes() {
		return std::vector<DriveType> { DriveType::TankDrive, DriveType::SwerveDrive };
	}

	const QString& getName() const {
		return name_;
	}

	void setName(const QString& name) {
		name_ = name;
	}

	const QString& getFilename() const {
		return filename_;
	}

	void setFilename(const QString& file) {
		filename_ = file;
	}

	DriveType getDriveType() const {
		return drivetype_;
	}

	void setDriveType(DriveType t) {
		drivetype_ = t;
	}

	void setLengthUnits(const QString& units) {
		length_units_ = units;
	}

	void setWeightUnits(const QString& units) {
		weight_units_ = units;
	}

	const QString& getLengthUnits() const {
		return length_units_;
	}

	const QString& getWeightUnits() const {
		return weight_units_;
	}

	void convert(const QString& units) {
		elength_ = UnitConverter::convert(elength_, length_units_, units);
		ewidth_ = UnitConverter::convert(ewidth_, length_units_, units);
		rlength_ = UnitConverter::convert(rlength_, length_units_, units);
		rwidth_ = UnitConverter::convert(rwidth_, length_units_, units);
		max_velocity_ = UnitConverter::convert(max_velocity_, length_units_, units);
		max_acceleration_ = UnitConverter::convert(max_acceleration_, length_units_, units);
		max_jerk_ = UnitConverter::convert(max_jerk_, length_units_, units);
		length_units_ = units;
	}

	double getWheelBaseWidth() const {
		return ewidth_;
	}

	void setWheelBaseWidth(double v) {
		ewidth_ = v;
	}

	double getWheelBaseLength() const {
		return elength_;
	}

	void setWheelBaseLength(double v) {
		elength_ = v;
	}

	double getBumberWidth() const {
		return rwidth_;
	}

	void setBumberWidth(double v) {
		rwidth_ = v;
	}

	double getRobotWeight() const {
		return weight_;
	}

	void setRobotWeight(double v) {
		weight_ = v;
	}

	double getBumberLength() const {
		return rlength_;
	}

	void setBumberLength(double v) {
		rlength_ = v;
	}

	double getMaxVelocity() const {
		return max_velocity_;
	}

	void setMaxVelocity(double v) {
		max_velocity_ = v;
	}

	double getMaxAccel() const {
		return max_acceleration_;
	}

	void setMaxAcceleration(double v) {
		max_acceleration_ = v;
	}

	double getMaxJerk() const {
		return max_jerk_;
	}

	void setMaxJerk(double v) {
		max_jerk_ = v;
	}

	double getTimestep() const {
		return timestep_;
	}

	void setTimestep(double v) {
		timestep_ = v;
	}

	double getMaxCentripetalForce() const {
		return max_centripetal_force_;
	}

	void setMaxCentripetalForce(double m) {
		max_centripetal_force_ = m;
	}

private:
	double rwidth_;
	double rlength_;
	double weight_;
	double ewidth_;
	double elength_;
	double max_velocity_;
	double max_acceleration_;
	double max_jerk_;
	double max_centripetal_force_;
	QString length_units_;
	QString weight_units_;
	QString name_;
	DriveType drivetype_;
	QString filename_;
	double timestep_;
};

