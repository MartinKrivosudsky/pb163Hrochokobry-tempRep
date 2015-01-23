#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"
#include "logfile.h"
#include "string"

// seconds to sleep in test with file locking
// should be more than 1000
const int milisec_to_sleep = 5000;

TEST_CASE( "get_logFile test: same pointer returned" ) {
    LogFile* temp1 = LogFile::get_logFile("test.txt");
    LogFile* temp2 = LogFile::get_logFile("test.txt");
    LogFile* temp3 = LogFile::get_logFile("test.txt");
    REQUIRE( temp1 == temp1 );
    REQUIRE( temp1 == temp2 );
    REQUIRE( temp1 == temp3 );
    REQUIRE( temp2 == temp3 );
}

TEST_CASE( "get_logFile test: no returnig NULL" ) {
    LogFile* temp1 = LogFile::get_logFile("test.txt");
    REQUIRE_FALSE( temp1 == NULL );
}

TEST_CASE( "make_log test: simple log" ) {
    LogFile* temp1 = LogFile::get_logFile("test.txt");
    temp1->make_log("Muhahaha1");
    temp1->make_log("Muhahaha2");
    temp1->make_log("Muhahaha3");

    ifstream input("test.txt");

    string line;
    string our_line;
    while ( getline(input, line) )
    {
        our_line = line;
    }

    size_t pos = our_line.find(";", 0);
    string to_compare = our_line.substr(pos+1, line.size() - (pos+1));

    REQUIRE( to_compare.compare("Muhahaha3") == 0 );
    REQUIRE( to_compare.compare("Muhahaha2") != 0 );
    REQUIRE( to_compare.compare("Muhahaha1") != 0 );
}

void block_file()
{
    LogFile* temp = LogFile::get_logFile("test.txt");
    HANDLE temp2 = LogFile::get_mutex();
    temp->make_log("FALSE POSITIVE");

    WaitForSingleObject(temp2, INFINITE);
    Sleep(milisec_to_sleep);
    ReleaseMutex(temp2);
}

TEST_CASE( "make_log test: log into locked file" )
{
    LogFile* temp1 = LogFile::get_logFile("test.txt");
    DWORD ThreadID;

    CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE) block_file, NULL, 0, &ThreadID);
    Sleep(1000);
    temp1->make_log("Muhahaha4");
    ifstream input("test.txt");

    string line;
    string our_line;
    while ( getline(input, line) )
    {
        our_line = line;
    }

    size_t pos = our_line.find(";", 0);
    string to_compare = our_line.substr(pos+1, line.size() - (pos+1));

    REQUIRE( to_compare.compare("Muhahaha4") == 0 );
}

TEST_CASE( "make_log test: log into locked file, FALSE case" )
{
    LogFile* temp1 = LogFile::get_logFile("test.txt");
    DWORD ThreadID;

    CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE) block_file, NULL, 0, &ThreadID);
    Sleep(1000);

    temp1->make_log("Muhahaha4");
    ifstream input("test.txt");

    string line;
    string our_line;
    while ( getline(input, line) )
    {
        our_line = line;
    }

    size_t pos = our_line.find(";", 0);
    string to_compare = our_line.substr(pos+1, line.size() - (pos+1));

    REQUIRE_FALSE( to_compare.compare("FALSE POSITIVE") == 0 );
}
