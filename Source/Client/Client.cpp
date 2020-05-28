// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Net/factoryenet.h"
#include "Net/client.h"
#include "Net/buffer.h"
namespace
{
    const int s_iPort = 65785;
}
int main()
{
    Net::CFactory* pFactory = new Net::CFactoryEnet();
    Net::CClient* pClient = pFactory->buildClient();

    pClient->init(1,0,0);
    std::cout << "write the server IP: ";

    char sMessage[1024];
    std::cin >> sMessage;

    Net::CConnection* pConnection = pClient->connect(sMessage, s_iPort, 1, 5000u);

    std::cout << "write your message: (\"exit\" to close connection.)";

    do 
    {
        std::cin >> sMessage;
        if (strcmp(sMessage, "exit") != 0) 
        {
            Net::CBuffer oData(1024, 256);
            oData.write(sMessage, strlen(sMessage));

            Net::CPacket oMessagePacket(Net::DATA, oData.getbuffer(), oData.getSize(), pConnection, 0);
            pClient->sendData(pConnection, oMessagePacket.getData(), oMessagePacket.getDataLength(), 0, true);
        }
    } while (strcmp(sMessage, "exit") != 0);

    pClient->disconnect(pConnection);
    pClient->release();

    delete pConnection;
    delete pClient;
    delete pFactory;
}
