//---------------------------------------------------------------------------
// factoriared.h
//---------------------------------------------------------------------------

/**
 * @file factoriared.h
 *
 * Especifica una factoria (abstracta) de objetos de red
 *
 *
 * @author Juan A. Recio-García
 * @date Octumbre, 2006
 */

#ifndef __FACTORIARED_H
#define __FACTORIARED_H

#include "server.h"
#include "client.h"

namespace Net {

/**
 * Factoría para obtener el servidor y cliente de red
 */
class CFactory {

public:
	/**
	 * Obtiene un servidor
	 */
	virtual CServer* buildServer()=0;

	/**
	 * Obtiene un cliente
	 */
	virtual CClient*  buildClient()=0;
};


} //namespace Net

#endif