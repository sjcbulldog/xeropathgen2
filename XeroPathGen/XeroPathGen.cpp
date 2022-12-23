#include "XeroPathGen.h"
#include "CSVWriter.h"
#include "PropertyEditor.h"
#include "EditableProperty.h"
#include "DriveBaseData.h"
#include "AboutDialog.h"
#include "SelectRobotDialog.h"
#include "TrajectoryNames.h"
#include <QtCore/QCoreApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QLabel>
#include <QtGui/QCloseEvent>
#include <QtGui/QActionGroup>
#include <fstream>

XeroPathGen* XeroPathGen::theOne = nullptr;

XeroPathGen::XeroPathGen(const QStringList &arglist, RobotManager& robots, GameFieldManager& fields, std::stringstream& sstrmm, QWidget *parent)
			: QMainWindow(parent), robots_(robots), fields_(fields), strstream_(sstrmm), paths_data_model_(generator_)
{
	theOne = this;
	args_ = arglist;

	QString exedir = QCoreApplication::applicationDirPath();
	QString imagepath = exedir + "/images/icon.png";
	QPixmap image(imagepath);
	QIcon icon(image);
	setWindowIcon(icon);

	path_edit_win_ = nullptr;
	path_win_ = nullptr;
	waypoint_win_ = nullptr;
	plot_win_ = nullptr;
	constraint_win_ = nullptr;
	logwin_ = nullptr;

	dock_path_params_win_ = nullptr;
	dock_path_win_ = nullptr;
	dock_waypoint_win_ = nullptr;
	dock_plot_win_ = nullptr;
	dock_constraint_win_ = nullptr;
	dock_logwin_ = nullptr;

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

	//
	// This is for a new paths data model.  If we read one in, we get the units from
	// the file being read.
	//
	QString units("m");
	if (settings_.contains("units")) {
		units = settings_.value("units").toString();
	}
	paths_data_model_.convert(units);
	setUnits(units);

	recents_ = new RecentFiles(settings_, "recentfiles",  *recent_menu_);
	recents_->initialize(this);

	project_recents_ = new RecentFiles(settings_, "recentprojects", *recent_project_menu_);
	project_recents_->initialize(this);

	populateFieldMenu();
	populateRobotMenu();

	setWindowTitle("Error Code Xero Path Generator (file):");

	connect(&generator_, &GenerationMgr::generationComplete, this, &XeroPathGen::trajectoryGenerationComplete);
	connect(&paths_data_model_, &PathsDataModel::unitsChanged, this, &XeroPathGen::setUnits);
	connect(&paths_data_model_, &PathsDataModel::trajectoryGeneratorChanged, this, &XeroPathGen::trajectoryGeneratorChanged);

	processArguments();
}

XeroPathGen::~XeroPathGen()
{
}

void XeroPathGen::processArguments()
{
	int i = 0;

	while (i < args_.length())
	{
		QString arg = args_.at(i++);
		if (arg == "--fields") {
			i++;
		}
		else if (arg == "--robots") {
			i++;
		}
		else if (arg == "--project") {
			if (i == args_.length()) {
				QMessageBox::critical(nullptr, "Command line error", "The --project option was provided without a project directory");
				break;
			}

			arg = args_.at(i++);
			recentOpenProject(arg);
		}
	}
}

void XeroPathGen::keyPressEvent(QKeyEvent* ev)
{
	if (ev->key() == Qt::Key_Z && (ev->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
	{
		undo();
	}
}

void XeroPathGen::setUnits(const QString& units)
{
	assert(units == paths_data_model_.units());

	//
	// Set everything else to use these units
	//
	fields_.convert(units);
	path_edit_win_->setUnits(units);
	path_win_->setUnits(units);

	path_params_win_->refresh();
	constraint_win_->refresh();
	if (waypoint_win_->getWaypoint() != -1) {
		waypointSelected(waypoint_win_->getWaypoint());
	}

	dock_path_win_->setWindowTitle("Paths: units '" + units + "'");
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// GUI related methods
//
//////////////////////////////////////////////////////////////////////////////////////////

bool XeroPathGen::createWindows()
{
	QList<int> sizes;

	if (settings_.contains(PlotWindowSplitterSize))
	{
		QList<QVariant> stored = settings_.value(PlotWindowSplitterSize).toList();
		for (const QVariant& v : stored) {
			sizes.push_back(v.toInt());
		}
	}

	plot_win_ = new PlotWindow(nullptr, sizes);
	dock_plot_win_ = new QDockWidget(tr("Plot"));
	dock_plot_win_->setObjectName("plot");
	dock_plot_win_->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
	dock_plot_win_->setWidget(plot_win_);
	addDockWidget(Qt::BottomDockWidgetArea, dock_plot_win_);
	dock_plot_win_->hide();

	if (settings_.contains(PlotWindowNodeList)) {
		QStringList list;
		QList<QVariant> stored = settings_.value(PlotWindowNodeList).toList();
		for (const QVariant& v : stored) {
			list.push_back(v.toString());
		}
		plot_win_->setNodeList(list);
	}

	path_edit_win_container_ = new QWidget();
	QVBoxLayout* lay = new QVBoxLayout();
	path_edit_win_container_->setLayout(lay);
	setCentralWidget(path_edit_win_container_);

	path_edit_win_ = new PathFieldView(paths_data_model_, nullptr);
	connect(path_edit_win_, &PathFieldView::mouseMoved, this, &XeroPathGen::mouseMoved);
	connect(path_edit_win_, &PathFieldView::waypointSelected, this, &XeroPathGen::waypointSelected);
	connect(path_edit_win_, &PathFieldView::waypointStartMoving, this, &XeroPathGen::waypointStartMoving);
	connect(path_edit_win_, &PathFieldView::waypointMoving, this, &XeroPathGen::waypointMoving);
	connect(path_edit_win_, &PathFieldView::waypointEndMoving, this, &XeroPathGen::waypointEndMoving);
	connect(path_edit_win_, &PathFieldView::undoRequested, this, &XeroPathGen::undo);
	lay->addWidget(path_edit_win_);

	path_edit_win_slider_ = new QSlider(Qt::Horizontal);
	path_edit_win_slider_->setMinimum(0);
	path_edit_win_slider_->setMaximum(100);
	lay->addWidget(path_edit_win_slider_);
	connect(path_edit_win_slider_, &QSlider::valueChanged, this, &XeroPathGen::sliderChanged);

	path_win_ = new PathWindow(paths_data_model_, nullptr);
	dock_path_win_ = new QDockWidget(tr("Paths"));
	dock_path_win_->setObjectName("paths");
	dock_path_win_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dock_path_win_->setWidget(path_win_);
	addDockWidget(Qt::RightDockWidgetArea, dock_path_win_);
	connect(path_win_, &PathWindow::pathSelected, this, &XeroPathGen::setPath);

	path_params_win_ = new PathParametersWindow(nullptr);
	dock_path_params_win_ = new QDockWidget(tr("Path Parameters"));
	dock_path_params_win_->setObjectName("path-params");
	dock_path_params_win_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dock_path_params_win_->setWidget(path_params_win_);
	addDockWidget(Qt::RightDockWidgetArea, dock_path_params_win_);

	waypoint_win_ = new WaypointWindow(nullptr);
	dock_waypoint_win_ = new QDockWidget(tr("Waypoint"));
	dock_waypoint_win_->setObjectName("waypoint");
	dock_waypoint_win_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dock_waypoint_win_->setWidget(waypoint_win_);
	addDockWidget(Qt::RightDockWidgetArea, dock_waypoint_win_);

	constraint_win_ = new ConstraintEditorWindow(nullptr);
	dock_constraint_win_ = new QDockWidget(tr("Constraints"));
	dock_constraint_win_->setObjectName("constraints");
	dock_constraint_win_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
	dock_constraint_win_->setWidget(constraint_win_);
	addDockWidget(Qt::BottomDockWidgetArea, dock_constraint_win_);
	dock_constraint_win_->hide();

	logwin_ = new LoggerWindow(nullptr);
	dock_logwin_ = new QDockWidget(tr("LogWindow"));
	dock_logwin_->setObjectName("loginw");
	dock_logwin_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
	dock_logwin_->setWidget(logwin_);
	addDockWidget(Qt::BottomDockWidgetArea, dock_logwin_);
	dock_logwin_->hide();
	logger_.setLogWindow(logwin_);

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
	action = file_menu_->addAction(tr("Open Project..."));
	(void)connect(action, &QAction::triggered, this, &XeroPathGen::fileOpenProject);
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
	recent_project_menu_ = file_menu_->addMenu("Recent Projects");
	file_menu_->addSeparator();
	action = file_menu_->addAction(tr("Exit"));
	(void)connect(action, &QAction::triggered, this, &XeroPathGen::fileExit);

	robot_menu_ = new QMenu(tr("&Robots"));
	menuBar()->addMenu(robot_menu_);
	robots_group_ = new QActionGroup(this);

	field_menu_ = new QMenu(tr("&Fields"));
	menuBar()->addMenu(field_menu_);
	fields_group_ = new QActionGroup(this);

	window_menu_ = new QMenu(tr("&Windows"));
	menuBar()->addMenu(window_menu_);
	window_menu_->addAction(dock_path_win_->toggleViewAction());
	window_menu_->addAction(dock_waypoint_win_->toggleViewAction());
	window_menu_->addAction(dock_path_params_win_->toggleViewAction());
	window_menu_->addAction(dock_constraint_win_->toggleViewAction());
	window_menu_->addAction(dock_plot_win_->toggleViewAction());
	window_menu_->addAction(dock_logwin_->toggleViewAction());

	help_menu_ = new QMenu(tr("&Help"));
	menuBar()->addMenu(help_menu_);
	action = help_menu_->addAction(tr("About"));
	(void)connect(action, &QAction::triggered, this, &XeroPathGen::showAbout);
	action = help_menu_->addAction(tr("Changes"));
	(void)connect(action, &QAction::triggered, this, &XeroPathGen::showChanges);

	return true;
}

bool XeroPathGen::createToolbar()
{
	return true;
}

bool XeroPathGen::createStatusBar()
{
	mouse_loc_text_ = new QLabel("Mouse:");
	statusBar()->addWidget(mouse_loc_text_);

	time_text_ = new QLabel("Robot:");
	statusBar()->addWidget(time_text_);

	path_filename_ = new QLabel("<unknown>");
	statusBar()->addPermanentWidget(path_filename_);

	path_gendir_ = new QLabel("<unknown>");
	statusBar()->addPermanentWidget(path_gendir_);

	updateStatusBar();

	return true;
}

void XeroPathGen::mouseMoved(Translation2d pos)
{
	QString str;

	str = "Mouse: ";
	
	str += QString::number(pos.getX(), 'f', 2);
	str += ", ";

	str += QString::number(pos.getY(), 'f', 2);
	mouse_loc_text_->setText(str);
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// Menu related methods
//
//////////////////////////////////////////////////////////////////////////////////////////

void XeroPathGen::showFileMenu()
{
}

void XeroPathGen::fileExit()
{
	QCoreApplication::quit();
}

void XeroPathGen::fileNew()
{
	if (!internalFileClose())
		return;

	project_mode_ = false;
	paths_data_model_.reset();
}

void XeroPathGen::recentOpenProject(const QString& dirname)
{
	QString msg;
	project_mode_ = true;

	QString pathfile = dirname + "/src/main/paths/robot.xeropath";
	QString robotfile = dirname + "/src/main/paths/robot.json";
	QString outdir = dirname + "/src/main/deploy/paths";

	QFileInfo info = QFileInfo(robotfile);
	if (info.exists())
	{
		QFile file(robotfile);
		auto robot = robots_.load(file);
		setRobot(robot);
	}
	else
	{
		//
		// Tell the user what we are doing
		//
		QMessageBox::information(this, "Create Robot", "There is not robot descriptor in the project loaded.  Please configure the robot parameters in the next window.");

		//
		// Create a new robot file, and store it
		//
		createEditRobot(nullptr, robotfile);
	}

	paths_data_model_.reset();

	info = QFileInfo(pathfile);
	if (info.exists())
	{
		paths_data_model_.blockSignals(true);
		if (!paths_data_model_.load(pathfile, msg))
		{
			QMessageBox::critical(this, "Load Failed", "The file '" + pathfile + "' cannot be loaded - " + msg);
		}
		paths_data_model_.blockSignals(false);
		setUnits(paths_data_model_.units());
	}
	else {
		paths_data_model_.setFilename(pathfile);
	}

	project_recents_->addRecentFile(this, dirname);
	paths_data_model_.setOutputDir(outdir);
	path_win_->refresh();

	updateStatusBar();

	setWindowTitle("Error Code Xero Path Generator (project): " + dirname);
}

void XeroPathGen::fileOpenProject()
{
	if (!internalFileClose())
		return;

	QString dirname = QFileDialog::getExistingDirectory(this, tr("Load Xero1425 Project"), "");
	if (dirname.length() == 0) {
		return;
	}

	recentOpenProject(dirname);
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

	project_mode_ = false;

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
		setUnits(paths_data_model_.units());
	}
	paths_data_model_.blockSignals(false);
	recents_->addRecentFile(this, filename);
	updateStatusBar();
}

void XeroPathGen::fileClose()
{
	internalFileClose();
	project_mode_ = false;
}

void XeroPathGen::fileSave()
{
	internalFileSave();
}

void XeroPathGen::fileSaveAs()
{
	if (project_mode_) {
		QMessageBox::information(this, "Not Allowed", "Save As is not allowed when in project mode");
	}
	else {
		internalFileSaveAs();
	}
}

void XeroPathGen::recentOpen(const QString& name, const QString& filename)
{
	if (name == "recentfiles")
		recentOpen(filename);
	else
		recentOpenProject(filename);
}

void XeroPathGen::recentOpen(const QString& filename)
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
			setUnits(paths_data_model_.units());
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

	QString filename = QFileDialog::getSaveFileName(this, tr("Save Path File"), "", tr("Path File (*.xeropath);;All Files (*)"));
	if (filename.length() == 0)
		return false;

	QString msg;
	if (!paths_data_model_.saveAs(filename, msg)) {
		QMessageBox::critical(this, "Save As Failed", "The paths cannot be saved to the file '" + filename + "' - " + msg);
		ret = false;
	}

	updateStatusBar();

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

	setPath(nullptr);
	paths_data_model_.reset();
	path_win_->refresh();

	return true;
}

void XeroPathGen::fileGenerateAs()
{
	if (current_robot_ == nullptr) {
		QMessageBox::information(this, "Invalid Action", "You cannot generate paths until a robot is created.  Either load a project or use the Robot/Create New menu options");
		return;
	}

	if (project_mode_) {
		QMessageBox::information(this, "Invalid Action", "You cannot use generate as when a project is loaded.  The path files will be generated into the project in the correct locaion");
		return;
	}

	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.length() == 0)
		return;

	QFileInfo info(dir);
	if (!info.exists()) 
	{
		QMessageBox::critical(this, "Invalid Directory", "the path '" + dir + "' does not exist");
		return;
	}

	if (!info.isDir())
	{
		QMessageBox::critical(this, "Invalid Directory", "the path '" + dir + "' exists but is not a directory");
		return;
	}

	paths_data_model_.setOutputDir(dir);
	updateStatusBar();

	fileGenerate();
}

void XeroPathGen::fileGenerate()
{
	if (current_robot_ == nullptr) {
		QMessageBox::information(this, "Invalid Action", "You cannot generate paths until a robot is created.  Either load a project or use the Robot/Create New menu options");
		return;
	}

	if (!paths_data_model_.hasOutpuDir()) 
	{
		fileGenerateAs();
	}
	else {

		QDir dirobj = QDir(paths_data_model_.outputDir());
		if (!dirobj.exists()) {
			if (!dirobj.mkpath(dirobj.absolutePath())) {
				QMessageBox::critical(this, "Error!", "Cannot create the destination directory '" + dirobj.absolutePath() + "' - path generation failed");
				return;
			}
		}

		updateAllPaths(true);

		//
		// Now all paths have been processed
		//
		for (auto path : paths_data_model_.getAllPaths()) {
			auto trajgrp = generator_.getTrajectoryGroup(path);
			generateOnePath(path, trajgrp);
		}
	}
}

void XeroPathGen::generateOnePath(std::shared_ptr<RobotPath> path, std::shared_ptr<TrajectoryGroup> group)
{
	QVector<QString> headers =
	{
		RobotPath::TimeTag,
		RobotPath::XTag,
		RobotPath::YTag,
		RobotPath::PositionTag,
		RobotPath::VelocityTag,
		RobotPath::AccelerationTag,
		RobotPath::HeadingTag,
		RobotPath::CurvatureTag,
		RobotPath::RotationTag,
	};

	for (const QString& name : group->trajectoryNames())
	{
		auto traj = group->getTrajectory(name);
		QDir dirobj = QDir(paths_data_model_.outputDir());
		QString filename = dirobj.absoluteFilePath(path->pathGroup()->name() + "-" + path->name() + "-" + name + ".csv");

		std::ofstream outstrm(filename.toStdString());
		CSVWriter::write<QVector<Pose2dWithTrajectory>::const_iterator>(outstrm, headers, traj->begin(), traj->end());
	}
}

void XeroPathGen::updateStatusBar()
{
	if (paths_data_model_.hasFilename()) 
	{
		QFileInfo info(paths_data_model_.filename());
		path_filename_->setText("PathFile: " + info.fileName());
	}
	else
	{
		path_filename_->setText("PathFile: <not set>");
	}

	if (paths_data_model_.hasOutpuDir()) {
		path_gendir_->setText("Output Directory: " + paths_data_model_.outputDir());
	}
	else {
		path_gendir_->setText("Output Directory: <not set>");
	}

	if (!project_mode_) {
		if (paths_data_model_.hasFilename()) {
			setWindowTitle("Error Code Xero Path Generator (file): " + paths_data_model_.filename());
		}
		else {
			setWindowTitle("Error Code Xero Path Generator (file):");
		}
	}
}

void XeroPathGen::updateAllPaths(bool wait)
{
	generator_.clear();

	for (auto path : paths_data_model_.getAllPaths())
	{
		generator_.addPath(paths_data_model_.generatorType(), path);
	}

	if (wait) {
		while (!generator_.isEmpty()) {
			QCoreApplication::processEvents();
			QThread::msleep(10);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// Events and associated methods
//
//////////////////////////////////////////////////////////////////////////////////////////

void XeroPathGen::messageLogger(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	const char* file = context.file ? context.file : "";
	const char* function = context.function ? context.function : "";
	QString complete;

	switch (type) {
	case QtDebugMsg:
		complete += "Debug: ";
		break;
	case QtInfoMsg:
		complete += "Info: ";
		break;
	case QtWarningMsg:
		complete += "Warning: ";
		break;
	case QtCriticalMsg:
		complete += "Critical: ";
		break;
	case QtFatalMsg:
		complete += "Error: ";
		break;
	}
	complete += msg;
	theOne->logger_.addMessage(complete);
}

void XeroPathGen::closeEvent(QCloseEvent* ev)
{
	if (paths_data_model_.isDirty())
	{
		int reply;

		QMessageBox question;
		question.setText("The paths have been modified.");
		question.setInformativeText("Do you want to save your changes?");
		question.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		question.setDefaultButton(QMessageBox::Save);
		reply = question.exec();
		if (reply == QMessageBox::Cancel) {
			ev->ignore();
			return;
		}
		else if (reply == QMessageBox::Save)
		{
			if (!internalFileSave()) {
				ev->ignore();
				return;
			}
		}
	}

	settings_.setValue(GeometrySetting, saveGeometry());
	settings_.setValue(WindowStateSetting, saveState());
	QList<QVariant> param;

	if (plot_win_->isSplitterPositionValid()) {
		for (auto size : plot_win_->getSplitterPosition()) {
			param.push_back(QVariant(size));
		}
		settings_.setValue(PlotWindowSplitterSize, param);
	}

	param.clear();
	const QStringList& list = plot_win_->nodeList();
	for (const QString& node : list) {
		param.push_back(QVariant(node));
	}
	settings_.setValue(PlotWindowNodeList, param);

	QMainWindow::closeEvent(ev);
}

void XeroPathGen::showEvent(QShowEvent* ev)
{
	QMainWindow::showEvent(ev);

	qInstallMessageHandler(messageLogger);

	std::string line;
	while (std::getline(strstream_, line))
	{
		QString qline = QString::fromStdString(line);
		qline = qline.trimmed();
		logger_.addMessage(qline);
	}
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

//
// This is called when a specific path is selected in the path window.  It 
// tells all of the windows which path is the current path so they can update
// their display.  Note, this can also be called by things like close() to
// indicate that no path is selected.  In this case the path value is nullptr.
//
void XeroPathGen::setPath(std::shared_ptr<RobotPath> path)
{
	auto trajgrp = generator_.getTrajectoryGroup(path);

	path_edit_win_->setPath(path);
	constraint_win_->setPath(path);
	waypoint_win_->setPath(path);
	path_params_win_->setPath(path);

	if (trajgrp != nullptr) {
		setTrajectoryGroup(trajgrp);
	}
}

void XeroPathGen::setTrajectoryGroup(std::shared_ptr<TrajectoryGroup> group)
{
	auto main = group->getTrajectory(TrajectoryName::Main);
	path_edit_win_->setTrajectory(main);

	if (main == nullptr) {
		//
		// If we have a group, but have no main trajectory, this means trajectory generation
		// failed.  Lets tell the path window so it can display this is some way.
		//
		path_win_->trajectoryGenerationError(group->path(), true);
	}
	else {
		path_win_->trajectoryGenerationError(group->path(), false);
	}

	if (group->path() == path_win_->selectedPath()) {
		path_params_win_->setTrajectory(main);
		plot_win_->setTrajectoryGroup(group);
		waypointSelected(waypoint_win_->getWaypoint());

		path_edit_win_->setTrajectory(main);
		path_edit_win_slider_->setValue(0);
		if (main == nullptr) {
			path_edit_win_slider_->setMaximum(0);
		}
		else {
			path_edit_win_slider_->setMaximum(static_cast<int>(main->getEndTime() * 1000));
		}
	}
}


void XeroPathGen::setRobot(const QString& name)
{
	setRobot(robots_.getRobotByName(name));
}

void XeroPathGen::setRobot(std::shared_ptr<RobotParams> robot)
{
	current_robot_ = robot;
	path_edit_win_->setRobot(current_robot_);
	generator_.setRobot(current_robot_);
	path_win_->setRobot(current_robot_);

	//
	// Now check the right robot in the menu
	//
	if (project_mode_) 
	{
		for (auto action : robot_menu_->actions())
		{
			if (!action->isSeparator() && action != edit_robot_action_)
			{
				action->setDisabled(true);
			}
		}
	}
	else {
		for (auto action : robot_menu_->actions())
		{
			if (action->isSeparator())
				break;

			if (action->text() == robot->getName())
			{
				action->setChecked(true);
				break;
			}
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
	else
		setRobot(static_cast<std::shared_ptr<RobotParams>>(nullptr));
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
	createEditRobot(nullptr, "");
	updateAllPaths(false);
}

void XeroPathGen::editRobotAction()
{
	createEditRobot(current_robot_, "");
	setRobot(current_robot_) ;
	updateAllPaths(false);
}

void XeroPathGen::showRobotMenu()
{
}

void XeroPathGen::deleteRobotAction()
{
	if (robots_.getRobots().size() == 1)
	{
		std::string msg = "Only a single robot exists and it cannot be deleted";
		QMessageBox box(QMessageBox::Icon::Critical,
			"Error", msg.c_str(), QMessageBox::StandardButton::Ok);
		box.exec();
	}
	else
	{
		SelectRobotDialog dialog(robots_, current_robot_);

		if (dialog.exec() == QDialog::Accepted)
		{
			QString str = dialog.getSelectedRobot();
			if (current_robot_ != nullptr && str == current_robot_->getName())
			{
				QMessageBox box(QMessageBox::Icon::Critical,
					"Error", "You cannot delete the current robot", QMessageBox::StandardButton::Ok);
				box.exec();
				return;
			}

			if (robots_.deleteRobot(str))
			{
				auto actlist = robot_menu_->actions();
				auto it = std::find_if(actlist.begin(), actlist.end(), [str](QAction* item) { return item->text() == str; });
				robot_menu_->removeAction(*it);
			}
		}
	}
}

void XeroPathGen::exportCurrentRobot()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Robot File"), "", tr("Path File (*.robot);;All Files (*)"));
	if (filename.length() == 0)
		return;

	QFile file(filename);
	robots_.save(current_robot_, file);
}

void XeroPathGen::importRobot()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Load Path File"), "", tr("Path File (*.robot);;All Files (*)"));
	if (filename.length() == 0)
		return;

	QFile file(filename);
	try
	{
		auto robot = robots_.importRobot(file);
		QAction* newRobotAction = new QAction(robot->getName());
		robots_group_->addAction(newRobotAction);
		robot_menu_->insertAction(robot_seperator_, newRobotAction);
		newRobotAction->setCheckable(true);
		(void)connect(newRobotAction, &QAction::triggered, this, [this, robot] { newRobotSelected(robot); });
		setRobot(robot->getName());
	}
	catch (const std::runtime_error& error)
	{
		std::string msg = "Could not import robot file '";
		msg += filename.toStdString();
		msg += "' - ";
		msg += error.what();

		QMessageBox box(QMessageBox::Icon::Critical,
			"Error", msg.c_str(), QMessageBox::StandardButton::Ok);
		box.exec();
	}
}

void XeroPathGen::waypointSelected(int index)
{
	auto dists = paths_data_model_.getDistancesForPath(waypoint_win_->path());
	double dist = 0.0;
	if (index < dists.size())
	{
		dist = dists[index];
	}

	waypoint_win_->setWaypoint(index, dist);
}

void XeroPathGen::waypointStartMoving(int index)
{
	paths_data_model_.enableGeneration(false);
	waypoint_win_->refresh();
}

void XeroPathGen::waypointMoving(int index)
{
	waypoint_win_->refresh();
}

void XeroPathGen::waypointEndMoving(int index)
{
	paths_data_model_.enableGeneration(true);
	waypoint_win_->refresh();
}

void XeroPathGen::createEditRobot(std::shared_ptr<RobotParams> robot, const QString &path)
{
	double elength, ewidth, rlength, rwidth, rweight;
	double velocity, accel, timestep;
	double cent;
	RobotParams::DriveType drivetype;
	QString lengthunits, weightunits;
	QString name;
	bool create = (robot == nullptr);
	QString title;

	if (robot == nullptr)
	{
		//
		// Creating a new robot, use defaults
		//
		elength = RobotParams::DefaultLength;
		ewidth = RobotParams::DefaultWidth;
		rlength = RobotParams::DefaultLength;
		rwidth = RobotParams::DefaultWidth;
		velocity = RobotParams::DefaultMaxVelocity;
		accel = RobotParams::DefaultMaxAcceleration;
		cent = RobotParams::DefaultCentripetal;
		rweight = RobotParams::DefaultWeight;
		timestep = RobotParams::DefaultTimestep;
		drivetype = RobotParams::DefaultDriveType;
		lengthunits = RobotParams::DefaultLengthUnits;
		weightunits = RobotParams::DefaultWeightUnits;

		title = "Create Robot";
	}
	else
	{
		elength = robot->getWheelBaseLength();
		ewidth = robot->getWheelBaseWidth();
		rlength = robot->getBumberLength();
		rwidth = robot->getBumberWidth();
		rweight = robot->getRobotWeight();
		velocity = robot->getMaxVelocity();
		accel = robot->getMaxAccel();
		timestep = robot->getTimestep();
		drivetype = robot->getDriveType();
		name = robot->getName();
		lengthunits = robot->getLengthUnits();
		weightunits = robot->getWeightUnits();
		title = "Edit Robot";
	}

	if (path.length() > 0) {
		title += ": " + path;
	}

	while (1)
	{
		PropertyEditor* editor = new PropertyEditor(title, this);
		PropertyEditorTreeModel& model = editor->getModel();
		std::shared_ptr<EditableProperty> prop;

		prop = std::make_shared<EditableProperty>(RobotDialogName, EditableProperty::PropertyType::PTString,
			name, "The name of the robot", !create);
		model.addProperty(prop);

		prop = std::make_shared<EditableProperty>(RobotDialogLengthUnits, EditableProperty::PropertyType::PTStringList,
			QVariant(lengthunits), "The units of measurement for lengths, can differ from the paths");
		auto list = UnitConverter::getAllLengthUnits();
		for (auto& unit : list)
			prop->addChoice(unit);
		model.addProperty(prop);

		prop = std::make_shared<EditableProperty>(RobotDialogWeightUnits, EditableProperty::PropertyType::PTStringList,
			QVariant(weightunits), "The units of measurement for weight");
		list = UnitConverter::getAllWeightUnits();
		for (auto& unit : list)
			prop->addChoice(unit);
		model.addProperty(prop);

		prop = std::make_shared<EditableProperty>(RobotDialogELength, EditableProperty::PropertyType::PTDouble,
			QString::number(elength), "The effective length of the robot");
		model.addProperty(prop);

		prop = std::make_shared<EditableProperty>(RobotDialogEWidth, EditableProperty::PropertyType::PTDouble,
			QString::number(ewidth), "The effective width of the robot");
		model.addProperty(prop);

		prop = std::make_shared<EditableProperty>(RobotDialogRLength, EditableProperty::PropertyType::PTDouble,
			QString::number(rlength), "The physical length of the robot (outside bumper to outside bumper)");
		model.addProperty(prop);

		prop = std::make_shared<EditableProperty>(RobotDialogRWidth, EditableProperty::PropertyType::PTDouble,
			QString::number(rwidth), "The physical width of the robot (outside bumper to outside bumpter)");
		model.addProperty(prop);

		prop = std::make_shared<EditableProperty>(RobotDialogWeight, EditableProperty::PropertyType::PTDouble,
			QString::number(rweight), "The physical weight of the robot (including battery and bumpers)");
		model.addProperty(prop);

		prop = std::make_shared<EditableProperty>(RobotDialogMaxVelocity, EditableProperty::PropertyType::PTDouble,
			QString::number(velocity), "The maximum velocity of the robot");
		model.addProperty(prop);

		prop = std::make_shared<EditableProperty>(RobotDialogMaxAcceleration, EditableProperty::PropertyType::PTDouble,
			QString::number(accel), "The maximum acceleration of the robot");
		model.addProperty(prop);

		prop = std::make_shared<EditableProperty>(RobotDialogDriveType, EditableProperty::PropertyType::PTStringList,
			QVariant(DriveBaseData::typeToName(drivetype)), "The drive type for the robot");
		auto drivetypes = RobotParams::getDriveTypes();
		QList<QVariant> dtypes;
		for (auto type : drivetypes)
			prop->addChoice(DriveBaseData::typeToName(type));
		model.addProperty(prop);

		prop = std::make_shared<EditableProperty>(RobotDialogTimeStep, EditableProperty::PropertyType::PTDouble,
			QString::number(timestep), "The time interval for the drive base control loop");
		model.addProperty(prop);

		if (editor->exec() == QDialog::Rejected)
		{
			delete editor;
			return;
		}

		elength = model.getProperty(RobotDialogELength)->getValue().toDouble();
		ewidth = model.getProperty(RobotDialogEWidth)->getValue().toDouble();
		rlength = model.getProperty(RobotDialogRLength)->getValue().toDouble();
		rwidth = model.getProperty(RobotDialogRWidth)->getValue().toDouble();
		rweight = model.getProperty(RobotDialogWeight)->getValue().toDouble();
		velocity = model.getProperty(RobotDialogMaxVelocity)->getValue().toDouble();
		accel = model.getProperty(RobotDialogMaxAcceleration)->getValue().toDouble();
		drivetype = DriveBaseData::nameToType(model.getProperty(RobotDialogDriveType)->getValue().toString());
		timestep = model.getProperty(RobotDialogTimeStep)->getValue().toDouble();
		lengthunits = model.getProperty(RobotDialogLengthUnits)->getValue().toString();
		weightunits = model.getProperty(RobotDialogWeightUnits)->getValue().toString();

		if (create && model.getProperty(RobotDialogName)->getValue().toString().length() == 0)
		{
			QString msg("The robot name is empty, a robot name must be supplied.");
			QMessageBox box(QMessageBox::Icon::Critical,
				"Error", msg, QMessageBox::StandardButton::Ok);
			box.exec();

			delete editor;
			continue;
		}

		if (create && robots_.exists(model.getProperty(RobotDialogName)->getValue().toString()))
		{
			QString msg("A robot with the name'");
			msg += model.getProperty("name")->getValue().toString();
			msg += "' alread exists, please choose another name";
			QMessageBox box(QMessageBox::Icon::Critical,
				"Error", msg, QMessageBox::StandardButton::Ok);
			box.exec();

			delete editor;
			continue;
		}

		if (create)
			robot = std::make_shared<RobotParams>(model.getProperty(RobotDialogName)->getValue().toString());

		robot->setWheelBaseWidth(ewidth);
		robot->setWheelBaseLength(elength);
		robot->setBumberWidth(rwidth);
		robot->setBumberLength(rlength);
		robot->setRobotWeight(rweight);
		robot->setMaxVelocity(velocity);
		robot->setMaxAcceleration(accel);
		robot->setTimestep(timestep);
		robot->setDriveType(drivetype);
		robot->setLengthUnits(lengthunits);
		robot->setWeightUnits(weightunits);

		if (create)
		{
			if (!path.isEmpty()) {
				setRobot(robot);
				QFile file(path);
				RobotManager::save(robot, file);
			}
			else {
				if (!robots_.add(robot))
				{
					QMessageBox box(QMessageBox::Icon::Critical,
						"Error", "Cannot save robot to robot direcotry",
						QMessageBox::StandardButton::Ok);
					return;
				}
				QAction* newRobotAction = new QAction(robot->getName());
				robots_group_->addAction(newRobotAction);
				robot_menu_->insertAction(robot_seperator_, newRobotAction);
				newRobotAction->setCheckable(true);
				(void)connect(newRobotAction, &QAction::triggered, this, [this, robot] { newRobotSelected(robot); });
				setRobot(robot->getName());
			}
		}
		else
		{
			if (!path.isEmpty()) {
				QFile file(path);
				RobotManager::save(robot, file);
			}
			else {
				if (!robots_.save(robot))
				{
					QMessageBox box(QMessageBox::Icon::Critical,
						"Error", "Cannot save robot to robot direcotry",
						QMessageBox::StandardButton::Ok);
					return;
				}
			}
		}
		delete editor;
		break;
	}
}

void XeroPathGen::showAbout()
{
	AboutDialog about(fields_);
	about.exec();
}

void XeroPathGen::showChanges()
{
	QString exedir = QCoreApplication::applicationDirPath();
	QString textpath = exedir + "/Changes.txt";

	QFile file(textpath);
	if (!file.open(QIODeviceBase::ReadOnly))
		return;

	QByteArray data = file.readAll();
	file.close();

	QString str = QString::fromUtf8(data);

	QMainWindow* win = new QMainWindow();
	QPlainTextEdit* text = new QPlainTextEdit();

	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);

	text->setFont(font);
	text->appendPlainText(str);

	QTextCursor cursor = text->textCursor();
	cursor.movePosition(QTextCursor::Start);
	text->setTextCursor(cursor);

	win->setCentralWidget(text);

	win->setMinimumWidth(800);
	win->setMinimumHeight(600);
	win->show();
}

void XeroPathGen::trajectoryGeneratorChanged()
{
	generator_.clear();
	generator_.removeAllTrajectories();

	//
	// This will reset all of the various windows to
	// reflect the udpated data
	//
	setPath(path_edit_win_->getPath());
	updateAllPaths(false);
}

void XeroPathGen::trajectoryGenerationComplete(std::shared_ptr<RobotPath> path)
{
	auto group = generator_.getTrajectoryGroup(path);
	assert(group != nullptr);

	setTrajectoryGroup(group);
}

void XeroPathGen::sliderChanged(int value)
{
	QString text;
	double time = static_cast<double>(value) / 1000.0;
	path_edit_win_->setTrajectoryTime(time);
	text = "Time: " + QString::number(time, 'f', 1);

	auto trajgrp = generator_.getTrajectoryGroup(path_win_->selectedPath());
	if (trajgrp != nullptr) {
		auto traj = trajgrp->getTrajectory(TrajectoryName::Main);
		if (traj) {
			int index = traj->getIndex(time);
			if (index >= 0 && index < traj->size()) {
				const Pose2dWithTrajectory& pose = (*traj)[index];
				text += ",  X: " + QString::number(pose.translation().getX(), 'f', 2);
				text += ",  Y: " + QString::number(pose.translation().getY(), 'f', 2);
				text += ",  Heading: " + QString::number(pose.rotation().toDegrees(), 'f', 2);
				text += ",  SwerveRotation: " + QString::number(pose.swrot().toDegrees(), 'f', 2);
			}
		}
	}
	time_text_->setText(text);
}

void XeroPathGen::undo()
{
}