#include "GenerationMgr.h"
#include "Generator.h"
#include "RobotPath.h"
#include "PathGroup.h"
#include <QtCore/QStandardPaths>
#include <QtCore/QFile>

GenerationMgr::GenerationMgr()
{
	worker_ = nullptr;
	thread_ = nullptr;
	active_type_ = GeneratorType::CheesyPoofs;
	timestep_ = 0.02;

	QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
	logfile_ = dirs.front() + "/generators_log.txt";

	QFile file(logfile_);
	if (file.exists()) {
		file.remove();
	}
}

void GenerationMgr::clear()
{
	pending_queue_mutex_.lock();
	pending_queue_.clear();
	pending_queue_mutex_.unlock();

	if (thread_ != nullptr) {
		thread_->terminate();
		thread_ = nullptr;
		worker_ = nullptr;
	}
}

std::shared_ptr<TrajectoryGroup> GenerationMgr::getTrajectoryGroup(std::shared_ptr<RobotPath> path)
{
	std::shared_ptr<TrajectoryGroup> ret;

	trajectory_group_mutex_.lock();
	if (trajectories_.contains(path)) {
		ret = trajectories_.value(path);
	}
	trajectory_group_mutex_.unlock();

	return ret;
}

void GenerationMgr::addPath(GeneratorType type, std::shared_ptr<RobotPath> path)
{
	if (robot_ != nullptr) {
		removePath(path);
		pending_queue_mutex_.lock();
		pending_queue_.push_back(QPair<GeneratorType, std::shared_ptr<RobotPath>>(type, path));
		pending_queue_mutex_.unlock();
		schedulePath();
	}
}

void GenerationMgr::removePath(std::shared_ptr<RobotPath> path)
{
	pending_queue_mutex_.lock();

	auto it = std::find_if(pending_queue_.begin(), pending_queue_.end(), 
		[&path](QPair<GeneratorType, std::shared_ptr<RobotPath>> pair) { return pair.second == path; });

	if (it != pending_queue_.end()) {
		pending_queue_.erase(it);
	}

	pending_queue_mutex_.unlock();
}

void GenerationMgr::schedulePath()
{
	pending_queue_mutex_.lock();
	active_queue_mutex_.lock();

	if (pending_queue_.size() > 0 && worker_ == nullptr && thread_ == nullptr) {
		GeneratorType type = pending_queue_.front().first;
		std::shared_ptr<RobotPath> path = pending_queue_.front().second;

		pending_queue_.pop_front();
		pending_queue_mutex_.unlock();

		auto trajgrp = std::make_shared<TrajectoryGroup>(type, path);

		thread_ = new QThread();
		worker_ = new Generator(logfile_, loglock_, timestep_, robot_, trajgrp);
		worker_->moveToThread(thread_);

		connect(thread_, &QThread::started, worker_, &Generator::generateTrajectory);
		connect(thread_, &QThread::finished, worker_, &QObject::deleteLater);

		connect(worker_, &Generator::trajectoryComplete, this, &GenerationMgr::pathFinished);
		connect(worker_, &Generator::trajectoryComplete, thread_, &QThread::quit);
		connect(worker_, &Generator::trajectoryComplete, worker_, &QObject::deleteLater);

		thread_->start();
	}
	else {
		pending_queue_mutex_.unlock();
	}
	active_queue_mutex_.unlock();
}

void GenerationMgr::pathFinished(std::shared_ptr<TrajectoryGroup> group)
{
	active_queue_mutex_.lock();
	thread_ = nullptr;
	worker_ = nullptr;
	trajectories_.insert(group->path(), group);
	active_queue_mutex_.unlock();
	schedulePath();

	emit generationComplete(group->path());
}