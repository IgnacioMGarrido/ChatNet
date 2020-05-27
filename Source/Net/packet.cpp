//---------------------------------------------------------------------------
// paquete.cpp
//---------------------------------------------------------------------------

/**
 * @file paquete.cpp
 *
 * Representación de los datos transmitidos por red
 *
 *
 * @author Juan A. Recio-García
 * @date Octumbre, 2006
 */

#include "packet.h"
#include "string.h"

namespace Net {

	CPacket::CPacket( EPacketType tipo, 
						byte* data, 
						size_t dataLength, 
						CConnection* connection, 
						unsigned char channel)
	{
		_data = 0;
		_dataLength = 0;
		_packetType = tipo;
		setData(data,dataLength);
		_connection = connection;
		_channel = channel;	
	}

	CPacket::~CPacket()
	{
		if(_data != 0)
			delete[] _data;
	}

	byte* CPacket::getData()
	{
		return _data;
	}

	void CPacket::setData(byte* data, size_t length)
	{
		if (_data != 0)
			delete[] _data;
	
		if((data == 0)||(length == 0))
			return;

		_data = new byte[length];
		memcpy(_data, data, length);
		_dataLength = length;
	}

	size_t CPacket::getDataLength()
	{
		return _dataLength;
	}


	EPacketType CPacket::getType()
	{
		return _packetType;
	}

	void CPacket::setType(EPacketType type)
	{
		_packetType = type;
	}

	CConnection* CPacket::getConnection()
	{
		return _connection;
	}

	void CPacket::setConnection(CConnection* connection)
	{
		_connection = connection;
	}

	void CPacket::setChannel(unsigned char channel)
	{
		_channel = channel;
	}

	unsigned char CPacket::getChannel()
	{
		return _channel;
	}

} // namespace Net