#pragma once

#include "SplinePair.h"
#include "Pose2dWithRotation.h"
#include <QtCore/QVector>

namespace xero
{
	namespace paths
	{
		class TrajectoryUtils
		{
		public:
			TrajectoryUtils() = delete;
			~TrajectoryUtils() = delete;

			static QVector<Pose2dWithRotation> parameterize(const QVector<std::shared_ptr<SplinePair>>& splines,
													double maxDx, double maxDy, double maxDTheta);

		private:
			static void getSegmentArc(std::shared_ptr<SplinePair> pair, QVector<Pose2dWithRotation>& results,
									  double t0, double t1, double maxDx, double maxDy, double maxDTheta);
		};
	}
}

