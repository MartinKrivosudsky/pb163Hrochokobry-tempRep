#ifndef LOGFILE_H
#define LOGFILE_H

#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <windows.h>

using namespace std;

class LogFile
{
private:
    string file_name;

    /**
    *	@brief private constructor for loggFile, - for getting logFile object use get_logFile() - SINGLETON pattern
    *
    *   @string fileName name of file to write loggs
    *
    *   @return LogFile to existing logFile
    **/
    LogFile(string fileName);
public:

    /**
    *	@brief getter for LogFile - SINGLETON pattern, return LogFile object, if already exist, return it, else
    *   created it and then return
    *
    *   @string buffer data to write into file
    **/
    static LogFile* get_logFile(string name);

    /**
    *	@brief log function for windows to write log's data into file loggs.txt, now in synchronous approach,
    *   later locks or mutexes will be add.
    *   Log data in format:
    *   [year],[month],[day],[hour],[minute],[second],[milisecond];[data]\n
    *
    *   @string buffer data to write into file
    **/
    void make_log(string data);

    /**
    *	@brief getter for created mutex, only for tests cases
    *
    *   @return HANDLE to mutex
    **/
    static HANDLE get_mutex();

    ~LogFile();
};


#endif // LOGFILE_H
