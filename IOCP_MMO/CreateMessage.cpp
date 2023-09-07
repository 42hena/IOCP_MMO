#include <Windows.h>

#include "CSerializationBuffer.h"
#include "PACKET_PROTOCOL.h"

void
CreateMyCharacterPacket
(CSerialization& buffer, DWORD id, BYTE dir, short x, short y, BYTE hp)
{
	short type = dfPACKET_SC_CREATE_MY_CHARACTER;

	// Contents HeaderInfo
	buffer << type;

	// Payload Info
	buffer << id << dir << x << y << hp;
}

void
CreateOtherCharacterPacket
(CSerialization& buffer, DWORD id, BYTE dir, short x, short y, BYTE hp)
{
	short type = dfPACKET_SC_CREATE_OTHER_CHARACTER;

	// Contents HeaderInfo
	buffer << type;

	// Payload Info
	buffer << id << dir << x << y << hp;
}

void
CreateDeleteCharacterPacket
(CSerialization& buffer, DWORD id)
{
	short type = dfPACKET_SC_DELETE_CHARACTER;

	// Contents HeaderInfo
	buffer << type;

	// Payload Info
	buffer << id;
}

void
CreateMoveStartPacket
(CSerialization& buffer, DWORD id, BYTE dir, short x, short y)
{
	short type = dfPACKET_SC_MOVE_START;
	// HeaderInfo
	buffer << type;

	// Payload Info
	buffer << id << dir << x << y;
}

void
CreateMoveStopPacket
(CSerialization& buffer, DWORD id, BYTE dir, short x, short y)
{
	short type = dfPACKET_SC_MOVE_STOP;
	
	// HeaderInfo
	buffer << type;

	// Payload Info
	buffer << id << dir << x << y;
}

void
CreateAttack1Packet
(CSerialization& buffer, DWORD id, BYTE dir, short x, short y)
{
	short type = dfPACKET_SC_ATTACK1;

	// HeaderInfo
	buffer << type;

	// Payload Info
	buffer << id << dir << x << y;
}

void
CreateAttack2Packet
(CSerialization& buffer, DWORD id, BYTE dir, short x, short y)
{
	short type = dfPACKET_SC_ATTACK2;

	// HeaderInfo
	buffer << type;

	// Payload Info
	buffer << id << dir << x << y;
}

void
CreateAttack3Packet
(CSerialization& buffer, DWORD id, BYTE dir, short x, short y)
{
	short type = dfPACKET_SC_ATTACK3;

	// HeaderInfo
	buffer << type;

	// Payload Info
	buffer << id << dir << x << y;
}

void
CreateDamagePacket
(CSerialization& buffer, DWORD attacker_id, DWORD victim_id, BYTE victim_hp)
{
	short type = dfPACKET_SC_DAMAGE;

	// HeaderInfo
	buffer << type;

	// Payload Info
	buffer << attacker_id << victim_id << victim_hp;
}

void
CreateSyncPacket
(CSerialization& buffer, DWORD id, short x, short y)
{
	short type = dfPACKET_SC_SYNC;

	// HeaderInfo
	buffer << type;

	// Payload Info
	buffer << id << x << y;
}

void
CreateEchoPacket
(CSerialization& buffer, DWORD time)
{
	short type = dfPACKET_SC_ECHO;

	// HeaderInfo
	buffer << type;

	// Payload Info
	buffer << time;
}

//------------------------------------------------------------------

void
CreateMoveStartJobPacket
(CSerialization& buffer, DWORD s_id, BYTE dir, short x, short y)
{
	short type = dfPACKET_CS_MOVE_START;

	// session id
	buffer << s_id;

	// HeaderInfo
	buffer << type;

	// Payload Info
	buffer << dir << x << y;
}

void
CreateMoveStopJobPacket
(CSerialization& buffer, DWORD s_id, BYTE dir, short x, short y)
{
	short type = dfPACKET_CS_MOVE_STOP;

	// session id
	buffer << s_id;

	// HeaderInfo
	buffer << type;

	// Payload Info
	buffer << dir << x << y;
}

void
CreateAttack1JobPacket
(CSerialization& buffer, DWORD s_id, BYTE dir, short x, short y)
{
	short type = dfPACKET_CS_ATTACK1;

	// session id
	buffer << s_id;

	// HeaderInfo
	buffer << type;

	// Payload Info
	buffer << dir << x << y;
}

void
CreateAttack2JobPacket
(CSerialization& buffer, DWORD s_id, BYTE dir, short x, short y)
{
	short type = dfPACKET_CS_ATTACK2;

	// session id
	buffer << s_id;

	// HeaderInfo
	buffer << type;

	// Payload Info
	buffer << dir << x << y;
}

void
CreateAttack3JobPacket
(CSerialization& buffer, DWORD s_id, BYTE dir, short x, short y)
{
	short type = dfPACKET_CS_ATTACK3;

	// session id
	buffer << s_id;

	// HeaderInfo
	buffer << type;

	// Payload Info
	buffer << dir << x << y;
}

void
CreateEchoJobPacket
(CSerialization& buffer, DWORD s_id, DWORD time)
{
	short type = dfPACKET_CS_ECHO;

	// session id
	buffer << s_id;

	// HeaderInfo
	buffer << type;

	// Payload Info
	buffer << time;
}