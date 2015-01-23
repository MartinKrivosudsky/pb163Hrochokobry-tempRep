#include "client.h"
#include "server.h"
#include <QString>
#include <iostream>
#include <QApplication>
#include <time.h>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    std::string choose;
    std::cout << "Enter '1' to connect to server or '2' to simulate two clients." << std::endl;
    std::getline(std::cin, choose);

    if(strncmp(choose.c_str(),"1",1) == 0)
    {
        Client * client = new Client();
        client->startSSL("88.88.88.120", 8888);
    }

    if(strncmp(choose.c_str(),"2",2) == 0)
    {
        std::string test;
        std::cout << "Enter '1' for fisrt client or '2' for sedond client." << std::endl;

        std::getline(std::cin, test);

        if(strncmp(test.c_str(),"1",1) == 0)
        {
            Server * serverino = new Server(7777);
            std::cout << "Turn on second client and enter any character" << std::endl;
            std::getline(std::cin, test);
            Client * client = new Client(true);
            std::string serverIP = "127.0.0.1";
            client->start(serverIP.c_str(), 8888);
        }

        if(strncmp(test.c_str(),"2",2) == 0)
        {
            Server * serverino = new Server(8888);
            std::cout << "Turn on second client and enter any character" << std::endl;
            std::getline(std::cin, test);
            Client * client = new Client(false);
            std::string serverIP = "127.0.0.1";
            client->start(serverIP.c_str(), 7777);
        }
    }

    return app.exec();
}

