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
#include "DriveBaseData.h"
#include <cassert>

		QVector<QString> DriveBaseData::tank_drive_trajectories_ =
		{
			"main",
			"left",
			"right"
		};

		QVector<QString> DriveBaseData::swerve_drive_trajectories_ =
		{
			"main",
			"fl",
			"fr",
			"bl",
			"br"
		};

		QVector<QString> DriveBaseData::invalid_drive_trajectories_ =
		{
		};


		const QVector<QString> DriveBaseData::getTrajectories(RobotParams::DriveType type)
		{
			switch (type)
			{
			case RobotParams::DriveType::TankDrive:
				return tank_drive_trajectories_;

			case RobotParams::DriveType::SwerveDrive:
				return swerve_drive_trajectories_;

			case RobotParams::DriveType::Invalid:
			  	return invalid_drive_trajectories_ ;
			}

			assert(false);
			return QVector<QString>();
		}
