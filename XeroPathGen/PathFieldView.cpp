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
#include "PathFieldView.h"
#include "SplinePair.h"
#include "PathsDataModel.h"
#include "RobotPath.h"
#include "PathGroup.h"
#include "Pose2d.h"
#include <QtCore/QPointF>
#include <QtCore/QCoreApplication>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QMouseEvent>
#include <QtGui/QFontMetrics>
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include <cmath>

PathFieldView::PathFieldView(PathsDataModel&model, QWidget *parent) : QWidget(parent), path_data_model_(model)
{
	QImage* im;

	units_ = "in";
	setMouseTracking(true);
	setFocusPolicy(Qt::ClickFocus);
	selected_ = std::numeric_limits<int>::max();
	image_scale_ = 1.0;
	dragging_ = false;
	rotating_ = false;
	traj_time_ = 0.0;
	heading_ = false;
}

PathFieldView::~PathFieldView()
{
}

void PathFieldView::setUnits(const QString& units)
{
	for (int i = 0; i < triangle_.size(); i++)
	{
		double x = UnitConverter::convert(triangle_[i].rx(), units_, units);
		double y = UnitConverter::convert(triangle_[i].ry(), units_, units);
		triangle_[i] = QPointF(x, y);
	}

	for (int i = 0; i < arrow_.size(); i++)
	{
		double x = UnitConverter::convert(arrow_[i].rx(), units_, units);
		double y = UnitConverter::convert(arrow_[i].ry(), units_, units);
		arrow_[i] = QPointF(x, y);
	}

	robot_width_ = UnitConverter::convert(robot_width_, units_, units);
	robot_length_ = UnitConverter::convert(robot_length_, units_, units);

	units_ = units;
	createTransforms();
	repaint();
}

bool PathFieldView::isInsertWaypointValid()
{
	if (path_ == nullptr)
		return false;

	//
	// The -1 is because insert is also not valid if the last waypoint is selected
	//
	return selected_ < path_->size() - 1;
}

bool PathFieldView::isDeleteWaypointValid()
{
	if (path_ == nullptr)
		return false;

	return selected_ < path_->size() - 1 && selected_ != 0;
}

QSize PathFieldView::minimumSizeHint() const
{
	return QSize(field_image_.size().width() / 4, field_image_.size().height() / 4);
}

QSize PathFieldView::sizeHint() const
{
	return field_image_.size();
}

void PathFieldView::paintEvent(QPaintEvent* event)
{
	(void)event;

	//
	// Draw the field
	//
	QPainter paint(this);
	doPaint(paint);
}

void PathFieldView::doPaint(QPainter &paint, bool printing)
{
	QRectF rect(0.0f, 0.0f, field_image_.width() * image_scale_, field_image_.height() * image_scale_);
	paint.drawImage(rect, field_image_);

	//
	// Draw the path
	//
	if (path_ != nullptr)
	{
		drawPath(paint);

		if (robot_ != nullptr) {
			drawRobot(paint);
		}
	}
}

void PathFieldView::emitMouseMoved(Translation2d pos)
{
	emit mouseMoved(pos);
}

void PathFieldView::emitWaypointSelected(int index)
{
	emit waypointSelected(index);
}

void PathFieldView::emitWaypointDeleted()
{
	emit waypointDeleted();
}

void PathFieldView::emitWaypointInserted()
{
	emit waypointInserted();
}

void PathFieldView::emitWaypointEndMoving(int index)
{
	emit waypointEndMoving(index);
}

void PathFieldView::emitWaypointMoving(int index)
{
	emit waypointMoving(index);
}

void PathFieldView::emitWaypointStartMoving(int index)
{
	emit waypointStartMoving(index);
}

void PathFieldView::mouseMoveEvent(QMouseEvent* ev)
{
	world_ = windowToWorld(ev->localPos());

	if (dragging_)
	{
		const Pose2dWithRotation& pt = path_->getPoint(selected_);
		Translation2d t(world_.rx(), world_.ry());
		Pose2dWithRotation newpt(t, pt.getRotation(), pt.getSwrot());
		path_->replacePoint(selected_, newpt);

		emitWaypointMoving(selected_);
		repaint(geometry());
	}
	else if (rotating_)
	{
		const Pose2dWithRotation& pt = path_->getPoint(selected_);
		double dy = world_.ry() - pt.getTranslation().getY();
		double dx = world_.rx() - pt.getTranslation().getX();
		double angle = std::atan2(dy, dx) - MathUtils::kPI / 2;
		Rotation2d r = Rotation2d::fromRadians(angle);

		if (heading_) {
			Pose2dWithRotation newpt(pt.getTranslation(), r, pt.getSwrot());
			path_->replacePoint(selected_, newpt);
		}
		else {
			Pose2dWithRotation newpt(pt.getTranslation(), pt.getRotation(), r);
			path_->replacePoint(selected_, newpt);
		}

		emitWaypointMoving(selected_);
		repaint(geometry());
	}
	emitMouseMoved(Translation2d(world_.rx(), world_.ry()));
}

void PathFieldView::mousePressEvent(QMouseEvent* ev)
{
	if (ev->buttons() != Qt::LeftButton)
		return;

	bool shift = (ev->modifiers() == Qt::KeyboardModifier::ShiftModifier);

	if (path_ != nullptr) {
		int index;
		WaypointRegion region;
		if (hitTestWaypoint(QPointF(ev->x(), ev->y()), index, region))
		{
			grabMouse();

			if (region == WaypointRegion::Center)
			{
				if (selected_ != index)
				{
					if (selected_ != std::numeric_limits<int>::max())
					{
						selected_ = std::numeric_limits<int>::max();
						invalidateWaypoint(selected_);
					}

					selected_ = index;
					invalidateWaypoint(selected_);
					emitWaypointSelected(selected_);
				}
				dragging_ = true;
				emitWaypointStartMoving(selected_);
			}
			else
			{
				rotating_ = true;
				heading_ = !shift;
				emitWaypointStartMoving(selected_);
			}
		}
		else
		{
			if (selected_ != std::numeric_limits<int>::max())
			{
				int save = selected_;
				selected_ = std::numeric_limits<int>::max();
				invalidateWaypoint(save);
				emitWaypointSelected(selected_);
			}
		}
	}
}

void PathFieldView::mouseReleaseEvent(QMouseEvent* ev)
{
  	(void)ev ;
	
	if (dragging_ || rotating_)
		emitWaypointEndMoving(selected_);

	dragging_ = false;
	rotating_ = false;

	releaseMouse();
}

void PathFieldView::deleteWaypoint()
{
	if (path_ == nullptr || selected_ >= path_->size())
		return;

	const Pose2dWithRotation& pt = path_->getPoint(selected_);
	path_->removePoint(selected_);
	emitWaypointDeleted();
	repaint();
}

void PathFieldView::deleteWaypoint(const QString& group, const QString& path, int index)
{
	auto p = path_data_model_.getPathByName(group, path);
	if (p != nullptr)
	{
		p->removePoint(index);
		if (path_ == p)
		{
			repaint();
			emitWaypointDeleted();
		}
	}
}

void PathFieldView::addWaypoint(const QString& group, const QString& path, int index, const Pose2dWithRotation& pt)
{
	auto p = path_data_model_.getPathByName(group, path);
	if (p != nullptr)
	{
		p->insertPoint(index, pt);
		if (path_ == p)
		{
			repaint();
			emitWaypointInserted();
		}
	}
}

void PathFieldView::insertWaypoint()
{
	if (selected_ >= path_->size() - 1)
		return;

	const Pose2dWithRotation& p1 = path_->getPoint(selected_);
	const Pose2dWithRotation& p2 = path_->getPoint(selected_ + 1);

	Rotation2d r = Rotation2d::fromRadians((p1.getRotation().toRadians() + p2.getRotation().toRadians()) / 2.0);
	Translation2d t((p1.getTranslation().getX() + p2.getTranslation().getX()) / 2.0, (p1.getTranslation().getY() + p2.getTranslation().getY()) / 2.0);
	Pose2dWithRotation newpt(t, r, p1.getSwrot());

	path_->insertPoint(selected_, newpt) ;

	// Move selected waypoint to the one we just created
	selected_++;

	emitWaypointInserted();
	emitWaypointSelected(selected_);
	repaint(geometry());
}

void PathFieldView::moveWaypoint(bool shift, int dx, int dy)
{
	if (selected_ <= path_->size())
	{
		emitWaypointStartMoving(selected_);

		double delta = shift ? SmallWaypointMove : BigWaypointMove;
		delta = UnitConverter::convert(delta, "in", units_);

		const Pose2dWithRotation& pt = path_->getPoint(selected_);

		Translation2d t(pt.getTranslation().getX() + dx * delta, pt.getTranslation().getY() + dy * delta);
		Pose2dWithRotation newpt(t, pt.getRotation(), pt.getSwrot());
		path_->replacePoint(selected_, newpt);

		emitWaypointEndMoving(selected_);
		repaint(geometry());
	}
}

void PathFieldView::rotateWaypoint(bool shift, int dir)
{
	if (selected_ <= path_->size())
	{
		emitWaypointStartMoving(selected_);

		double delta = shift ? SmallWaypointRotate : BigWaypointRotate;
		delta *= dir;

		const Pose2dWithRotation& pt = path_->getPoint(selected_);

		Rotation2d r = Rotation2d::fromDegrees(MathUtils::boundDegrees(pt.getRotation().toDegrees() + delta));
		Pose2dWithRotation newpt(pt.getTranslation(), r, pt.getSwrot());
		path_->replacePoint(selected_, newpt);
		emitWaypointEndMoving(selected_);
		repaint(geometry());
	}
}

void PathFieldView::keyPressEvent(QKeyEvent* ev)
{
	if (ev->modifiers() == Qt::KeyboardModifier::ControlModifier)
	{
		if (ev->key() == Qt::Key::Key_C)
		{
			copyCoordinates();
		}
		else if (ev->key() == Qt::Key::Key_V)
		{
			pasteCoordinates(false);
		}
		else if (ev->key() == Qt::Key::Key_Z)
		{
			emit undoRequested();
		}
	}
	else if (ev->modifiers() == (Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier))
	{
		if (ev->key() == Qt::Key::Key_V)
		{
			pasteCoordinates(true);
		}
	}
	else
	{
		bool shift = (ev->modifiers() == Qt::KeyboardModifier::ShiftModifier);

		switch (ev->key())
		{
		case Qt::Key::Key_Delete:
			deleteWaypoint();
			break;
		case Qt::Key::Key_Insert:
			insertWaypoint();
			break;
		case Qt::Key::Key_Up:
			moveWaypoint(shift, 0, 1);
			break;
		case Qt::Key::Key_Down:
			moveWaypoint(shift, 0, -1);
			break;
		case Qt::Key::Key_Left:
			moveWaypoint(shift, -1, 0);
			break;
		case Qt::Key::Key_Right:
			moveWaypoint(shift, 1, 0);
			break;
		case Qt::Key::Key_PageUp:
			rotateWaypoint(shift, 1);
			break;
		case Qt::Key::Key_PageDown:
			rotateWaypoint(shift, -1);
			break;
		case Qt::Key::Key_Plus:
			rotateWaypoint(shift, 1);
			break;
		case Qt::Key::Key_Minus:
			rotateWaypoint(shift, -1);
			break;
		}
	}
}

void PathFieldView::copyCoordinates()
{
	if (selected_ <= path_->size())
	{
		const Pose2dWithRotation& pt = path_->getPoint(selected_);
		QClipboard* clip = QGuiApplication::clipboard();
		QString str = "%%%%";
		str += QString::number(pt.getTranslation().getX());
		str += "," + QString::number(pt.getTranslation().getY());
		str += "," + QString::number(pt.getRotation().toDegrees());
		str += "," + QString::number(pt.getSwrot().toDegrees());
		str += "%%%%";
		clip->setText(str);
	}
}

void PathFieldView::pasteCoordinates(bool b)
{
	QClipboard* clip = QGuiApplication::clipboard();

	if (selected_ <= path_->size())
	{
		QString txt = clip->text();
		if (!txt.startsWith("%%%%"))
			return;

		txt = txt.mid(4);
		if (!txt.endsWith("%%%%"))
			return;

		txt = txt.mid(0, txt.length() - 4);
		QStringList numbers = txt.split(',');
		if (numbers.size() != 4)
			return;

		bool ok;
		double x, y, angle;
		double rot;

		x = numbers.front().toDouble(&ok);
		if (!ok)
			return;
		numbers.pop_front();

		y = numbers.front().toDouble(&ok);
		if (!ok)
			return;
		numbers.pop_front();

		angle = numbers.front().toDouble(&ok);
		if (!ok)
			return;
		numbers.pop_front();

		rot = numbers.front().toDouble(&ok);
		if (!ok)
			return;
		numbers.pop_front();

		Rotation2d r;
		if (b)
			r = Rotation2d::fromDegrees(angle + 180.0);
		else
			r = Rotation2d::fromDegrees(angle);

		emitWaypointStartMoving(selected_);
		Translation2d t(x, y);
		Pose2dWithRotation newpt(t, r, Rotation2d::fromDegrees(rot));
		path_->replacePoint(selected_, newpt);
		emitWaypointEndMoving(selected_);

		repaint(geometry());
	}
}

QVector<QPointF> PathFieldView::transformPoints(QTransform& trans, const QVector<QPointF>& points)
{
	QVector<QPointF> result;

	for (const QPointF& pt : points)
	{
		QPointF newpt = trans.map(pt);
		result.push_back(newpt);
	}

	return result;
}

void PathFieldView::drawWheel(QPainter& paint, QBrush &brush, const Translation2d& loc, const Pose2dWithRotation& pose)
{
	static const double wheelWidth = UnitConverter::convert(2.0, "in", units_);
	static const double wheelLength = UnitConverter::convert(4.0, "in", units_);

	Translation2d fl, fr, bl, br;
	fl = Translation2d(wheelWidth, wheelLength);
	fr = Translation2d(wheelWidth, -wheelLength);
	bl = Translation2d(-wheelWidth, wheelLength);
	br = Translation2d(-wheelWidth, -wheelLength);

	fl = fl.rotateBy(pose.getSwrot()).rotateBy(Rotation2d::fromDegrees(90.0));
	fr = fr.rotateBy(pose.getSwrot()).rotateBy(Rotation2d::fromDegrees(90.0));
	bl = bl.rotateBy(pose.getSwrot()).rotateBy(Rotation2d::fromDegrees(90.0));
	br = br.rotateBy(pose.getSwrot()).rotateBy(Rotation2d::fromDegrees(90.0));

	fl = fl.translateBy(loc);
	fr = fr.translateBy(loc);
	bl = bl.translateBy(loc);
	br = br.translateBy(loc);

	QVector<QPointF> corners;
	corners.push_back(QPointF(fl.getX(), fl.getY()));
	corners.push_back(QPointF(fr.getX(), fr.getY()));
	corners.push_back(QPointF(bl.getX(), bl.getY()));
	corners.push_back(QPointF(br.getX(), br.getY()));

	auto winloc = worldToWindow(corners);

	QPainterPath path;
	path.moveTo(winloc[0]);
	path.lineTo(winloc[1]);
	path.lineTo(winloc[3]);
	path.lineTo(winloc[2]);
	path.lineTo(winloc[0]);
	paint.fillPath(path, brush);
}

void PathFieldView::drawRobot(QPainter& paint)
{
	if (path_ == nullptr || robot_ == nullptr || traj_ == nullptr)
		return;

	int index = traj_->getIndex(traj_time_);
	if (index == -1)
		return;

	const Pose2dWithTrajectory& pose = (*traj_)[index];
	drawRobot(paint, pose.pose(), QColor(0, 153, 0), QColor(255, 128, 0));
}

void PathFieldView::drawRobot(QPainter& paint, const Pose2dWithRotation& pose, QColor body, QColor wheel)
{
	Translation2d fl, fr, bl, br;
	robot_->getLocations(path_->units(), fl, fr, bl, br);
	fl = fl.rotateBy(pose.getSwrot());
	fr = fr.rotateBy(pose.getSwrot());
	bl = bl.rotateBy(pose.getSwrot());
	br = br.rotateBy(pose.getSwrot());

	fl = fl.translateBy(pose.getTranslation());
	fr = fr.translateBy(pose.getTranslation());
	bl = bl.translateBy(pose.getTranslation());
	br = br.translateBy(pose.getTranslation());

	QVector<QPointF> corners;
	corners.push_back(QPointF(fl.getX(), fl.getY()));
	corners.push_back(QPointF(fr.getX(), fr.getY()));
	corners.push_back(QPointF(bl.getX(), bl.getY()));
	corners.push_back(QPointF(br.getX(), br.getY()));

	auto winloc = worldToWindow(corners);

	QBrush brush(wheel);
	paint.setBrush(brush);

	drawWheel(paint, brush, fl, pose);
	drawWheel(paint, brush, fr, pose);
	drawWheel(paint, brush, bl, pose);
	drawWheel(paint, brush, br, pose);

	QPen pen(body);
	paint.setPen(pen);
	pen.setWidthF(2.0f);
	paint.setPen(pen);

	paint.drawLine(winloc[1], winloc[3]);
	paint.drawLine(winloc[2], winloc[3]);
	paint.drawLine(winloc[2], winloc[0]);

	pen.setWidthF(5.0);
	paint.setPen(pen);
	paint.drawLine(winloc[0], winloc[1]);
}

void PathFieldView::drawPath(QPainter &paint)
{
	drawSplines(paint);
	drawPoints(paint);
}

void PathFieldView::drawPoints(QPainter& paint)
{
	for (int i = 0; i < path_->size(); i++)
		drawOnePoint(paint, path_->getPoint(i), i == selected_);
}

void PathFieldView::drawOnePoint(QPainter& paint, const Pose2dWithRotation& pt, bool selected)
{
	if (selected)
	{
		QPen pen(QColor(0xff, 0xff, 0x00, 0xff));
		pen.setWidthF(2.0f);
		paint.setPen(pen);
	}
	else
	{
		QPen pen(QColor(0x00, 0x00, 0x00, 0xFF));
		pen.setWidthF(2.0f);
		paint.setPen(pen);
	}
	QBrush brush(QColor(0xFF, 0xA5, 0x00, 0xFF));
	paint.setBrush(brush);

	QTransform swrot, hding;

	swrot.translate(pt.getTranslation().getX(), pt.getTranslation().getY());
	swrot.rotateRadians(pt.getSwrot().toRadians());

	hding.translate(pt.getTranslation().getX(), pt.getTranslation().getY());
	hding.rotateRadians(pt.getRotation().toRadians());

	//
	// Draw the triangle that represents the path waypoint
	//
	QVector<QPointF> mapped = worldToWindow(transformPoints(hding, triangle_));
	paint.drawPolygon(&mapped[0], 3);

	//
	// Draw the robot outline showing the angle of the robot
	//
	if (selected)
	{
		drawRobot(paint, pt, QColor(255, 255, 0), QColor(255, 255, 0));
	}
}

void PathFieldView::drawSplines(QPainter& paint)
{
	auto splines = path_data_model_.getSplinesForPath(path_) ;
	for (int i = 0; i < splines.size(); i++)
		drawSpline(paint, splines[i]);
}

void PathFieldView::findSplineStep(std::shared_ptr<SplinePair> pair)
{
	double step = 0.1;
	double cx, cy;
	double nx, ny;
	QPointF current, prev;

	while (true) {
		bool first = true;
		double maxdist = 0.0;

		for (double t = 0.0; t < 1.0; t += step) {

			Translation2d loc = pair->evalPosition(t);
			Rotation2d heading = pair->evalHeading(t);

			cx = loc.getX() - robot_width_ * heading.getSin() / 2.0;
			cy = loc.getY() + robot_width_ * heading.getCos() / 2.0;

			current = worldToWindow(QPointF(cx, cy));

			if (!first)
			{
				double dx = std::abs(current.x() - prev.x());
				double dy = std::abs(current.y() - prev.y());

				maxdist = std::max(maxdist, std::max(dx, dy));
			}

			if (maxdist > 1.5)
			{
				step /= 2.0;
				break;
			}

			first = false;
			prev = current;
		}

		if (maxdist <= 2.0)
			break;
	}
	pair->setStep(step);
}

void PathFieldView::drawSpline(QPainter& paint, std::shared_ptr<SplinePair> pair)
{
	double px, py;
	QColor c(0xF0, 0x80, 0x80, 0xFF);

	QBrush brush(c);
	paint.setBrush(brush);

	QPen pen(c);
	paint.setPen(pen);

	if (!pair->hasStep())
		findSplineStep(pair);

	for (float t = 0.0f; t < 1.0f; t += pair->step())
	{
		Translation2d loc = pair->evalPosition(t);
		Rotation2d heading = pair->evalHeading(t);

		px = loc.getX() - robot_width_ * heading.getSin() / 2.0;
		py = loc.getY() + robot_width_ * heading.getCos() / 2.0;

		QPointF qp = worldToWindow(QPointF(px, py));
		paint.drawPoint(qp);

		px = loc.getX() + robot_width_ * heading.getSin() / 2.0;
		py = loc.getY() - robot_width_ * heading.getCos() / 2.0;

		qp = worldToWindow(QPointF(px, py));
		paint.drawPoint(qp);
	}
}

void PathFieldView::resizeEvent(QResizeEvent* event)
{
  	(void)event ;
	createTransforms();
}

void PathFieldView::setField(std::shared_ptr<GameField> field)
{
	field_ = field;

	//
	// Load the image,
	//
	QImage image(field_->getImageFile());
	field_image_ = image;

	//
	// Create new transforms
	//
	createTransforms();

	//
	// Redraw the widget
	//
	repaint(geometry());
}

void PathFieldView::pathChanged(const QString &grname, const QString &pathname)
{
	repaint(geometry());
}

void PathFieldView::setPath(std::shared_ptr<RobotPath> path)
{
	if (path_ != nullptr) {
		disconnect(path_.get(), &RobotPath::afterPathChanged, this, &PathFieldView::pathChanged);
	}

	if (path_ != path)
	{
		path_ = path;
		selected_ = std::numeric_limits<int>::max();
		repaint(geometry());

		if (path_ != nullptr) {
			connect(path_.get(), &RobotPath::afterPathChanged, this, &PathFieldView::pathChanged);
		}
	}
}

void PathFieldView::setRobot(std::shared_ptr<RobotParams> params)
{
	robot_ = params;

	if (robot_ != nullptr) {
		robot_width_ = UnitConverter::convert(robot_->getBumberWidth(), robot_->getLengthUnits(), units_);
		robot_length_ = UnitConverter::convert(robot_->getBumberLength(), robot_->getLengthUnits(), units_);
		drive_type_ = robot_->getDriveType();
	}
	else {
		robot_width_ = UnitConverter::convert(28.0, "in", units_);
		robot_width_ = UnitConverter::convert(28.0, "in", units_);
		drive_type_ = RobotParams::DriveType::SwerveDrive;
	}
}

void PathFieldView::createTransforms()
{
	//
	// First calculate the scale to use to scale the image to the window
	// without changing the aspec ratio.  Pick the largest scale factor that will fit
	// the most constrained dimension
	//
	int w = width();
	int h = height();
	double sx = (double)w / (double)field_image_.width();
	double sy = (double)h / (double)field_image_.height();

	if (sx < sy)
		image_scale_ = sx;
	else
		image_scale_ = sy;

	//
	// Now, we know now to scale the image to window pixels, scale the top left and bottom
	// right coordiantes from the image to the window
	//
	Translation2d tl = field_->getTopLeft().scale(image_scale_);
	Translation2d br = field_->getBottomRight().scale(image_scale_);

	//
	// Now we know two points on the field and how they coorespond to two points on the window.
	// The field point (0, FieldSize.Y) cooresponds to the point tl.
	// The field point (FieldsSize.X, 0) coorespons to the point br.
	//
	// Compute the transform based on this.  
	//
	world_to_window_ = QTransform();

	world_to_window_.translate(tl.getX(), br.getY());
	double scale = (br.getX() - tl.getX()) / field_->getSize().getX();
	world_to_window_.scale(scale, -scale);

	window_to_world_ = world_to_window_.inverted();

	//
	// Define the triangle for waypoints based on the current transform
	//
	triangle_ =
	{
		{ TriangleSize / scale, 0.0},
		{ -TriangleSize / scale / 2.0, TriangleSize / scale / 2.0},
		{ -TriangleSize / scale / 2.0, -TriangleSize / scale / 2.0 }
	};

	arrow_ =
	{
		{ 0.0, 0.0 }, 
		{ ArrowBodyLength / scale, 0.0},
		{ ArrowBodyLength / scale, 0.0},
		{ ArrowBodyLength / scale - ArrowHeadOffset / scale, ArrowHeadOffset / scale},
		{ ArrowBodyLength / scale, 0.0},
		{ ArrowBodyLength / scale - ArrowHeadOffset / scale, -ArrowHeadOffset / scale},
	};
}

QPointF PathFieldView::worldToWindow(const QPointF& pt)
{
	return world_to_window_.map(pt);
}

QPointF PathFieldView::windowToWorld(const QPointF& pt)
{
	return window_to_world_.map(pt);
}

QVector<QPointF> PathFieldView::worldToWindow(const QVector<QPointF> &points)
{
	return transformPoints(world_to_window_, points);
}

QVector<QPointF> PathFieldView::windowToWorld(const QVector<QPointF>& points)
{
	return transformPoints(window_to_world_, points);
}

bool PathFieldView::hitTestWaypoint(const QPointF& pt, int& index, WaypointRegion& region)
{
	if (path_ == nullptr)
		return false;

	for (int i = 0; i < path_->size(); i++)
	{
		Translation2d t2d = path_->getPoint(i).getTranslation();
		QPointF world = worldToWindow(QPointF(t2d.getX(), t2d.getY()));
		double dist = QVector2D(pt - world).length();

		//
		// Test to see if we are close enough to the center of the
		// waypoint to be a h it
		//
		if (dist < CenterHitDistance)
		{
			index = i;
			region = WaypointRegion::Center;
			return true;
		}
	}

	if (selected_ < path_->size())
	{
		Translation2d t2d = path_->getPoint(selected_).getTranslation();
		QPointF world = worldToWindow(QPointF(t2d.getX(), t2d.getY()));
		double dist = QVector2D(pt - world).length();

		if (dist < RotateHitDistance)
		{
			index = selected_;
			region = WaypointRegion::Rotate;
			return true;
		}

	}
	return false;
}

void PathFieldView::invalidateWaypoint(int index)
{
	if (index >= path_->size())
		return;

	//
	// For now, invalidate the entire window
	//
	repaint(geometry());
}
