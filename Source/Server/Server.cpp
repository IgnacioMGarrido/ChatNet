// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Net/factoryenet.h"
#include "Net/server.h"
#include "Net/buffer.h"

namespace
{
    const int s_iPort = 65785;
    enum class EMessageType
    {
        setID,
        setName,
        Message
    };
}

Net::NetID m_xNextID = 0;

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
                    ++m_xNextID;
                    Net::CBuffer oData;
                    EMessageType xType = EMessageType::setID;
                    oData.write(&xType, sizeof(xType));
                    oData.write(&m_xNextID, sizeof(m_xNextID));
                    Net::CPacket oIDPacket(Net::EPacketType::DATA, oData.getbuffer(), oData.getSize(), pPacket->getConnection(), 0);
                    pServer->sendData(pPacket->getConnection(), oIDPacket.getData(), oIDPacket.getDataLength(), 0, true);
                    std::cout << "a new client has been connected\n";
                }
                break;
                case Net::DATA:
                {
                    Net::CBuffer Data(1024, 256);
                    Data.write(pPacket->getData(), pPacket->getDataLength());
                    Data.reset();

                    EMessageType Type = EMessageType::setID;
                    Data.read(&Type, sizeof(Type));

                    switch(Type) {
                    case EMessageType::Message:
                        {
                            Net::NetID xOtherID;
                            Data.read(&xOtherID, sizeof(xOtherID));
                            size_t Size = Data.getSize() - sizeof(Type) - sizeof(xOtherID);
                            Data.read(sMessage, Size);
                            sMessage[Size] = '\0';
                            std::cout << xOtherID << ": " << sMessage << "\n";
                            pServer->sendAll(pPacket->getData(), pPacket->getDataLength(), 0, true);
                        }
                        break;
                    default: ;
                    }

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
