#pragma once
void
CreateMyCharacterPacket
(CSerialization& buffer, DWORD id, BYTE dir, short x, short y, BYTE hp);

void
CreateOtherCharacterPacket
(CSerialization& buffer, DWORD id, BYTE dir, short x, short y, BYTE hp);

void
CreateDeleteCharacterPacket
(CSerialization& buffer, DWORD id);

void
CreateMoveStartPacket
(CSerialization& buffer, DWORD id, BYTE dir, short x, short y);

void
CreateMoveStopPacket
(CSerialization& buffer, DWORD id, BYTE dir, short x, short y);

void
CreateAttack1Packet
(CSerialization& buffer, DWORD id, BYTE dir, short x, short y);

void
CreateAttack2Packet
(CSerialization& buffer, DWORD id, BYTE dir, short x, short y);

void
CreateAttack3Packet
(CSerialization& buffer, DWORD id, BYTE dir, short x, short y);

void
CreateDamagePacket
(CSerialization& buffer, DWORD attacker_id, DWORD victim_id, BYTE victim_hp);

void
CreateSyncPacket
(CSerialization& buffer, DWORD id, short x, short y);

void
CreateEchoPacket
(CSerialization& buffer, DWORD time);

// ----- job Packet

void
CreateMoveStartJobPacket
(CSerialization& buffer, DWORD s_id, BYTE dir, short x, short y);

void
CreateMoveStopJobPacket
(CSerialization& buffer, DWORD s_id, BYTE dir, short x, short y);

void
CreateAttack1JobPacket
(CSerialization& buffer, DWORD s_id, BYTE dir, short x, short y);

void
CreateAttack2JobPacket
(CSerialization& buffer, DWORD s_id, BYTE dir, short x, short y);

void
CreateAttack3JobPacket
(CSerialization& buffer, DWORD s_id, BYTE dir, short x, short y);

void
CreateEchoJobPacket
(CSerialization& buffer, DWORD s_id, DWORD time);