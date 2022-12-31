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
#pragma once

#include "RobotParams.h"
#include <string>
#include <vector>

class DriveBaseData
{
public:
	DriveBaseData() = delete;
	~DriveBaseData() = delete;

	static constexpr const char* TankDriveName = "Tank Drive";
	static constexpr const char* SwerveDriveName = "Swerve Drive";
	static constexpr const char* InvalidDriveName = "Invalid Drive";

	static const QVector<QString> getTrajectories(RobotParams::DriveType type);


	static RobotParams::DriveType nameToType(const QString& name)
	{
		RobotParams::DriveType ret = RobotParams::DriveType::Invalid;

		if (name == TankDriveName)
			ret = RobotParams::DriveType::TankDrive;
		else if (name == SwerveDriveName)
			ret = RobotParams::DriveType::SwerveDrive;
		else
			assert(false);

		return ret;
	}


	static QString typeToName(RobotParams::DriveType t)
	{
		QString ret = "Error";

		switch (t)
		{
		case RobotParams::DriveType::TankDrive:
			ret = TankDriveName;
			break;

		case RobotParams::DriveType::SwerveDrive:
			ret = SwerveDriveName;
			break;

		case RobotParams::DriveType::Invalid:
			ret = InvalidDriveName;
			break;

		default:
			assert(false);
			break;
		}

		return ret;
	}

private:
	static QVector<QString> tank_drive_trajectories_;
	static QVector<QString> swerve_drive_trajectories_;
	static QVector<QString> invalid_drive_trajectories_;
};

