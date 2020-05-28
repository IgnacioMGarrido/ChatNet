// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Net/factoryenet.h"
#include "Net/server.h"
#include "Net/buffer.h"

namespace
{
    const int s_iPort = 65785;
}

int main()
{
    Net::CFactory* pFactory = new Net::CFactoryEnet();
    Net::CServer* pServer = pFactory->buildServer();

    pServer->init(s_iPort, 10);
    std::vector<Net::CPacket*> pPackets;
    char sMessage[1024];
    do
    {
        pServer->service(pPackets);
        for (std::vector<Net::CPacket*>::iterator it = pPackets.begin(); it != pPackets.end(); ++it)
        {
            Net::CPacket* pPacket = *it;
            switch (pPacket->getType())
            {
                case Net::CONNECTION:
                {
                    std::cout << "a new client has been connected\n";
                }
                break;
                case Net::DATA:
                {
                    Net::CBuffer oData;
                    oData.write(pPacket->getData(), pPacket->getDataLength());
                    oData.reset();
                    size_t iSize = oData.getSize();
                    oData.read(sMessage, iSize);
                    sMessage[iSize] = '\0';
                    std::cout << sMessage << "\n";
                }
                break;
                case Net::DISCONNECTION:
                {
                    std::cout << "a client has been disconnected\n";
                }
                break;
                default:
                    break;
            }
            delete pPacket;
        }
        pPackets.clear();
    } while (strcmp(sMessage, "closeserver") != 0);

    pServer->release();
    delete pServer;
    delete pFactory;
}
