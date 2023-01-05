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
#include "PathTrajectory.h"
#include <cmath>

int PathTrajectory::getIndex(double time)
{
	if (size() == 0)
		return std::numeric_limits<int>::max();

	if (time < points_.front().time())
		return std::numeric_limits<int>::max();

	if (time > points_.back().time())
		return std::numeric_limits<int>::max();

	double delta = std::numeric_limits<double>::max();
	int ret = 0;
	for (int i = 0; i < size(); i++)
	{
		double dt = std::fabs(points_[i].time() - time);
		if (dt < delta)
		{
			delta = dt;
			ret = i;
		}
	}

	return ret;
}

bool PathTrajectory::getTimeForDistance(double dist, double& time)
{
	if (size() == 0)
		return false;

	if (dist < 0.0)
		return false;

	if (dist > points_.back().position())
	{
		time = points_.back().time();
		return true;
	}

	//
	// Do a binary search to find the time for the distance given
	//
	int low = 0;
	int high = points_.size() - 1;

	while (high - low > 1)
	{
		int mid = (high + low) / 2;
		if (dist > points_[mid].position())
		{
			low = mid;
		}
		else
		{
			high = mid;
		}
	}

	double pcnt = (dist - points_[low].position()) / (points_[high].position() - points_[low].position());
	Pose2dWithTrajectory pt = points_[low].interpolate(points_[high], pcnt);

	time = pt.time();
	return true;
}
