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
	Vector2			m_currentPosition;
	Vector2			m_gotoPosition;
};



#endif // include_Player