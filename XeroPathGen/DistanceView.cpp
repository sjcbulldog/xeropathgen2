#include "DistanceView.h"
#include "TrajectoryUtils.h"

DistanceView::DistanceView(const QVector<Pose2dWithRotation>& points, double step)
{
	static const double kEpsilon = 1e-6;
	double d;

	distances_.push_back(0.0);
	for (int i = 1; i < points.size(); i++)
		distances_.push_back(points[i].distance(points[i - 1]) + distances_[i - 1]);

	int index = 0;
	for (d = 0.0; d <= distances_.back(); d += step)
	{
		while (d > distances_[index + 1])
			index++;

		double percent = (d - distances_[index]) / (distances_[index + 1] - distances_[index]);
		Pose2dWithRotation newpt = points[index].interpolate(points[index + 1], percent);
		points_.push_back(newpt);
	}

	distances_.clear();
	distances_.push_back(0.0);
	for (int i = 1; i < points_.size(); i++)
		distances_.push_back(points_[i].distance(points_[i - 1]) + distances_[i - 1]);

	TrajectoryUtils::computeCurvature(points_);

}

Pose2dWithRotation DistanceView::operator[](double dist) const
{
	Pose2d result;
	int low = 0;
	int high = distances_.size();

	while (high - low > 1)
	{
		int mid = (high + low) / 2;
		if (dist > distances_[mid])
			low = mid;
		else
			high = mid;
	}

	if (high == distances_.size())
		result = points_[low];
	else
	{
		double percent = (dist - distances_[low]) / (distances_[high] - distances_[low]);
		result = points_[low].interpolate(points_[high], percent);
	}

	return result;
}

Pose2dWithRotation DistanceView::operator[](int index) const {
	return points_[index];
}
