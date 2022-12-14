#include "XeroPathGen.h"
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QLabel>
#include <QtGui/QCloseEvent>
#include <QtGui/QActionGroup>

XeroPathGen* XeroPathGen::theOne = nullptr;

XeroPathGen::XeroPathGen(RobotManager& robots, GameFieldManager& fields, std::ofstream& ostrm, std::stringstream& sstrmm, QWidget *parent)
			: QMainWindow(parent), robots_(robots), fields_(fields), logstream_(ostrm), strstream_(sstrmm), paths_data_model_(generator_)
{
	theOne = this;

	path_edit_win_ = nullptr;
	path_win_ = nullptr;
	waypoint_win_ = nullptr;

	dock_path_params_win_ = nullptr;
	dock_path_win_ = nullptr;
	dock_waypoint_win_ = nullptr;

	createWindows();
	createMenus();
	createToolbar();
	createStatusBar();

	if (settings_.contains(GeometrySetting))
		restoreGeometry(settings_.value(GeometrySetting).toByteArray());

	if (settings_.contains(WindowStateSetting))
		restoreState(settings_.value(WindowStateSetting).toByteArray());

	setDefaultField();
	setDefaultRobot();

	QString units("m");
	if (settings_.contains("units")) {
		units = settings_.value("units").toString();
	}
	setUnits(units);

	recents_ = new RecentFiles(settings_, *recent_menu_);
	recents_->initialize(this);

	populateFieldMenu();
	populateRobotMenu();
}

XeroPathGen::~XeroPathGen()
{
}

void XeroPathGen::setUnits(const QString& units)
{
	paths_data_model_.convert(units);
	fields_.convert(units);
	robots_.convert(units);
	path_edit_win_->setUnits(units);
	units_ = units;
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// GUI related methods
//
//////////////////////////////////////////////////////////////////////////////////////////

bool XeroPathGen::createWindows()
{
	path_edit_win_ = new PathFieldView(paths_data_model_, nullptr);
	connect(path_edit_win_, &PathFieldView::mouseMoved, this, &XeroPathGen::mouseMoved);
	connect(path_edit_win_, &PathFieldView::waypointSelected, this, &XeroPathGen::waypointSelected);
	connect(path_edit_win_, &PathFieldView::waypointStartMoving, this, &XeroPathGen::waypointStartMoving);
	connect(path_edit_win_, &PathFieldView::waypointMoving, this, &XeroPathGen::waypointMoving);
	connect(path_edit_win_, &PathFieldView::waypointEndMoving, this, &XeroPathGen::waypointEndMoving);
	setCentralWidget(path_edit_win_);

	path_win_ = new PathWindow(paths_data_model_, nullptr);
	dock_path_win_ = new QDockWidget(tr("Path Groups"));
	dock_path_win_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dock_path_win_->setWidget(path_win_);
	addDockWidget(Qt::RightDockWidgetArea, dock_path_win_);
	connect(path_win_, &PathWindow::pathSelected, this, &XeroPathGen::pathSelected);

	path_params_win_ = new PathParametersWindow(nullptr);
	dock_path_params_win_ = new QDockWidget(tr("Path Parameters"));
	dock_path_params_win_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dock_path_params_win_->setWidget(path_params_win_);
	addDockWidget(Qt::RightDockWidgetArea, dock_path_params_win_);

	waypoint_win_ = new WaypointWindow(nullptr);
	dock_waypoint_win_ = new QDockWidget(tr("Waypoint Parameters"));
	dock_waypoint_win_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dock_waypoint_win_->setWidget(waypoint_win_);
	addDockWidget(Qt::RightDockWidgetArea, dock_waypoint_win_);

	return true;
}

bool XeroPathGen::createMenus()
{
	QAction* action;

	file_menu_ = new QMenu(tr("&File"));
	(void)connect(file_menu_, &QMenu::aboutToShow, this, &XeroPathGen::showFileMenu);
	menuBar()->addMenu(file_menu_);
	action = file_menu_->addAction(tr("New"));
	(void)connect(action, &QAction::triggered, this, &XeroPathGen::fileNew);
	action = file_menu_->addAction(tr("Open ..."));
	(void)connect(action, &QAction::triggered, this, &XeroPathGen::fileOpen);
	file_menu_->addSeparator();
	file_save_ = file_menu_->addAction(tr("Save"));
	(void)connect(file_save_, &QAction::triggered, this, &XeroPathGen::fileSave);
	action = file_menu_->addAction(tr("Save As ..."));
	(void)connect(action, &QAction::triggered, this, &XeroPathGen::fileSaveAs);
	file_menu_->addSeparator();
	action = file_menu_->addAction(tr("Close"));
	(void)connect(action, &QAction::triggered, this, &XeroPathGen::fileClose);
	file_menu_->addSeparator();
	action = file_menu_->addAction(tr("Generate Paths As ..."));
	(void)connect(action, &QAction::triggered, this, &XeroPathGen::fileGenerateAs);
	action = file_menu_->addAction(tr("Generate Paths"));
	(void)connect(action, &QAction::triggered, this, &XeroPathGen::fileGenerate);
	file_menu_->addSeparator();
	recent_menu_ = file_menu_->addMenu("Recent Files");

	robot_menu_ = new QMenu(tr("&Robots"));
	menuBar()->addMenu(robot_menu_);
	robots_group_ = new QActionGroup(this);

	field_menu_ = new QMenu(tr("&Fields"));
	menuBar()->addMenu(field_menu_);
	fields_group_ = new QActionGroup(this);

	return true;
}

bool XeroPathGen::createToolbar()
{
	return true;
}

bool XeroPathGen::createStatusBar()
{
	xpos_text_ = new QLabel("X:");
	xpos_text_->setFixedWidth(80);
	statusBar()->insertWidget(0, xpos_text_);

	ypos_text_ = new QLabel("Y:");
	ypos_text_->setFixedWidth(80);
	statusBar()->insertWidget(1, ypos_text_);

	path_filename_ = new QLabel("<unknown>");
	statusBar()->insertWidget(2, path_filename_);

	path_gendir_ = new QLabel("<unknown>");
	statusBar()->insertWidget(3, path_gendir_);

	return true;
}

void XeroPathGen::mouseMoved(Translation2d pos)
{
	QString str;
	
	str = QString::number(pos.getX(), 'f', 2);
	xpos_text_->setText(str);

	str = QString::number(pos.getY(), 'f', 2);
	ypos_text_->setText(str);
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// Menu related methods
//
//////////////////////////////////////////////////////////////////////////////////////////

void XeroPathGen::showFileMenu()
{
}

void XeroPathGen::fileNew()
{
	if (!internalFileClose())
		return;

	paths_data_model_.reset();
}

void XeroPathGen::fileOpen()
{
	static const char* FileLoadPathTag = "FileLoadPath";
	if (!internalFileClose())
		return;

	paths_data_model_.reset();
	QString dir;

	if (settings_.contains(FileLoadPathTag)) {
		dir = settings_.value(FileLoadPathTag).toString();
	}

	QString filename = QFileDialog::getOpenFileName(this, tr("Load Path File"), dir, tr("Path File (*.path);;All Files (*)"));
	if (filename.length() == 0)
		return;

	QFileInfo info(filename);
	settings_.setValue(FileLoadPathTag, info.absolutePath());

	QString msg;
	paths_data_model_.blockSignals(true);
	if (!paths_data_model_.load(filename, msg)) 
	{
		QMessageBox::critical(this, "Load Failed", "The file '" + filename + "' cannot be loaded - " + msg);
	}
	else 
	{
		path_win_->refresh();
		QFileInfo info(paths_data_model_.filename());
		path_filename_->setText(info.fileName());

		if (paths_data_model_.hasOutpuDir()) {
			path_gendir_->setText(paths_data_model_.outputDir());
		}
		else {
			path_gendir_->setText("<not set>");
		}
	}
	paths_data_model_.blockSignals(false);
	recents_->addRecentFile(this, filename);
}

void XeroPathGen::fileClose()
{
	internalFileClose();
}

void XeroPathGen::fileSave()
{
	internalFileSave();
}

void XeroPathGen::fileSaveAs()
{
	internalFileSaveAs();
}

void XeroPathGen::recentOpen(const QString &filename)
{
	if (!internalFileClose())
		return;

	paths_data_model_.reset();

	QFileInfo info(filename);
	if (!info.exists()) {
		recents_->removeRecentFile(this, filename);
	}
	else {
		QString msg;
		paths_data_model_.blockSignals(true);
		if (!paths_data_model_.load(filename, msg))
		{
			QMessageBox::critical(this, "Load Failed", "The file '" + filename + "' cannot be loaded - " + msg);
		}
		else
		{
			path_win_->refresh();
			QFileInfo info(paths_data_model_.filename());
			path_filename_->setText(info.fileName());

			if (paths_data_model_.hasOutpuDir()) {
				path_gendir_->setText(paths_data_model_.outputDir());
			}
			else {
				path_gendir_->setText("<not set>");
			}
		}
		paths_data_model_.blockSignals(false);
		recents_->addRecentFile(this, filename);
	}
}

bool XeroPathGen::internalFileSave()
{
	bool ret = true;

	if (paths_data_model_.hasFilename()) {
		QString msg;
		if (!paths_data_model_.save(msg)) {
			QMessageBox::critical(this, "Save As Failed", "The paths cannot be saved to the file '" + paths_data_model_.filename() + "' - " + msg);
			ret = false;
		}
	}
	else {
		ret = internalFileSaveAs();
	}

	return ret;
}

bool XeroPathGen::internalFileSaveAs()
{
	bool ret = true;

	QFileDialog dialog;

	QString filename = QFileDialog::getSaveFileName(this, tr("Save Path File"), "", tr("Path File (*.path);;All Files (*)"));
	if (filename.length() == 0)
		return false;

	QString msg;
	if (!paths_data_model_.saveAs(filename, msg)) {
		QMessageBox::critical(this, "Save As Failed", "The paths cannot be saved to the file '" + filename + "' - " + msg);
		ret = false;
	}

	return ret;
}

bool XeroPathGen::internalFileClose()
{
	if (paths_data_model_.isDirty()) 
	{
		QMessageBox question;
		question.setText("The paths have been modified.");
		question.setInformativeText("Do you want to save your changes?");
		question.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		question.setDefaultButton(QMessageBox::Save);
		int ret = question.exec();
		if (ret == QMessageBox::Cancel)
			return false;

		if (ret == QMessageBox::Save)
		{
			if (!internalFileSave()) {
				return false;
			}
		}
	}

	paths_data_model_.reset();
	return true;
}

void XeroPathGen::fileGenerateAs()
{

}

void XeroPathGen::fileGenerate()
{

}

//////////////////////////////////////////////////////////////////////////////////////////
//
// Events and associated methods
//
//////////////////////////////////////////////////////////////////////////////////////////

void XeroPathGen::messageLogger(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	theOne->addLogMessage(msg);
}

void XeroPathGen::closeEvent(QCloseEvent* ev)
{
	if (paths_data_model_.isDirty())
	{
		QMessageBox::StandardButton reply;

		reply = QMessageBox::question(this, "Question?", "You have unsaved changes, do you want to save?", QMessageBox::Yes | QMessageBox::No);
		if (reply == QMessageBox::Yes)
		{
			if (!internalFileSave()) {
				ev->ignore();
			}
			return;
		}
	}

	settings_.setValue(GeometrySetting, saveGeometry());
	settings_.setValue(WindowStateSetting, saveState());

	QMainWindow::closeEvent(ev);
}

void XeroPathGen::showEvent(QShowEvent* ev)
{
	QMainWindow::showEvent(ev);

	// qInstallMessageHandler(messageLogger);

	std::string line;
	while (std::getline(strstream_, line))
	{
		QString qline = QString::fromStdString(line);
		qline = qline.trimmed();
		this->addLogMessage(qline);
	}

	if (robots_.getRobots().size() == 0)
	{
		std::shared_ptr<RobotParams> robot = std::make_shared<RobotParams>("DefaultRobot");
		robot->setEffectiveWidth(28.0);
		robot->setEffectiveLength(28.0);
		robot->setRobotWidth(28.0);
		robot->setRobotLength(28.0);
		robot->setMaxVelocity(120);
		robot->setMaxAcceleration(120);
		robot->setMaxJerk(1200);
		robot->setTimestep(0.02);
		robot->setDriveType(RobotParams::DriveType::TankDrive);
		robots_.add(robot);
	}
}

void XeroPathGen::pathSelected(const QString& grname, const QString& pathname)
{
	auto path = paths_data_model_.getPathByName(grname, pathname);
	path_edit_win_->setPath(path);
	waypoint_win_->setPath(path);
	path_params_win_->setPath(path);
}

void XeroPathGen::setField(const QString &name)
{
	//
	// Set the current field
	//
	current_field_ = fields_.getFieldByName(name);
	auto actions = field_menu_->actions();
	for (auto action : actions)
	{
		action->setCheckable(true);
		action->setChecked(false);
	}
	path_edit_win_->setField(current_field_);

	if (current_field_ != nullptr)
	{
		for (auto action : actions)
		{
			if (action->text() == name)
				action->setChecked(true);
		}
	}
}

void XeroPathGen::setDefaultField()
{
	QString field = settings_.value("field").toString();
	if (field.length() == 0 || !fields_.exists(field))
	{
		auto deffield = fields_.getDefaultField();
		if (deffield != nullptr)
			field = deffield->getName();
	}

	if (field.length() > 0)
		setField(field);
}

void XeroPathGen::setRobot(const QString& name)
{
	current_robot_ = robots_.getRobotByName(name);
	path_edit_win_->setRobot(current_robot_);

	//
	// Now check the right robot in the menu
	//
	for (auto action : robot_menu_->actions())
	{
		if (action->isSeparator())
			break;

		if (action->text() == name)
		{
			action->setChecked(true);
			break;
		}
	}
}
void XeroPathGen::setDefaultRobot()
{
	QString robot = settings_.value("robot").toString();
	if (robot.length() == 0 || !robots_.exists(robot))
	{
		auto defrobot = robots_.getDefaultRobot();
		if (defrobot != nullptr)
			robot = defrobot->getName();
	}
	if (robot.length())
		setRobot(robot);
}

void XeroPathGen::populateRobotMenu()
{
	bool first = true;
	for (auto robot : robots_.getRobots())
	{
		QAction* act = new QAction(robot->getName());
		act->setCheckable(true);
		act->setChecked(first);
		if (first)
			first = false;
		robot_menu_->addAction(act);
		robots_group_->addAction(act);
		(void)connect(act, &QAction::triggered, this, [this, robot] { newRobotSelected(robot); });
	}

	robot_seperator_ = robot_menu_->addSeparator();
	QAction* action = robot_menu_->addAction(tr("New Robot ..."));
	(void)connect(action, &QAction::triggered, this, &XeroPathGen::newRobotAction);

	edit_robot_action_ = robot_menu_->addAction(tr("Edit Current Robot ..."));
	(void)connect(edit_robot_action_, &QAction::triggered, this, &XeroPathGen::editRobotAction);

	action = robot_menu_->addAction(tr("Delete Robot"));
	(void)connect(action, &QAction::triggered, this, &XeroPathGen::deleteRobotAction);

	robot_menu_->addSeparator();

	action = robot_menu_->addAction(tr("Export Current Robot ..."));
	(void)connect(action, &QAction::triggered, this, &XeroPathGen::exportCurrentRobot);

	action = robot_menu_->addAction(tr("Import Robot ..."));
	(void)connect(action, &QAction::triggered, this, &XeroPathGen::importRobot);
}

void XeroPathGen::populateFieldMenu()
{
	for (auto field : fields_.getFields())
	{
		QAction* act = field_menu_->addAction(field->getName());
		fields_group_->addAction(act);
		(void)connect(act, &QAction::triggered, this, [this, field] { newFieldSelected(field); });
	}
}

void XeroPathGen::newFieldSelected(std::shared_ptr<GameField> field)
{
	if (field != current_field_)
	{
		setField(field->getName());
		settings_.setValue("field", field->getName());
	}
}

void XeroPathGen::newRobotSelected(std::shared_ptr<RobotParams> robot)
{
	if (robot != current_robot_)
	{
		setRobot(robot->getName());
		settings_.setValue("robot", robot->getName());
	}
}

void XeroPathGen::newRobotAction()
{

}

void XeroPathGen::editRobotAction()
{

}

void XeroPathGen::showRobotMenu()
{

}

void XeroPathGen::deleteRobotAction()
{

}

void XeroPathGen::exportCurrentRobot()
{

}

void XeroPathGen::importRobot()
{
}

void XeroPathGen::waypointSelected(size_t index)
{
	waypoint_win_->setWaypoint(index);
}

void XeroPathGen::waypointStartMoving(size_t index)
{
	waypoint_win_->refresh();
}

void XeroPathGen::waypointMoving(size_t index)
{
	waypoint_win_->refresh();
}

void XeroPathGen::waypointEndMoving(size_t index)
{
	waypoint_win_->refresh();

}
