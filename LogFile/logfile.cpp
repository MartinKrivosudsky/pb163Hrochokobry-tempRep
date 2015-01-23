#include "iomanip"
#include "logfile.h"

static LogFile* our_log_file = NULL;
static HANDLE mutex = NULL;

LogFile::LogFile(string new_file_name)
{
    file_name = new_file_name;
}

LogFile::~LogFile()
{
    if (mutex != NULL)
    {
        CloseHandle(mutex);
        mutex = NULL;
    }
}

LogFile* LogFile::get_logFile(string name)
{
    if(our_log_file == NULL)
    {
        our_log_file = new LogFile(name);
        mutex = CreateMutex(NULL, false, NULL);
        if (NULL == mutex)
        {
            fputs("Create mutex error!\n", stderr);
        }
    }
    return our_log_file;
}

void LogFile::make_log(string buffer)
{
    ofstream log_file;
    SYSTEMTIME st;
    GetSystemTime(&st);

    WaitForSingleObject(mutex, INFINITE);

    log_file.open(file_name.c_str(), std::ofstream::app);
    log_file << setfill( '0' );
    log_file << "[" << st.wYear << "-" << setw( 2 ) << st.wMonth << "-" << setw( 2 ) << st.wDay << "," << setw( 2 ) << st.wHour << ":"
             << setw( 2 ) << st.wMinute << ":" << setw( 2 ) << st.wSecond << ":" << setw( 3 ) << st.wMilliseconds << "] ";
    log_file << buffer << endl;
    log_file.close();

    ReleaseMutex(mutex);
}

HANDLE LogFile::get_mutex()
{
    return mutex;
}
