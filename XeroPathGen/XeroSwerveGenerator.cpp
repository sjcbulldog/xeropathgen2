#include "XeroSwerveGenerator.h"

XeroSwerveGenerator::XeroSwerveGenerator(double diststep, double timestep, double maxdx, double maxdy, double maxtheta, std::shared_ptr<RobotParams> robot)
{

}

XeroSwerveGenerator::~XeroSwerveGenerator()
{

}

std::shared_ptr<PathTrajectory> 
XeroSwerveGenerator::generate(const QVector<Pose2dWithRotation>& waypoints, const QVector<std::shared_ptr<PathConstraint>>& constraints,
	double startvel, double endvel, double maxvel, double maxaccel, double maxjerk);