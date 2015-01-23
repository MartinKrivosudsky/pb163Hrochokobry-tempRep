#include <iostream>
#include "Server.h"

using namespace std;

#include "server.h"
#include <QApplication>

LogFile* g_log = LogFile::get_logFile( "log.txt" );

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    Server server;

    return app.exec();
}
