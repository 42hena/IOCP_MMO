#pragma once
bool	PacketProc_ClientJoin(SESSEION_TYPE& session_id, CSerialization& buffer);
void	PacketProc_MoveStartPacket(SESSEION_TYPE& session_id, CSerialization& buffer);
void	PacketProc_MoveStopPacket(SESSEION_TYPE& session_id, CSerialization& buffer);
void	PacketProc_Attack1Packet(SESSEION_TYPE& session_id, CSerialization& buffer);
void	PacketProc_Attack2Packet(SESSEION_TYPE& session_id, CSerialization& buffer);
void	PacketProc_Attack3Packet(SESSEION_TYPE& session_id, CSerialization& buffer);
void	PacketProc_Echo(SESSEION_TYPE& session_id, CSerialization& buffer);

void	PacketProc_ClientLeave(SESSEION_TYPE& session_id);


void Update();