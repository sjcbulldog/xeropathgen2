#include "Generator.h"
#include "RobotPath.h"
#include "PathGroup.h"
#include "CheesyGenerator.h"
#include "TrajectoryNames.h"
#include "TrajectoryUtils.h"
#include <QtCore/QThread>

int Generator::global_which_ = 1;

Generator::Generator(const QString& logfile, QMutex& mutex, double timestep, std::shared_ptr<RobotParams> robot, std::shared_ptr<TrajectoryGroup> group)
	: logfile_(logfile), loglock_(mutex)
{
	timestep_ = timestep;
	group_ = group;
	robot_ = robot ;

	which_ = global_which_++;
}

void Generator::generateTrajectory()
{
	auto path = group_->path();
	std::shared_ptr<PathTrajectory> traj;

	double diststep = UnitConverter::convert(1.0, "in", path->units());			// 1 inch works well, convert to units being used
	double maxdx = UnitConverter::convert(2.0, "in", path->units());			// 2 inches works well, convert to units being used
	double maxdy = UnitConverter::convert(0.5, "in", path->units());			// 0.5 inches works well, convert to units being used
	double maxtheta = 0.1;

	if (group_->type() == GeneratorType::CheesyPoofs) {
		CheesyGenerator gen(logfile_, loglock_, which_, diststep, timestep_, maxdx, maxdy, maxtheta, robot_, false);
		auto traj = gen.generate(path);

		if (traj != nullptr) {
			group_->addTrajectory(traj);
		}
	}
	else if (group_->type() == GeneratorType::ErrorCodeXeroSwerve) {
		CheesyGenerator gen(logfile_, loglock_, which_, diststep, timestep_, maxdx, maxdy, maxtheta, robot_, true);
		auto traj = gen.generate(path);

		if (traj != nullptr) {
			group_->addTrajectory(traj);
		}
	}

	if (!group_->hasError()) {
		if (robot_->getDriveType() == RobotParams::DriveType::TankDrive) {
			//
			// Add in trajectories for the left and right wheels.  These are here
			// as they will always be independent of how the main trajectory is generated
			//
			addTankDriveTrajectories();
		}
	}

	emit trajectoryComplete(group_);
}

void Generator::addTankDriveTrajectories()
{
	//
	// Get the width of the robot in the same units used by the paths
	//
	double width = UnitConverter::convert(robot_->getWheelBaseWidth(), robot_->getLengthUnits(), group_->path()->units());

	auto traj = group_->getTrajectory(TrajectoryName::Main);
	if (traj == nullptr) {
		return;
	}

	QVector<Pose2dWithTrajectory> leftpts;
	QVector<Pose2dWithTrajectory> rightpts;

	double lcurv = 0, rcurv = 0;
	double lvel = 0, lacc = 0, lpos = 0;
	double rvel = 0, racc = 0, rpos = 0;
	double plx = 0, ply = 0, prx = 0, pry = 0;
	double plvel = 0, prvel = 0;
	double placc = 0, pracc = 0;

	for (size_t i = 0; i < traj->size(); i++)
	{
		const Pose2dWithTrajectory& pt = (*traj)[i];
		double time = pt.time();
		double px = pt.x();
		double py = pt.y();

		double lx = px - width * pt.rotation().getSin() / 2.0;
		double ly = py + width * pt.rotation().getCos() / 2.0;
		double rx = px + width * pt.rotation().getSin() / 2.0;
		double ry = py - width * pt.rotation().getCos() / 2.0;

		if (i == 0)
		{
			lvel = 0.0;
			lacc = 0.0;
			lpos = 0.0;

			rvel = 0.0;
			racc = 0.0;
			rpos = 0.0;
		}
		else
		{
			double dt = time - (*traj)[i - 1].time();
			double ldist = std::sqrt((lx - plx) * (lx - plx) + (ly - ply) * (ly - ply));
			double rdist = std::sqrt((rx - prx) * (rx - prx) + (ry - pry) * (ry - pry));

			lvel = ldist / dt;
			rvel = rdist / dt;

			lacc = (lvel - plvel) / dt;
			racc = (rvel - prvel) / dt;

			lpos += ldist;
			rpos += rdist;
		}

		Translation2d lpt(lx, ly);
		Pose2d l2d(lpt, pt.rotation());
		Pose2dWithTrajectory ltraj(l2d, time, lpos, lvel, lacc);
		leftpts.push_back(ltraj);

		Translation2d rpt(rx, ry);
		Pose2d r2d(rpt, pt.rotation());
		Pose2dWithTrajectory rtraj(r2d, time, rpos, rvel, racc);
		rightpts.push_back(rtraj);

		plx = lx;
		ply = ly;
		prx = rx;
		pry = ry;
		plvel = lvel;
		prvel = rvel;
		placc = lacc;
		pracc = racc;
	}

	assert(leftpts.size() == rightpts.size());

	std::shared_ptr<PathTrajectory> left = std::make_shared<PathTrajectory>(TrajectoryName::Left, leftpts);
	TrajectoryUtils::computeCurvature(left);

	std::shared_ptr<PathTrajectory> right = std::make_shared<PathTrajectory>(TrajectoryName::Right, rightpts);
	TrajectoryUtils::computeCurvature(right);


	group_->addTrajectory(left);
	group_->addTrajectory(right);
}