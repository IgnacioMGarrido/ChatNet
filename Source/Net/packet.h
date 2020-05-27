//---------------------------------------------------------------------------
// paquete.h
//---------------------------------------------------------------------------

/**
 * @file paquete.h
 *
 * Representación de los datos transmitidos por red
 *
 *
 * @author Juan A. Recio-García
 * @date Octumbre, 2006
 */

#ifndef __PAQUETE_H
#define __PAQUETE_H

#include "connection.h"

namespace Net {

	/**
	 * Los paquetes manejarán bytes
	 */
	typedef unsigned char byte;

	/**
	 * Identificador de paquetes
	 */
    enum EPacketType{
		DATA,
		CONNECTION,
		DISCONNECTION,
		CONTROL
	};

/**
 * Clase que representa de los datos transmitidos por red
 */
class CPacket {

public:

	/**
	 * Constructora. Crea un paquete de datos.
	 * @param tipo del paquete
	 * @param data es el contenido del paquete
	 * @param dataLength es el tamaño de los datos (data)
	 * @param conexion es la conexión por la que se transmiten
	 * @param canan es el canal por el que se transmiten
	 */
	CPacket(EPacketType tipo, byte* data, size_t dataLength, CConnection* conexion, unsigned char canal);

	/**
	 * Destructura
	 */
	~CPacket();

	/**
	 * Obtener los datos
	 */
	byte* getData();

	/**
	 * Establecer el contenido del paquete
	 */
	void setData(byte* data, size_t length);

	/**
	 * Obtener el tamaño de los datos del paquete
	 */ 
	size_t getDataLength();

	/**
	 * Obtener el tipo de los datos
	 */
	EPacketType getType();

	/**
	 * Establecer el tipo de los datos
	 */
	void setType(EPacketType type);

	/**
	 * Establecer la conexión que transmite los datos
	 */ 
	void setConnection(CConnection* connection);

	/**
	 * Obtener la conexión que transmite los datos
	 */ 
	CConnection* getConnection();

	/**
	 * Establecer el canal que transmite los datos
	 */ 
	void setChannel(unsigned char channel);

	/**
	 * Obtener el canal que transmite los datos
	 */ 
	unsigned char getChannel();

protected:
	unsigned char _channel;
	size_t _dataLength;
	byte* _data;
	EPacketType _packetType;
	CConnection* _connection;

};


} //namespace Net

#endif