#pragma once
#include "Pose2dWithRotation.h"
#include <QtCore/QString>

class Pose2dWithTrajectory : public ICsv
{
public:
	Pose2dWithTrajectory()
	{
		time_ = 0.0;
		position_ = 0.0;
		velocity_ = 0.0;
		acceleration_ = 0.0;
	}

	Pose2dWithTrajectory(const Pose2dWithRotation& pose, double time, double pos, double vel, double acc) {
		pose_ = pose;
		time_ = time;
		position_ = pos;
		velocity_ = vel;
		acceleration_ = acc;
	}

	virtual ~Pose2dWithTrajectory() {
	}

	const Pose2dWithRotation& pose() const {
		return pose_;
	}

	Pose2dWithRotation& pose() {
		return pose_;
	}

	void setPose(const Pose2dWithRotation& p) {
		pose_ = p;
	}

	const Translation2d& translation() const {
		return pose_.getTranslation();
	}

	const Rotation2d& rotation() const {
		return pose_.getRotation();
	}

	const Rotation2d& swrot() const {
		return pose_.swrot();
	}

	double x() const {
		return pose_.getTranslation().getX();
	}

	double y() const {
		return pose_.getTranslation().getY();
	}

	double time() const {
		return time_;
	}

	double position() const {
		return position_;
	}

	double curvature() const {
		return pose_.curvature();
	}

	void setSwRotation(Rotation2d d) {
		pose_.setSwrot(d);
	}

	const Rotation2d &swrotation() const {
		return pose_.swrot();
	}

	double velocity() const {
		return velocity_;
	}

	void setPosition(double p) {
		position_ = p;
	}

	double acceleration() const {
		return acceleration_;
	}

	void setVelocity(double d) {
		velocity_ = d;
	}

	void setAcceleration(double a) {
		acceleration_ = a;
	}

	double getField(const QString& field) const;

	Pose2dWithTrajectory interpolate(const Pose2dWithTrajectory& other, double percent) const;

private:
	Pose2dWithRotation pose_;
	double time_;
	double position_;
	double velocity_;
	double acceleration_;
};
