#pragma once

#include "GameFieldManager.h"
#include "RobotManager.h"
#include "PathFieldView.h"
#include "PathsDataModel.h"
#include "PathWindow.h"
#include "PlotWindow.h"
#include "PathParametersWindow.h"
#include "WaypointWindow.h"
#include "RecentFiles.h"
#include "GenerationMgr.h"
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtCore/QSettings>
#include <fstream>
#include <sstream>

class XeroPathGen : public QMainWindow
{
    friend class RecentFiles;

    Q_OBJECT

public:
    XeroPathGen(RobotManager &robots, GameFieldManager &fields, std::ofstream& ostrm, std::stringstream& sstrmm, QWidget *parent = nullptr);
    ~XeroPathGen();

private:
    // Top level GUI methods
    bool createWindows();
    bool createMenus();
    bool createToolbar();
    bool createStatusBar();

    // Menu related methods
    void showFileMenu();
    void fileNew();
    void fileOpen();
    void fileOpenProject();
    void fileSave();
    void fileSaveAs();
    void fileClose();
    void fileGenerateAs();
    void fileGenerate();
    void recentOpen(const QString& name, const QString& filename);
    void recentOpen(const QString& filename);
    void recentOpenProject(const QString& filename);

protected:
    void closeEvent(QCloseEvent* ev);
    void showEvent(QShowEvent* ev);

private:
    static void messageLogger(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:
    static constexpr const char* GeometrySetting = "geometry";
    static constexpr const char* WindowStateSetting = "windowState";
    static constexpr const char* PlotWindowSplitterSize = "plotWindowSplitterSize";

private:
    void setDefaultField();
    void setField(const QString &name);
    void populateFieldMenu();
    void newFieldSelected(std::shared_ptr<GameField> field);

    void setDefaultRobot();
    void setRobot(const QString& name);
    void setRobot(std::shared_ptr<RobotParams> robot);
    void populateRobotMenu();
    void newRobotSelected(std::shared_ptr<RobotParams> robot);

    void newRobotAction();
    void editRobotAction();
    void showRobotMenu();
    void deleteRobotAction();
    void exportCurrentRobot();
    void importRobot();

    void pathSelected(const QString& grname, const QString& pathname);

    bool internalFileSave();
    bool internalFileSaveAs();
    bool internalFileClose();

    void setUnits(const QString &units);
    void mouseMoved(Translation2d pos);

    void addLogMessage(const QString& msg) {
        logMessages_.push_back(msg.trimmed());
    }

    void waypointSelected(size_t index);
    void waypointStartMoving(size_t index);
    void waypointMoving(size_t index);
    void waypointEndMoving(size_t index);

    void generateOnePath(std::shared_ptr<RobotPath> path, std::shared_ptr<TrajectoryGroup> group);
    void updateStatusBar();
    void updateAllPaths(bool wait);
    void createEditRobot(std::shared_ptr<RobotParams> robot, const QString &path);

    void showAbout();

    void trajectoryGenerationComplete(std::shared_ptr<RobotPath> path);

private:
    static constexpr const char* RobotDialogName = "Name";
    static constexpr const char *RobotDialogEWidth = "Effective Width";
    static constexpr const char *RobotDialogELength = "Effective Length";
    static constexpr const char *RobotDialogRWidth = "Physical Width";
    static constexpr const char *RobotDialogRLength = "Physical Length";
    static constexpr const char *RobotDialogWeight = "Weight";
    static constexpr const char *RobotDialogMaxVelocity = "Max Velocity";
    static constexpr const char *RobotDialogMaxAcceleration = "Max Acceleration";
    static constexpr const char *RobotDialogMaxJerk = "Max Jerk";
    static constexpr const char *RobotDialogMaxCentripetal = "Max Centripetal Force";
    static constexpr const char *RobotDialogLengthUnits = "Units (lengths)";
    static constexpr const char *RobotDialogWeightUnits = "Units (weights)";
    static constexpr const char *RobotDialogDriveType = "Drive Type";
    static constexpr const char *RobotDialogTimeStep = "Time Step";

private:
    static XeroPathGen* theOne;

private:
    QSettings settings_;
    QString units_;

    GameFieldManager& fields_;
    RobotManager& robots_;
    GenerationMgr generator_;
    PathsDataModel paths_data_model_;

    std::shared_ptr<GameField> current_field_;
    std::shared_ptr<RobotParams> current_robot_;

    std::ofstream& logstream_;
    std::stringstream& strstream_;

    // Windows
    PathFieldView* path_edit_win_;
    PathWindow* path_win_;
    PathParametersWindow* path_params_win_;
    WaypointWindow* waypoint_win_;
    PlotWindow* plot_win_;

    // Docking windows
    QDockWidget* dock_path_win_;
    QDockWidget* dock_path_params_win_;
    QDockWidget* dock_waypoint_win_;
    QDockWidget* dock_plot_win_;

    // Menus
    QMenu* file_menu_;
    QMenu* recent_menu_;
    QMenu* recent_project_menu_;
    QAction* file_save_;

    QMenu* robot_menu_;
    QActionGroup* robots_group_;
    QAction* robot_seperator_;
    QAction* edit_robot_action_;

    QMenu* field_menu_;
    QActionGroup* fields_group_;

    QMenu* window_menu_;

    QMenu* help_menu_;

    // Status bar widgets
    QLabel* xpos_text_;
    QLabel* ypos_text_;
    QLabel* path_filename_;
    QLabel* path_gendir_;

    QStringList logMessages_;
    RecentFiles* recents_;
    RecentFiles* project_recents_;

    bool project_mode_;
};
