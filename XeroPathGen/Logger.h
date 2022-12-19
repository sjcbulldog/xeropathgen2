#pragma once

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <fstream>


class LoggerWindow;

class Logger
{
public:
	Logger();

	const QStringList& messages() const {
		return messages_;
	}

	void setLogWindow(LoggerWindow* win) {
		logwin_ = win;
	}

	void addMessage(const QString& msg);

private:
	LoggerWindow* logwin_;
	QStringList messages_;
	QString filename_;
	bool new_session_;
};

