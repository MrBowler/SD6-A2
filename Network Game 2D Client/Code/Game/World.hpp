#ifndef include_World
#define include_World
#pragma once

//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>
#include "Packet.hpp"
#include "Player.hpp"
#include "GameCommon.hpp"
#include "../Engine/Clock.hpp"
#include "../Engine/Mouse.hpp"
#include "../Engine/Camera.hpp"
#include "../Engine/Vertex.hpp"
#include "../Engine/pugixml.hpp"
#include "../Engine/Texture.hpp"
#include "../Engine/Vector2.hpp"
#include "../Engine/Keyboard.hpp"
#include "../Engine/Material.hpp"
#include "../Engine/BitmapFont.hpp"
#include "../Engine/DebugGraphics.hpp"
#include "../Engine/OpenGLRenderer.hpp"
#include "../Engine/NamedProperties.hpp"
#include "../Engine/ConsoleCommandArgs.hpp"
#include "../Engine/XMLParsingFunctions.hpp"
#include "../Engine/ErrorWarningAssertions.hpp"
#pragma comment(lib,"ws2_32.lib")


//-----------------------------------------------------------------------------------------------
const float SPEED_PIXELS_PER_SECOND = 250.f;
const float POINT_SIZE_PIXELS = 20.f;
const float ONE_HALF_POINT_SIZE_PIXELS = POINT_SIZE_PIXELS * 0.5f;
const unsigned char PLAYER_COLOR_R = 0;
const unsigned char PLAYER_COLOR_G = 0;
const unsigned char PLAYER_COLOR_B = 0;
const unsigned short PORT_NUMBER = 5000;
//const Vector2 INIT_POSITION = Vector2( 50.f, 50.f );
//const std::string IP_ADDRESS = "129.119.142.83";
const std::string IP_ADDRESS = "127.0.0.1";


//-----------------------------------------------------------------------------------------------
class World
{
public:
	World( float worldWidth, float worldHeight );
	void Initialize();
	void Destruct();
	void SetPlayerColor( unsigned char r, unsigned char g, unsigned char b );
	void ChangeIPAddress( const std::string& ipAddrString );
	void ChangePortNumber( unsigned short portNumber );
	void Update( float deltaSeconds, const Keyboard& keyboard, const Mouse& mouse );
	void RenderObjects3D();
	void RenderObjects2D();

private:
	void InitializeConnection();
	void UpdateFromInput( const Keyboard& keyboard, const Mouse& mouse );
	void SendPackets();
	void ReceivePackets();
	void InterpolatePositions( float deltaSeconds );

	Camera					m_camera;
	Vector2					m_size;
	WSADATA					m_wsaData;
	SOCKET					m_socket;
	struct sockaddr_in		m_serverAddr;
	bool					m_isConnectedToServer;
	Player*					m_mainPlayer;
	std::vector< Player* >	m_players;
};


#endif // include_World