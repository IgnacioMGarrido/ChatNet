//---------------------------------------------------------------------------
// factoriaredenet.h
//---------------------------------------------------------------------------

/**
 * @file factoriaredenet.h
 *
 * Especifica una factoria de objetos de red implementados con ENet
 *
 *
 * @author Juan A. Recio-García
 * @date Octumbre, 2006
 */

#ifndef __FACTORIAREDENET_H
#define __FACTORIAREDENET_H

#include "serverenet.h"
#include "clientENet.h"
#include "factory.h"

namespace Net {

/**
 * Factoría para obtener el servidor y cliente de red implementados con la librería ENet
 */
class CFactoryEnet: public CFactory {

public:

	CFactoryEnet() {};

	CServer* buildServer();


	CClient*  buildClient();

};



CServer* CFactoryEnet::buildServer()
{
	return new CServerENet();
}

CClient* CFactoryEnet::buildClient()
{
	return new CClientENet();
}


} //namespace Net

#endif