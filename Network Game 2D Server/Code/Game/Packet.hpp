#ifndef include_Packet
#define include_Packet
#pragma once

//-----------------------------------------------------------------------------------------------
#include "Player.hpp"


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
	Packet() : m_packetID( PACKET_STATE_NONE ), r( 0 ), g( 0 ), b( 0 ), m_playerID( 0 ), x( 0.f ), y( 0.f ) {}
	Packet( const Player* p ) : m_packetID( PACKET_STATE_NONE ), r( p->m_r ), g( p->m_g ), b( p->m_b ), m_playerID( p->m_id ), x( p->m_position.x ), y( p->m_position.y ) {}

	unsigned char	m_packetID;
	unsigned char	r;
	unsigned char	g;
	unsigned char	b;
	int				m_playerID;
	float			x;
	float			y;
};


#endif // include_Packet