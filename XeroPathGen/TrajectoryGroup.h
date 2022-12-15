#pragma once

#include "GeneratorType.h"
#include "RobotPath.h"
#include "PathTrajectory.h"
#include <QtCore/QMap>
#include <memory>

class TrajectoryGroup 
{
public:
	TrajectoryGroup(GeneratorType type, std::shared_ptr<RobotPath> path);

	std::shared_ptr<RobotPath> path() {
		return path_;
	}

	GeneratorType type() const {
		return type_;
	}

	void addTrajectory(std::shared_ptr<PathTrajectory> traj) {
		trajectories_.insert(traj->name(), traj);
	}

	std::shared_ptr<PathTrajectory> getTrajectory(const QString& name) {
		std::shared_ptr<PathTrajectory> ret;

		if (trajectories_.contains(name)) {
			ret = trajectories_.value(name);
		}
		
		return ret;
	}

	void setErrorMessage(const QString& msg) {
		err_msg_ = msg;
	}

	bool hasError() const {
		return err_msg_.length() > 0;
	}

private:
	GeneratorType type_;
	std::shared_ptr<RobotPath> path_;
	QMap<QString, std::shared_ptr<PathTrajectory>> trajectories_;
	QString err_msg_;
};

