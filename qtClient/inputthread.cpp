#include "inputthread.h"
#include <iostream>

InputThread::InputThread(QObject* obj) : QThread(obj)
{
    qRegisterMetaType<std::string>("std::string");
}

void InputThread::run()
{
    while(true)
    {
        std::string buffer;
        std::getline(std::cin, buffer);
        emit inputSignal(buffer);
    }
}
