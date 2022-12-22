#pragma once

#include "UnitConverter.h"
#include "Translation2d.h"
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
	static constexpr const char* MaxCentripetalTag = "maxcentripetal";
	static constexpr const char* TimeStepTag = "timestep";
	static constexpr const char* DriveTypeTag = "drivetype";
	static constexpr const char* LengthUnitsTag = "lengthunits";
	static constexpr const char* WeightUnitsTag = "weightunits";

public:
	static constexpr double DefaultLength = 0.762;
	static constexpr double DefaultWidth = 0.6096;
	static constexpr double DefaultWeight = 180.0;
	static constexpr double DefaultMaxVelocity = 4.0;
	static constexpr double DefaultMaxAcceleration = 4.0;
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
		length_units_ = "in";
		weight_units_ = "lbs";
		timestep_ = 0.02;
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

	double getTimestep() const {
		return timestep_;
	}

	void setTimestep(double v) {
		timestep_ = v;
	}

	void getLocations(const QString &units, Translation2d& fl, Translation2d& fr, Translation2d& bl, Translation2d& br) {
		double width = UnitConverter::convert(rwidth_, length_units_, units);
		double length = UnitConverter::convert(rlength_, length_units_, units);

		fl = Translation2d(width / 2.0, length / 2.0);
		fr = Translation2d(width / 2.0, -length / 2.0);
		bl = Translation2d(-width / 2.0, length / 2.0);
		br = Translation2d(-width / 2.0, -length / 2.0);
	}

private:
	double rwidth_;
	double rlength_;
	double weight_;
	double ewidth_;
	double elength_;
	double max_velocity_;
	double max_acceleration_;
	QString length_units_;
	QString weight_units_;
	QString name_;
	DriveType drivetype_;
	QString filename_;
	double timestep_;
};

