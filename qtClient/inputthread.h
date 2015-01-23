#ifndef INPUTTHREAD_H
#define INPUTTHREAD_H
#include <QThread>
#include <string>
#include <QString>

class InputThread : public QThread
{ Q_OBJECT
signals:
    void inputSignal(std::string);
public:
    InputThread(QObject*);
    void run();
};

#endif // INPUTTHREAD_H
