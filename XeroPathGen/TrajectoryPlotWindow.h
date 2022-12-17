#pragma once

#include "TrajectoryGroup.h"
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QtWidgets/QWidget>

class TrajectoryPlotWindow : public QChartView
{
public:
	TrajectoryPlotWindow(const QVector<QString> &varnames, QWidget* parent);

	void clear();
	void setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group);

protected:
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
	void dragLeaveEvent(QDragLeaveEvent* event) override;
	void dropEvent(QDropEvent* ev) override;

	bool viewportEvent(QEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void keyPressEvent(QKeyEvent* event);

private:
	enum class AxisType
	{
		Distance,
		Speed,
		Acceleration,
		Curvature,
		Angle
	};

private:
	AxisType mapVariableToAxis(const QString& var);
	void insertNode(const QString& node);
	QValueAxis* createYAxis(const QString& node);
	void setupLegend();
	void setupTimeAxis();

private:
	std::shared_ptr<TrajectoryGroup> group_;

	QValueAxis* time_axis_;
	QMap<AxisType, QValueAxis*> y_axis_;
	const QVector<QString>& varnames_;
	bool left_right_;
	QStringList nodes_;
	QPoint last_mouse_;
};

