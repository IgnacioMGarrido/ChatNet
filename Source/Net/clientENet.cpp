//---------------------------------------------------------------------------
// clienteENet.cpp
//---------------------------------------------------------------------------

/**
 * @file clienteENet.cpp
 *
 * Implemente un cliente de red con ENet
 *
 * @author  Juan A. Recio García
 * @date Junio, 2006
 */
#include "clientENet.h"
#include "connectionenet.h"
#include <iostream>

#define DEBUG_CLIENT 0

namespace Net {

	CClientENet::CClientENet()
	{
		state = NO_INIT;
	}


	CClientENet::~CClientENet()
	{
		std::vector<CConnection*>::iterator it = connections.begin();
		while (it != connections.end()) {
			delete (*it);
			++it;
		}
		connections.clear();
	}

	bool CClientENet::init(unsigned int maxConnections, unsigned int maxinbw, unsigned int maxoutbw)
	{
		if(state != NO_INIT)
			return false;

		if (enet_initialize () != 0)
		{
			printf ("An error occurred while initializing ENet.\n");
			return false;

		}

		client = enet_host_create (NULL /* create a client host */,
                maxConnections, /* only allow 1 outgoing connection */
                maxinbw ,
                maxoutbw );

		if (client == NULL)
		{
			printf ("An error occurred while trying to create an ENet client host.\n");
			return false;
		}
		if(DEBUG_CLIENT)
			fprintf(stdout, "Client initialized");
	
		state = INIT_NOT_CONNECTED;

		return true;
	}

	CConnection* CClientENet::connect(char* address, int port, int channels, unsigned int timeout)
	{
		if(state == NO_INIT)
			return NULL;

		ENetAddress _address;
		ENetEvent event;
		CConnectionENet* connection = 0;

		/* Connect */
		enet_address_set_host (& _address, address);
		_address.port = port;

		ENetPeer* peer = enet_host_connect (client, & _address, channels);    
	    
		if (peer == NULL)
		{
			printf ("No available peers for initiating an ENet connection.\n");
		}
		else
		{
			/* Wait up to 5 seconds for the connection attempt to succeed.*/
			if (enet_host_service (client, & event, timeout) > 0 &&
				event.type == ENET_EVENT_TYPE_CONNECT)
			{		
				if(DEBUG_CLIENT)
					fprintf(stdout, "Connection succeeded.");
				connection = new CConnectionENet();
				connection -> setENetPeer(peer);
				connections.push_back(connection);
			}
			else
			{
				/* Either the 5 seconds are up or a disconnect event was */
				/* received. Reset the peer in the event the 5 seconds   */
				/* had run out without any significant event.            */
				enet_peer_reset (peer);

				printf ("Connection  failed.");
			}
		}

	    enet_host_flush (client);
		state = INIT_AND_CONNECTED;

		return connection;
	}

	std::vector<CConnection*>::iterator CClientENet::getConnectionsIt()
	{
		return connections.begin();
	}

	void CClientENet::sendData(CConnection* connection, void* data, int longData, int channel, bool reliable)
	{
		enet_uint32 rel = 0;
		if(reliable)
			rel = ENET_PACKET_FLAG_RELIABLE;


		
		ENetPacket * packet = enet_packet_create (data, 
												  longData,
												  rel);
	    
		enet_peer_send (((CConnectionENet*)connection)->getENetPeer(), channel, packet);

		if(DEBUG_CLIENT)
			fprintf (stdout, "Packet send ");
		enet_host_flush (client);
  }


  void CClientENet::sendAll(void* data, size_t longData, int channel, bool reliable)
  {
    enet_uint32 rel = 0;
    if (reliable)
      rel = ENET_PACKET_FLAG_RELIABLE;

    ENetPacket * packet = enet_packet_create(data, longData, rel);

    enet_host_broadcast(client, channel, packet);

    if (DEBUG_CLIENT)
      fprintf(stdout, "Packet send ");
    enet_host_flush(client);
  }


	void CClientENet::service(std::vector<CPacket*>&  packets)
	{
		ENetEvent event;
		CPacket* packet;
    
		if(!isConnected())
			return;

		/* Wait up to 0 milliseconds for an event. */
		while (enet_host_service (client, & event, 0) > 0)
		{
			switch (event.type)
			{
				case ENET_EVENT_TYPE_RECEIVE:
					if(DEBUG_CLIENT)
						printf ("A packet of length %u was received from %s on channel %u.\n",
							(unsigned int)event.packet -> dataLength,
							(char*) event.peer -> data,
							event.channelID);
					

					packet = new CPacket(DATA, event.packet->data, event.packet->dataLength, (CConnection*)event.peer->data, event.channelID);
					packets.push_back(packet);

					enet_packet_destroy (event.packet);            
					break;
				case ENET_EVENT_TYPE_DISCONNECT:
					if(DEBUG_CLIENT)
						fprintf(stdout,"Server disconected.\n");
					packet = new CPacket(DISCONNECTION,0,0,(CConnection*)event.peer->data,0);
					packets.push_back(packet);
					disconnectReceived((CConnection*)event.peer->data);
					break;		           
				default:
					puts("Unknown packet");
			}
		}
	}

	CPacket* CClientENet::readPacket()
	{
		ENetEvent event;
		CPacket *packet = NULL;
		enet_host_service (client, & event, 0);		
		switch (event.type)	{
				case ENET_EVENT_TYPE_RECEIVE:
					packet = new CPacket(DATA, event.packet->data, event.packet->dataLength, (CConnection*)event.peer->data, event.channelID);
					enet_packet_destroy (event.packet);            
					break;		           
				case ENET_EVENT_TYPE_DISCONNECT:
					if(DEBUG_CLIENT)
						fprintf(stdout,"Server disconnected.\n");
					packet = new CPacket(DISCONNECTION,0,0,(CConnection*)event.peer->data,0);
					disconnectReceived((CConnection*)event.peer->data);
					break;		           
			}	
		return packet;
	}

	void CClientENet::disconnect(CConnection* connection)
	{
		ENetEvent event;
    
		enet_peer_disconnect (((CConnectionENet*)connection)->getENetPeer(),0);

		/* Allow up to 3 seconds for the disconnect to succeed
			and drop any packets received packets.     */
		while (enet_host_service (client, & event, 3000) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_RECEIVE:
				enet_packet_destroy (event.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				if(DEBUG_CLIENT)
					fprintf (stdout, "Disconnection succeeded.");
					disconnectReceived(connection);
				return;
			}
		}
	    
		/* We've arrived here, so the disconnect attempt didn't */
		/* succeed yet.  Force the connection down.             */
		enet_peer_reset (((CConnectionENet*)connection)->getENetPeer());
		disconnectReceived(connection);

		if(DEBUG_CLIENT)
			fprintf(stdout, "Disconnection Forced");

		if(connections.empty())
			state = INIT_NOT_CONNECTED;
	}

	void CClientENet::disconnectAll()
	{
		if(state == INIT_AND_CONNECTED)
		{
			while(!connections.empty())
			{
				disconnect(connections.back());
			}
		}

		if(DEBUG_CLIENT)
			fprintf(stdout, "Everything Disconnected");

		state = INIT_NOT_CONNECTED;
	}

	void CClientENet::release()
	{
		if(state == INIT_AND_CONNECTED)
			disconnectAll();

		enet_host_destroy(client);
		atexit (enet_deinitialize);
		state = NO_INIT;
	}

	bool CClientENet::isConnected()
	{
		return (state == INIT_AND_CONNECTED);
	}

	bool CClientENet::isInitialized()
	{
		return (state != NO_INIT);
	}

	void CClientENet::disconnectReceived(CConnection* conexion)
	{
		std::vector<CConnection*>::iterator it = connections.begin();
		bool found = false;
		while ((it != connections.end()) && (!found)) {
			if(*it == conexion)
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

} // namespace Net
