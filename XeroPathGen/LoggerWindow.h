#pragma once

#include <QtWidgets/QPlainTextEdit>

class LoggerWindow : public QPlainTextEdit
{
public:
	LoggerWindow(QWidget* parent);

	void addMessage(const QString& msg) {
		moveCursor(QTextCursor::End);
		insertPlainText(msg);
		insertPlainText("\n");
		moveCursor(QTextCursor::End);
	}

private:
};

