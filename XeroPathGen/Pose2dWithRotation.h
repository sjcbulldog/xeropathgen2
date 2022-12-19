#pragma once

#include "Pose2d.h"

class Pose2dWithRotation : public Pose2d
{
public:
	Pose2dWithRotation();
	Pose2dWithRotation(double x, double y);
	Pose2dWithRotation(double x, double y, const Rotation2d& rot);
	Pose2dWithRotation(const Translation2d& pos);
	Pose2dWithRotation(const Rotation2d& pos);
	Pose2dWithRotation(const Translation2d& pos, const Rotation2d& rot);
	Pose2dWithRotation(const Translation2d& pos, const Rotation2d& rot, const Rotation2d& swrot, double cur = 0.0);
	Pose2dWithRotation(const Pose2d& other);
	Pose2dWithRotation(const Pose2dWithRotation& other);
	virtual ~Pose2dWithRotation();

	Pose2dWithRotation interpolate(const Pose2dWithRotation& other, double percent) const;

	const Rotation2d& swrot() const {
		return swrot_;
	}

	void setSwrot(const Rotation2d& swrot) {
		swrot_ = swrot;
	}

	void setCurvature(double d) {
		curvature_ = d;
	}

	double curvature() const {
		return curvature_;
	}

	static double triangleArea(const Pose2dWithRotation& a, const Pose2dWithRotation& b, const Pose2dWithRotation& c)
	{
		return ((b.getTranslation().getX() - a.getTranslation().getX()) * (c.getTranslation().getY() - a.getTranslation().getY())) -
			((b.getTranslation().getY() - a.getTranslation().getY()) * (c.getTranslation().getX() - a.getTranslation().getX()));
	}

	static double curvature(const Pose2dWithRotation& a, const Pose2dWithRotation& b, const Pose2dWithRotation& c)
	{
		double area = triangleArea(a, b, c);

		double len1 = a.distance(b);
		double len2 = b.distance(c);
		double len3 = a.distance(c);

		return 4.0 * area / (len1 * len2 * len3);
	}

private:
	Rotation2d swrot_;
	double curvature_;
};

