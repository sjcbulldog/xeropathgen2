#pragma once

#include "RobotPath.h"
#include "GeneratorType.h"
#include "Generator.h"
#include "TrajectoryGroup.h"
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QThread>

class GenerationMgr : public QObject
{
	Q_OBJECT

public:
	GenerationMgr();

	void addPath(GeneratorType type, std::shared_ptr<RobotPath> path);
	void removePath(std::shared_ptr<RobotPath> path);

	std::shared_ptr<TrajectoryGroup> getTrajectoryGroup(std::shared_ptr<RobotPath> path);

signals:
	void generationComplete(std::shared_ptr<RobotPath> path);

private:
	void schedulePath();
	void pathFinished(std::shared_ptr<TrajectoryGroup> path);

private:
	QMutex pending_queue_mutex_;
	QList<QPair<GeneratorType, std::shared_ptr<RobotPath>>> pending_queue_;

	QMutex trajectory_group_mutex_;
	QMap<std::shared_ptr<RobotPath>, std::shared_ptr<TrajectoryGroup>> trajectories_;

	QMutex active_queue_mutex_;
	GeneratorType active_type_;
	std::shared_ptr<RobotPath> active_path_;

	Generator* worker_;
	QThread *thread_;
};
