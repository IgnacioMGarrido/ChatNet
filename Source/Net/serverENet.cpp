//---------------------------------------------------------------------------
// servidorENet.cpp
//---------------------------------------------------------------------------

/**
 * @file servidorENet.cpp
 *
 * Implementa un servidor de red con ENet
 *
 *
 * @author Juan A. Recio-García
 * @date Junio, 2006
 */

#include "serverenet.h"
#include "connectionenet.h"

#define DEBUG_SERVER 0

namespace Net {

	CServerENet::CServerENet()
	{
		state = NO_INIT;
	}

	CServerENet::~CServerENet()
	{
		std::vector<CConnection*>::iterator it = connections.begin();
		while (it != connections.end()) {
			delete (*it);
			++it;
		}
		connections.clear();
	}

	bool CServerENet::init(int port, int clients, unsigned int maxinbw, unsigned int maxoutbw)
	{
		if(state != NO_INIT)
			return false;

		if (enet_initialize () != 0)
		{
			printf ("NET>> An error occurred while trying to init ENet.\n");
			return false;
		}
		
		ENetAddress address;

		/* Bind the server to the default localhost.     */
		/* A specific host address can be specified by   */
		/* enet_address_set_host (& address, "x.x.x.x"); */

		address.host = ENET_HOST_ANY;
		/* Bind the server to port. */
		address.port = port;

		server = enet_host_create (& address      /* the address to bind the server host to */, 
									clients       /* allow up to 32 clients and/or outgoing connections */,
									maxinbw       /* assume any amount of incoming bandwidth */,
									maxoutbw      /* assume any amount of outgoing bandwidth */);
		if (server == NULL)
		{
			printf ("NET >> An error occurred while trying to create an ENet server host.\n");
			return false;
		}

		if(DEBUG_SERVER)
			fprintf(stdout, "Server initialized");

		state = INIT_NOT_CONNECTED;

		return true;
	}


	void CServerENet::service(std::vector<CPacket*>&  packets)
	{
		ENetEvent event;
		CPacket* packet;
		CConnectionENet* connection;
   

		/* Wait up to 0 milliseconds for an event. */
		while (enet_host_service (server, & event, 0) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				if(DEBUG_SERVER)
					fprintf(stdout,"A new client connected from %x:%u.\n", 
							event.peer -> address.host,
							event.peer -> address.port);

				connection = new CConnectionENet();
				connection->setENetPeer(event.peer);

				event.peer -> data = connection;
				connections.push_back(connection);


				packet = new CPacket(CONNECTION,0,0,connection,0);
				packets.push_back(packet);
				
				state = INIT_AND_CONNECTED;
				
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				if(DEBUG_SERVER)
					fprintf(stdout,"A packet of length %u was received from %s on channel %u.\n",
				    (unsigned int) event.packet -> dataLength,
				    (char*) event.peer -> data,
				    event.channelID);
				packet = new CPacket(DATA, event.packet->data, event.packet->dataLength, (CConnection*)event.peer->data, event.channelID);
				packets.push_back(packet);
				enet_packet_destroy (event.packet);            
				break;
	           
			case ENET_EVENT_TYPE_DISCONNECT:
				if(DEBUG_SERVER)
					fprintf(stdout,"%s disconected.\n", (char*) event.peer -> data);
				packet = new CPacket(DISCONNECTION,0,0,(CConnection*)event.peer->data,0);
				packets.push_back(packet);
				disconnectReceived((CConnection*)event.peer->data);
				break;
			default:
				fprintf(stdout,"Unknown packet");
			}
		}
	}


	std::vector<CConnection*>::iterator CServerENet::getConnectionsIt()
	{
		return connections.begin();
	}

	void CServerENet::release()
	{
		if(state == INIT_AND_CONNECTED)
			disconnectAll();

		enet_host_destroy(server);
		atexit(enet_deinitialize);
		state = NO_INIT;
	}


	void CServerENet::sendData(CConnection* connection, void* data, size_t longData, int channel, bool reliable)
	{
		enet_uint32 rel = 0;
		if(reliable)
			rel = ENET_PACKET_FLAG_RELIABLE;
		
		ENetPacket * packet = enet_packet_create (data, 
												  longData, 
												  rel);
	    
		enet_peer_send (((CConnectionENet*)connection)->getENetPeer(), channel, packet);

		if(DEBUG_SERVER)
			fprintf (stdout, "Packet send ");
		enet_host_flush (server);
	}


	void CServerENet::sendAll(void* data, size_t longData, int channel, bool reliable)
	{
		enet_uint32 rel = 0;
		if(reliable)
			rel = ENET_PACKET_FLAG_RELIABLE;

		ENetPacket * packet = enet_packet_create (data,longData,rel);

    enet_host_broadcast(server, channel, packet);

		if(DEBUG_SERVER)
			fprintf (stdout, "Packet send ");
		enet_host_flush (server);
	}

	void CServerENet::disconnect(CConnection* connection)
	{
		ENetEvent event;
    
		enet_peer_disconnect (((CConnectionENet*)connection)->getENetPeer(),0);

		/* Allow up to 3 seconds for the disconnect to succeed
			and drop any packets received packets.     */
		while (enet_host_service (server, & event, 3000) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_RECEIVE:
				enet_packet_destroy (event.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				if(DEBUG_SERVER)
					fprintf (stdout, "Disconnection succeeded.");
					disconnectReceived(connection);
				return;
			}
		}
	    
		/* We've arrived here, so the disconnect attempt didn't */
		/* succeed yet.  Force the connection down.             */
		enet_peer_reset (((CConnectionENet*)connection)->getENetPeer());
		disconnectReceived(connection);

		if(DEBUG_SERVER)
			fprintf(stdout, "Disconnection Forced");

		if(connections.empty())
			state = INIT_NOT_CONNECTED;
	}


	void CServerENet::disconnectAll()
	{
		if(state == INIT_AND_CONNECTED)
		{
			while(!connections.empty())
			{
				disconnect(connections.back());
			}
		}

		if(DEBUG_SERVER)
			fprintf(stdout, "Everything Disconnected");

		state = INIT_NOT_CONNECTED;
	}


	void CServerENet::disconnectReceived(CConnection* connection)
	{
		std::vector<CConnection*>::iterator it = connections.begin();
		bool found = false;
		while ((it != connections.end()) && (!found)) {
			if(*it == connection)
				found=true;
			else
				++it;
		}
		if(found)
		{
			connections.erase(it);

			if(connections.empty())
				state = INIT_NOT_CONNECTED;
		}
	}

	bool CServerENet::isConnected()
	{
		return (state == INIT_AND_CONNECTED);
	}

	bool CServerENet::isInitialized()
	{
		return (state != NO_INIT);
	}

} // namespace Net
