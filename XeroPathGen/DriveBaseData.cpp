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
