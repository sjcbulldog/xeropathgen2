#pragma once

#include "Pose2dWithRotation.h"
#include "PathConstraint.h"
#include "PathParameters.h"
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QVector>
#include <QtCore/QJsonObject>
#include <memory>

class PathGroup;

class RobotPath : public QObject
{
	friend class CentripetalConstraint;
	friend class PathsDataModel;

	Q_OBJECT

public:
	static constexpr const char* VersionTag = "_version";
	static constexpr const char* UnitsTag = "units";
	static constexpr const char* TimeTag = "time";
	static constexpr const char* OutputTag = "outdir";
	static constexpr const char* GroupsTag = "groups";
	static constexpr const char* GroupTag = "group";
	static constexpr const char* PositionTag = "position";
	static constexpr const char* CurvatureTag = "curvature";
	static constexpr const char* AccelerationTag = "acceleration";
	static constexpr const char* JerkTag = "jerk";
	static constexpr const char* NameTag = "name";
	static constexpr const char* PathsTag = "paths";
	static constexpr const char* StartVelocityTag = "startvelocity";
	static constexpr const char* EndVelocityTag = "endvelocity";
	static constexpr const char* StartAngleTag = "startangle";
	static constexpr const char* StartAngleDelayTag = "startangledelay";
	static constexpr const char* EndAngleTag = "endangle";
	static constexpr const char* EndAngleDelayTag = "endangledelay";
	static constexpr const char* MaxVelocityTag = "maxvelocity";
	static constexpr const char* MaxAccelerationTag = "maxacceleration";
	static constexpr const char* MaxJerkTag = "maxjerk";
	static constexpr const char* MaxCentripetalTag = "maxcentripetal";
	static constexpr const char* ConstraintsTag = "constraints";
	static constexpr const char* TypeTag = "type";
	static constexpr const char* DistanceVelocityTag = "distance_velocity";
	static constexpr const char* BeforeTag = "before";
	static constexpr const char* AfterTag = "after";
	static constexpr const char* VelocityTag = "velocity";
	static constexpr const char* PointsTag = "points";
	static constexpr const char* XTag = "x";
	static constexpr const char* YTag = "y";
	static constexpr const char* SwerveRotationTag = "swrot";
	static constexpr const char* HeadingTag = "heading";
	static constexpr const char* RotationTag = "rotation";

public:
	RobotPath(const PathGroup *gr, const QString &name, const PathParameters &params);

	const QString& name() const {
		return name_;
	}

	void setName(const QString& name) {
		name_ = name;
		emitPathChangedSignal();
	}

	const PathParameters& params() const {
		return params_;
	}

	void setParams(const PathParameters& p) {
		params_ = p;
		emitPathChangedSignal();
	}

	const PathGroup* pathGroup() const {
		return group_;
	}

	void addWayPoint(const Pose2dWithRotation& waypoint) {
		waypoints_.push_back(waypoint);
		emitPathChangedSignal();
	}

	bool isEmpty() const {
		return waypoints_.size() == 0;
	}

	size_t size() const {
		return waypoints_.size();
	}

	const Pose2dWithRotation& getPoint(size_t index) const {
		return waypoints_[index];
	}

	void replacePoint(size_t index, const Pose2dWithRotation& pt) {
		waypoints_[index] = pt;
		emitPathChangedSignal();

	}

	void removePoint(size_t index) {
		waypoints_.remove(index, 1);
		emitPathChangedSignal();

	}

	void insertPoint(size_t index, const Pose2dWithRotation& pt) {
		waypoints_.insert(index, pt);
		emitPathChangedSignal();

	}

	void convert(const QString& from, const QString& to);

	static std::shared_ptr<RobotPath> fromJSONObject(const PathGroup *group, const QJsonObject& obj, QString &msg);
	QJsonObject toJSONObject();

signals:
	void pathChanged(const QString& groupName, const QString& pathName);

private:
	void emitPathChangedSignal();

	static bool readPoints(std::shared_ptr<RobotPath> path, const QJsonArray& obj, QString &msg);
	static bool readConstraints(std::shared_ptr<RobotPath> path, const QJsonArray& obj, QString &msg);
	static double getDoubleParam(const QJsonObject& obj, const QString& tag, QString& msg);
	static QString getStringParam(const QJsonObject& obj, const QString& tag, QString& msg);
	static QJsonObject getObjectParam(const QJsonObject& obj, const QString& tag, QString& msg);
	static QJsonArray getArrayParam(const QJsonObject& obj, const QString& tag, QString& msg);

private:
	const PathGroup* group_;										// The group this path belongs to
	QString name_;													// The name of the path
	QVector<Pose2dWithRotation> waypoints_;							// The waypoints along the path including robot motion heading and swerve rotation
	QList<std::shared_ptr<PathConstraint>> constraints_;			// The set of constrains to apply to the path
	PathParameters params_;											// The path velocity and acceleration parameters
};
