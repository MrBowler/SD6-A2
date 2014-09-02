#ifndef include_Packet
#define include_Packet
#pragma once

//-----------------------------------------------------------------------------------------------
enum
{
	PACKET_STATE_NONE,
	PACKET_STATE_ADD,
	PACKET_STATE_REMOVE,
	PACKET_STATE_UPDATE,
};


//-----------------------------------------------------------------------------------------------
struct Packet
{
	char	m_packetID;
	char	r;
	char	g;
	char	b;
	int		m_playerID;
	float	x;
	float	y;
};


#endif // include_Packet