
#pragma once

#include <QThread>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>

enum Command
{
    NONE,
    DB,
    MESSAGE
};

class Console : public QThread
{
Q_OBJECT
public:
    Console( QObject* parent );

    void run();

public slots:
    void execute();

private:
    Command m_command;

    QWidget* m_window;
    QTextEdit* m_textBox;
    QLineEdit* m_line;
    QLabel* m_label;

};
