// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Net/factoryenet.h"
#include "Net/client.h"
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
    std::vector<Net::CPacket*> pPackets;
    Net::NetID xID = 0;
    bool bIDSet = false;
    do
    {
        pClient->service(pPackets);
        for (std::vector<Net::CPacket*>::iterator it = pPackets.begin(); it != pPackets.end(); ++it)
        {
            Net::CPacket* pPacket = *it;
            Net::CBuffer oData;
            oData.write(pPacket->getData(), pPacket->getDataLength());
            oData.reset();
            EMessageType xMessageType;
            oData.read(&xMessageType, sizeof(xMessageType));
            switch(xMessageType) {

            case EMessageType::setID:
                {
                oData.read(&xID, sizeof(xID));
                bIDSet = true;
                }
            break;

            case EMessageType::Message: break;
            default: ;
            }
            delete pPacket;
        }
        pPackets.clear();
    } while (bIDSet == false);
    std::cout << "Write your name: ";
    std::cin >> sMessage;
    Net::CBuffer oData;
    EMessageType messageType = EMessageType::setName;
    oData.write(&messageType, sizeof(messageType));
    oData.write(&xID, sizeof(xID));
    oData.write(sMessage, strlen(sMessage));
    Net::CPacket oSetIDPacket(Net::EPacketType::DATA, oData.getbuffer(), oData.getSize(), pConnection, 0);
    pClient->sendData(pConnection, oSetIDPacket.getData(), oSetIDPacket.getDataLength(), 0, true);
    do 
    {
        pClient->service(pPackets);
        for (std::vector<Net::CPacket*>::iterator it = pPackets.begin(); it != pPackets.end(); ++it)
        {
            Net::CPacket* pPacket = *it;
            switch (pPacket->getType())
            {
            case Net::CONNECTION:
            {
                std::cout << "you have been connected\n";
            }
            break;
            case Net::DATA:
            {
                Net::CBuffer Data(1024, 256);
                Data.write(pPacket->getData(), pPacket->getDataLength());
                Data.reset();

                EMessageType Type;
                Data.read(&Type, sizeof(Type));

                switch (Type)
                {
                case EMessageType::setID:
                {

                }
                break;
                case EMessageType::Message:
                {
                    Net::NetID OtherID;
                    Data.read(&OtherID, sizeof(OtherID));
                    size_t Size = Data.getSize() - sizeof(Type) - sizeof(OtherID);
                    Data.read(sMessage, Size);
                    sMessage[Size] = '\0';
                    std::cout << OtherID << " : " << sMessage << "\n";
                }
                }
            }
                break;
            case Net::DISCONNECTION:
            {
                std::cout << "you have been disconnected\n";
            }
            break;
            default:
                break;
            }
            delete pPacket;
        }
        std::cin >> sMessage;
        if (strcmp(sMessage, "exit") != 0) 
        {
            Net::CBuffer oData(1024, 256);
            EMessageType xType = EMessageType::Message;

            oData.write(&xType, sizeof(xType));
            oData.write(&xID, sizeof(xID));
            oData.write(sMessage, strlen(sMessage));

            Net::CPacket oMessagePacket(Net::DATA, oData.getbuffer(), oData.getSize(), pConnection, 0);
            pClient->sendData(pConnection, oMessagePacket.getData(), oMessagePacket.getDataLength(), 0, true);

        }
        pPackets.clear();

    } while (strcmp(sMessage, "exit") != 0);

    pClient->disconnect(pConnection);
    pClient->release();

    delete pConnection;
    delete pClient;
    delete pFactory;
}
