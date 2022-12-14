//
// Copyright 2019 Jack W. Griffin
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

#include "GameField.h"
#include "RobotParams.h"
#include "RobotPath.h"
#include "Pose2d.h"
#include "SplinePair.h"
#include "PathsDataModel.h"
#include <QWidget>
#include <QPixmap>
#include <QTransform>
#include <memory>

class PathFileTreeModel;

class PathFieldView : public QWidget
{
	friend class ShowRobotWindow;

	Q_OBJECT

public:
	PathFieldView(PathsDataModel& model, QWidget *parent = Q_NULLPTR);
	virtual ~PathFieldView();

	const std::vector<QImage*>& markerImages() {
		return marker_images_;
	}

	void setField(std::shared_ptr<GameField> field);
	void setPath(std::shared_ptr<RobotPath> path);
	void setRobot(std::shared_ptr<RobotParams> params);
	void setUnits(const QString &units);

	std::shared_ptr<RobotPath> getPath() {
		return path_;
	}

	QPointF worldToWindow(const QPointF& pt);
	QPointF windowToWorld(const QPointF& pt);
	std::vector<QPointF> worldToWindow(const std::vector<QPointF>& points);
	std::vector<QPointF> windowToWorld(const std::vector<QPointF>& points);

	void deleteWaypoint();
	void deleteWaypoint(const QString &group, const QString &path, size_t index);
	void insertWaypoint();
	void addWaypoint(const QString& group, const QString& path, size_t index, const Pose2dWithRotation &pt);

	size_t getSelected() const {
		return selected_;
	}

	bool isInsertWaypointValid();
	bool isDeleteWaypointValid();

	void doPaint(QPainter& paint, bool printing = false);

signals:
	void mouseMoved(Translation2d pos);
	void waypointSelected(size_t index);
	void waypointStartMoving(size_t index);
	void waypointMoving(size_t index);
	void waypointEndMoving(size_t index);
	void waypointInserted();
	void waypointDeleted();

protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

private:
	static constexpr double ArrowBodyLength = 32.0;
	static constexpr double ArrowHeadOffset = 8.0;
	static constexpr double TriangleSize = 16.0;
	static constexpr double SplinePathDiameter = 3.0;
	static constexpr double CenterHitDistance = 8.0;
	static constexpr double RotateBubbleHalo = 5.0;
	static constexpr double CircleRadius = 4.0;
	static constexpr double BigWaypointMove = 12.0;
	static constexpr double SmallWaypointMove = 1.0;
	static constexpr double BigWaypointRotate = 5.0;
	static constexpr double SmallWaypointRotate = 0.5;

	static constexpr const char* FlagImage = "flag.png";
	static constexpr const char* Marker1Image = "marker1.png";
	static constexpr const char* Marker2Image = "marker2.png";
	static constexpr const char* Marker3Image = "marker3.png";
	static constexpr const char* Marker4Image = "marker4.png";


	std::vector<QPointF> triangle_;
	std::vector<QPointF> arrow_;

	enum WaypointRegion
	{
		None,
		Center,
		Rotate
	};

private:
	void copyCoordinates();
	void pasteCoordinates(bool rot180);

	void pathChanged(const QString& grname, const QString& pathname);

	void emitMouseMoved(Translation2d pos);
	void emitWaypointSelected(size_t which);
	void emitWaypointStartMoving(size_t which);
	void emitWaypointMoving(size_t which);
	void emitWaypointEndMoving(size_t which);
	void emitWaypointDeleted();
	void emitWaypointInserted();

	void moveWaypoint(bool shift, int dx, int dy);
	void rotateWaypoint(bool shift, int dir);

	std::vector<QPointF> transformPoints(QTransform& trans, const std::vector<QPointF>& points);
	void createTransforms();
	void drawPath(QPainter& paint);
	void drawPoints(QPainter& paint);
	void drawOnePoint(QPainter& paint, const Pose2dWithRotation& pt, bool selected);
	void drawSplines(QPainter &paint);
	void drawSpline(QPainter& paint, std::shared_ptr<SplinePair> pair);
	void findSplineStep(std::shared_ptr<SplinePair> pair);
	void drawRobot(QPainter& paint);
	bool hitTestWaypoint(const QPointF& pt, size_t& index, WaypointRegion& region);
	void invalidateWaypoint(size_t index);

private:
	QImage field_image_;
	QImage flagimage_;
	std::vector<QImage*> marker_images_;
	std::vector<QPoint> marker_offsets_;

	std::shared_ptr<GameField> field_;
	std::shared_ptr<RobotPath> path_;
	double image_scale_;
	QTransform world_to_window_;
	QTransform window_to_world_;
	std::shared_ptr<RobotParams> robot_;
	size_t selected_;
	QString units_;
	bool dragging_;
	bool rotating_;
	size_t demo_step_;

	double triangle_size_;

	QPointF world_;

	PathsDataModel& path_data_model_;
};
