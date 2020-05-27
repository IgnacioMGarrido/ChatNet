//---------------------------------------------------------------------------
// conexionENet.h
//---------------------------------------------------------------------------

/**
 * @file conexionENet.h
 *
 * Representa una conexión con un peer implementada con ENet.
 *
 *
 * @author Juan A. Recio-García
 * @date Octubre, 2006
 */


#include "connectionenet.h"

namespace Net {

	void CConnectionENet::setENetPeer(ENetPeer* p)
	{
		_peer = p;
	};

	ENetPeer* CConnectionENet::getENetPeer()
	{
		return _peer;
	};

	void CConnectionENet::setId(NetID id)
	{
		_id = id;
	}

	NetID CConnectionENet::getId()
	{
		return _id;
	}

	int CConnectionENet::getAddress()
	{
		return _peer->address.host;
	}

	short CConnectionENet::getPort()
	{
		return _peer->address.port;
	}

} //namespace