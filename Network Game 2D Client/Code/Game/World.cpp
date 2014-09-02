#include "World.hpp"
#include "../Engine/DeveloperConsole.hpp"
#include "../Engine/NewMacroDef.hpp"

//-----------------------------------------------------------------------------------------------
World::World( float worldWidth, float worldHeight )
	: m_size( worldWidth, worldHeight )
	, m_isConnectedToServer( false )
{
	m_mainPlayer = new Player();
	m_mainPlayer->m_r = PLAYER_COLOR_R;
	m_mainPlayer->m_g = PLAYER_COLOR_G;
	m_mainPlayer->m_b = PLAYER_COLOR_B;
	m_mainPlayer->m_currentPosition = Vector2( m_size.x * 0.5f, m_size.y * 0.5f );
	m_mainPlayer->m_gotoPosition = Vector2( m_size.x * 0.5f, m_size.y * 0.5f );
	m_players.push_back( m_mainPlayer );
}


//-----------------------------------------------------------------------------------------------
void World::Initialize()
{
	InitializeConnection();
}


//-----------------------------------------------------------------------------------------------
void World::Destruct()
{
	closesocket( m_socket );
	WSACleanup();
}


//-----------------------------------------------------------------------------------------------
void World::SetPlayerColor( unsigned char r, unsigned char g, unsigned char b )
{
	m_mainPlayer->m_r = r;
	m_mainPlayer->m_g = g;
	m_mainPlayer->m_b = b;
}


//-----------------------------------------------------------------------------------------------
void World::ChangeIPAddress( const std::string& ipAddrString )
{
	m_serverAddr.sin_addr.s_addr = inet_addr( ipAddrString.c_str() );
	m_isConnectedToServer = false;
	m_players.clear();
	m_players.push_back( m_mainPlayer );
}


//-----------------------------------------------------------------------------------------------
void World::ChangePortNumber( unsigned short portNumber )
{
	m_serverAddr.sin_port = htons( portNumber );
	m_isConnectedToServer = false;
	m_players.clear();
	m_players.push_back( m_mainPlayer );
}


//-----------------------------------------------------------------------------------------------
void World::Update( float deltaSeconds, const Keyboard& keyboard, const Mouse& mouse )
{
	UpdateFromInput( keyboard, mouse );
	SendPackets();
	ReceivePackets();
	InterpolatePositions( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void World::RenderObjects3D()
{
	
}


//-----------------------------------------------------------------------------------------------
void World::RenderObjects2D()
{
	for( unsigned int playerIndex = 0; playerIndex < m_players.size(); ++playerIndex )
	{
		Player* player = m_players[ playerIndex ];
		if( player == nullptr )
			continue;

		Vector2 playerPos = player->m_currentPosition;
		OpenGLRenderer::SetColor3f( player->m_r * ONE_OVER_TWO_HUNDRED_TWENTY_FIVE, player->m_g * ONE_OVER_TWO_HUNDRED_TWENTY_FIVE, player->m_b * ONE_OVER_TWO_HUNDRED_TWENTY_FIVE );
		OpenGLRenderer::BeginRender( QUADS );
		{
			OpenGLRenderer::SetVertex2f( playerPos.x - ONE_HALF_POINT_SIZE_PIXELS, playerPos.y - ONE_HALF_POINT_SIZE_PIXELS );
			OpenGLRenderer::SetVertex2f( playerPos.x + ONE_HALF_POINT_SIZE_PIXELS, playerPos.y - ONE_HALF_POINT_SIZE_PIXELS );
			OpenGLRenderer::SetVertex2f( playerPos.x + ONE_HALF_POINT_SIZE_PIXELS, playerPos.y + ONE_HALF_POINT_SIZE_PIXELS );
			OpenGLRenderer::SetVertex2f( playerPos.x - ONE_HALF_POINT_SIZE_PIXELS, playerPos.y + ONE_HALF_POINT_SIZE_PIXELS );
		}
		OpenGLRenderer::EndRender();
	}
}


//-----------------------------------------------------------------------------------------------
void World::InitializeConnection()
{
	if( WSAStartup( 0x202, &m_wsaData ) != 0 )
	{
		g_isQuitting = true;
		return;
	}

	m_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( m_socket == INVALID_SOCKET )
	{
		g_isQuitting = true;
		return;
	}

	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_addr.s_addr = inet_addr( IP_ADDRESS.c_str() );
	m_serverAddr.sin_port = htons( PORT_NUMBER );

	u_long mode = 1;
	if( ioctlsocket( m_socket, FIONBIO, &mode ) == SOCKET_ERROR )
	{
		g_isQuitting = true;
		return;
	}

	/*if( bind( m_socket, (struct sockaddr *) &m_serverAddr, sizeof( m_serverAddr ) ) < 0 )
	{
		g_isQuitting = true;
		return;
	}*/
}


//-----------------------------------------------------------------------------------------------
void World::UpdateFromInput( const Keyboard& keyboard, const Mouse& )
{
	if( g_developerConsole.m_drawConsole )
		return;

	if( keyboard.IsKeyPressedDown( KEY_A ) )
	{
		m_mainPlayer->m_currentPosition.x -= 1.f;
	}
	else if( keyboard.IsKeyPressedDown( KEY_D ) )
	{
		m_mainPlayer->m_currentPosition.x += 1.f;
	}

	if( keyboard.IsKeyPressedDown( KEY_S ) )
	{
		m_mainPlayer->m_currentPosition.y -= 1.f;
	}
	else if( keyboard.IsKeyPressedDown( KEY_W ) )
	{
		m_mainPlayer->m_currentPosition.y += 1.f;
	}
}


//-----------------------------------------------------------------------------------------------
void World::SendPackets()
{
	if( !m_isConnectedToServer )
	{
		Packet addPkt;
		addPkt.m_packetID = PACKET_STATE_ADD;
		sendto( m_socket, (char*) &addPkt, sizeof( addPkt ), 0, (struct sockaddr*) &m_serverAddr, sizeof( m_serverAddr ) );
		return;
	}

	Packet pkt;
	pkt.m_packetID = PACKET_STATE_UPDATE;
	pkt.r = m_mainPlayer->m_r;
	pkt.g = m_mainPlayer->m_g;
	pkt.b = m_mainPlayer->m_b;
	pkt.m_playerID = m_mainPlayer->m_id;
	pkt.x = m_mainPlayer->m_currentPosition.x;
	pkt.y = m_mainPlayer->m_currentPosition.y;

	sendto( m_socket, (char*) &pkt, sizeof( pkt ), 0, (struct sockaddr*) &m_serverAddr, sizeof( m_serverAddr ) );
}


//-----------------------------------------------------------------------------------------------
void World::ReceivePackets()
{
	Packet pkt;
	sockaddr_in clientAddr;
	int clientLen = sizeof( clientAddr );

	int loopCount = 0;
	while( recvfrom( m_socket, (char*) &pkt, sizeof( pkt ), 0, (struct sockaddr*) &clientAddr, &clientLen ) > 0 )
	{
		++loopCount;
		if( pkt.m_packetID == PACKET_STATE_ADD )
		{
			m_mainPlayer->m_id = (unsigned char) pkt.m_playerID;
			m_isConnectedToServer = true;
		}
		else if( pkt.m_packetID == PACKET_STATE_UPDATE )
		{
			if( pkt.m_playerID == m_mainPlayer->m_id )
				return;

			for( unsigned int playerIndex = 0; playerIndex < m_players.size(); ++playerIndex )
			{
				Player* player = m_players[ playerIndex ];
				if( pkt.m_playerID == player->m_id )
				{
					player->m_r = pkt.r;
					player->m_g = pkt.g;
					player->m_b = pkt.b;
					player->m_gotoPosition = Vector2( pkt.x, pkt.y );
					return;
				}
			}

			Player* player = new Player();
			player->m_id = (unsigned char) pkt.m_playerID;
			player->m_r = pkt.r;
			player->m_g = pkt.g;
			player->m_b = pkt.b;
			player->m_currentPosition = Vector2( pkt.x, pkt.y );
			player->m_gotoPosition = Vector2( pkt.x, pkt.y );
			m_players.push_back( player );
		}
		else if( pkt.m_packetID == PACKET_STATE_REMOVE )
		{
			for( unsigned int playerIndex = 0; playerIndex < m_players.size(); ++playerIndex )
			{
				Player* player = m_players[ playerIndex ];
				if( player != nullptr && player->m_id == pkt.m_playerID )
				{
					m_players.erase( m_players.begin() + playerIndex );
					break;
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void World::InterpolatePositions( float deltaSeconds )
{
	for( unsigned int playerIndex = 0; playerIndex < m_players.size(); ++playerIndex )
	{
		Player* player = m_players[ playerIndex ];
		if( player == nullptr || player == m_mainPlayer )
			continue;

		player->m_currentPosition += deltaSeconds * ( player->m_gotoPosition - player->m_currentPosition );
	}
}