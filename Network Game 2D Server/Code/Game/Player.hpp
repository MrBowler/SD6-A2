#ifndef include_Player
#define include_Player
#pragma once

//-----------------------------------------------------------------------------------------------
#include "../Engine/Vector2.hpp"


//-----------------------------------------------------------------------------------------------
struct Player
{
	unsigned char	m_id;
	unsigned char	m_r;
	unsigned char	m_g;
	unsigned char	m_b;
	unsigned short	m_portNumber;
	char*			m_ipAddress;
	Vector2			m_position;
	double			m_lastTime;
};



#endif // include_Player