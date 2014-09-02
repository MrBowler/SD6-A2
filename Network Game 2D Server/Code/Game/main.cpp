#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <WinSock2.h>
#include "Packet.hpp"
#include "Player.hpp"
#include "ClientInfo.hpp"
#include "../Engine/Time.hpp"
#pragma comment(lib,"ws2_32.lib")

//-----------------------------------------------------------------------------------------------
const unsigned short PORT_NUMBER = 5000;


//-----------------------------------------------------------------------------------------------
bool g_isQuitting = false;
unsigned char g_nextPlayerID = 0;
WSADATA g_wsaData;
SOCKET	g_socket;
double g_secondSinceLastSend;
struct sockaddr_in g_serverAddr;
struct sockaddr_in g_clientAddr;
int g_clientLen = sizeof( g_clientAddr );
std::map< ClientInfo, unsigned char > g_joinRequestIDs;
std::map< ClientInfo, Player* > g_players;


//-----------------------------------------------------------------------------------------------
void PrintError( const char* errorMessage, bool quitProgram )
{
	std::cout << errorMessage << "\n";
	g_isQuitting &= quitProgram;
	system( "PAUSE" );
}


//-----------------------------------------------------------------------------------------------
std::string ConvertNumberToString( int number )
{
	return static_cast< std::ostringstream* >( &( std::ostringstream() << number ) )->str();
}


//-----------------------------------------------------------------------------------------------
void InitializeServer()
{
	if( WSAStartup( 0x202, &g_wsaData ) != 0 )
	{
		PrintError( "Winsock failed to initialize", true );
		return;
	}

	g_socket = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( g_socket == INVALID_SOCKET )
	{
		PrintError( "Socket creation failed", true );
		return;
	}

	g_serverAddr.sin_family = AF_INET;
	g_serverAddr.sin_addr.s_addr = htonl( INADDR_ANY );
	g_serverAddr.sin_port = htons( PORT_NUMBER );

	g_clientAddr.sin_family = AF_INET;

	u_long mode = 1;
	if( ioctlsocket( g_socket, FIONBIO, &mode ) == SOCKET_ERROR )
	{
		PrintError( "Failed to set socket to non-blocking mode", true );
		return;
	}

	if( bind( g_socket, (struct sockaddr *) &g_serverAddr, sizeof( g_serverAddr ) ) < 0 )
	{
		PrintError( "Failed to bind socket", true );
		return;
	}
}


//-----------------------------------------------------------------------------------------------
void SendPlayerID()
{
	std::map< ClientInfo, unsigned char >::iterator idIter;
	for( idIter = g_joinRequestIDs.begin(); idIter != g_joinRequestIDs.end(); ++idIter )
	{
		Packet pkt;
		pkt.m_packetID = PACKET_STATE_ADD;
		pkt.m_playerID = idIter->second;

		g_clientAddr.sin_addr.s_addr = inet_addr( idIter->first.m_ipAddress );
		g_clientAddr.sin_port = idIter->first.m_portNumber;
		sendto( g_socket, (const char*) &pkt, sizeof( pkt ), 0, (struct sockaddr*) &g_clientAddr, g_clientLen );
	}
}


//-----------------------------------------------------------------------------------------------
void AddPlayer()
{
	ClientInfo info;
	info.m_ipAddress = inet_ntoa( g_clientAddr.sin_addr );
	info.m_portNumber = g_clientAddr.sin_port;
	
	std::map< ClientInfo, unsigned char >::iterator requestIter = g_joinRequestIDs.find( info );
	std::map< ClientInfo, Player* >::iterator playerIter = g_players.find( info );
	if( requestIter == g_joinRequestIDs.end() && playerIter == g_players.end() )
	{
		g_joinRequestIDs[ info ] = g_nextPlayerID;
		++g_nextPlayerID;
	}
}


//-----------------------------------------------------------------------------------------------
void RemovePlayer( Player* timeOutPlayer )
{
	Packet pkt;
	pkt.m_packetID = PACKET_STATE_REMOVE;
	pkt.m_playerID = timeOutPlayer->m_id;

	std::map< ClientInfo, Player* >::iterator addrIter;
	for( addrIter = g_players.begin(); addrIter != g_players.end(); ++addrIter )
	{
		g_clientAddr.sin_addr.s_addr = inet_addr( addrIter->second->m_ipAddress );
		g_clientAddr.sin_port = addrIter->second->m_portNumber;
		sendto( g_socket, (const char*) &pkt, sizeof( pkt ), 0, (struct sockaddr*) &g_clientAddr, g_clientLen );
	}

	std::cout << "Player " << timeOutPlayer->m_ipAddress << ":" << ConvertNumberToString( timeOutPlayer->m_portNumber ) << " has timed out and was removed\n";
}


//-----------------------------------------------------------------------------------------------
void UpdatePlayer( const Packet& pkt )
{
	ClientInfo info;
	info.m_ipAddress = inet_ntoa( g_clientAddr.sin_addr );
	info.m_portNumber = g_clientAddr.sin_port;
	Player* player = nullptr;

	std::map< ClientInfo, Player* >::iterator playerIter;
	playerIter = g_players.find( info );
	if( playerIter != g_players.end() )
	{
		player = playerIter->second;
	}
	else
	{
		std::map< ClientInfo, unsigned char >::iterator requestIter = g_joinRequestIDs.find( info );
		if( requestIter != g_joinRequestIDs.end() )
		{
			player = new Player();
			player->m_id = requestIter->second;
			player->m_ipAddress = info.m_ipAddress;
			player->m_portNumber = info.m_portNumber;
			g_joinRequestIDs.erase( requestIter );
			g_players[ info ] = player;

			std::cout << "Added player from " << player->m_ipAddress << ":" << ConvertNumberToString( player->m_portNumber ) << "\n";
		}
	}

	if( player == nullptr )
		return;

	player->m_r = pkt.r;
	player->m_g = pkt.g;
	player->m_b = pkt.b;
	player->m_position.x = pkt.x;
	player->m_position.y = pkt.y;
	player->m_lastTime = GetCurrentTimeSeconds();
}


//-----------------------------------------------------------------------------------------------
void GetPackets()
{
	Packet pkt;
	while( recvfrom( g_socket, (char*) &pkt, sizeof( pkt ), 0, (struct sockaddr*) &g_clientAddr, &g_clientLen ) > 0 )
	{
		if( pkt.m_packetID == PACKET_STATE_ADD )
		{
			AddPlayer();
		}
		else if( pkt.m_packetID == PACKET_STATE_UPDATE )
		{
			UpdatePlayer( pkt );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void RemoveTimedOutPlayers()
{
	std::map< ClientInfo, Player* > tempPlayerMap;
	std::map< ClientInfo, Player* >::iterator g_playerIter;
	for( g_playerIter = g_players.begin(); g_playerIter != g_players.end(); ++g_playerIter )
	{
		Player* player = g_playerIter->second;
		double timeSinceLastActivity = GetCurrentTimeSeconds() - player->m_lastTime;
		if( timeSinceLastActivity >= 5.f )
		{
			RemovePlayer( player );
			continue;
		}

		tempPlayerMap[ g_playerIter->first ] = g_playerIter->second;
	}

	g_players = tempPlayerMap;
}


//-----------------------------------------------------------------------------------------------
void SendUpdatesToClients()
{
	std::map< ClientInfo, Player* >::iterator addrIter;
	for( addrIter = g_players.begin(); addrIter != g_players.end(); ++addrIter )
	{
		g_clientAddr.sin_addr.s_addr = inet_addr( addrIter->second->m_ipAddress );
		g_clientAddr.sin_port = addrIter->second->m_portNumber;

		std::map< ClientInfo, Player* >::iterator playerIter;
		for( playerIter = g_players.begin(); playerIter != g_players.end(); ++playerIter )
		{
			Player* player = playerIter->second;
			Packet pkt( player );
			pkt.m_packetID = PACKET_STATE_UPDATE;
			sendto( g_socket, (char*) &pkt, sizeof( pkt ), 0, (struct sockaddr*) &g_clientAddr, g_clientLen );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Initialize()
{
	InitializeTime();
	InitializeServer();
	g_secondSinceLastSend = GetCurrentTimeSeconds();

	std::cout << "Server is up and running\n";
}


//-----------------------------------------------------------------------------------------------
void Update()
{
	GetPackets();
	RemoveTimedOutPlayers();
	if( GetCurrentTimeSeconds() - g_secondSinceLastSend > 0.1 )
	{
		SendPlayerID();
		SendUpdatesToClients();
		g_secondSinceLastSend = GetCurrentTimeSeconds();
	}
	
}


//-----------------------------------------------------------------------------------------------
int main( int, char* )
{
	Initialize();

	while( !g_isQuitting )
	{
		Update();
	}

	closesocket( g_socket );
	WSACleanup();
	return 0;
}