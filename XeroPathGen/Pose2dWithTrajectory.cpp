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
#include "Pose2dWithTrajectory.h"
#include <cmath>

Pose2dWithTrajectory Pose2dWithTrajectory::interpolate(const Pose2dWithTrajectory& other, double percent) const
{
	Pose2dWithRotation npose = pose_.interpolate(other.pose(), percent);
	double ntime = (other.time() - time()) * percent + time();
	double npos = (other.position() - position()) * percent + position();
	double nvel = (other.velocity() - velocity()) * percent + velocity();
	double nacc = (other.acceleration() - acceleration()) * percent + acceleration();
	return Pose2dWithTrajectory(npose, ntime, npos, nvel, nacc);
}

double Pose2dWithTrajectory::getField(const QString& field) const
{
	double v = std::nan("");

	if (field == "x")
	{
		v = x();
	}
	else if (field == "y")
	{
		v = y();
	}
	else if (field == "heading")
	{
		v = rotation().toDegrees();
	}
	else if (field == "time")
	{
		v = time();
	}
	else if (field == "position")
	{
		v = position();
	}
	else if (field == "velocity")
	{
		v = velocity();
	}
	else if (field == "acceleration")
	{
		v = acceleration();
	}
	else if (field == "curvature")
	{
		v = curvature();
	}
	else if (field == "rotation")
	{
		v = swrot().toDegrees();
	}
	else if (field == "swrotvel")
	{
		v = rotVel();
	}

	return v;
}
