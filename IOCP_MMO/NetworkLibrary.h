#pragma once

bool AcceptRecvPost(st_session& now_session);
void SendPost(st_session& now_session);
void SendPacket(SESSEION_TYPE& session_id, CSerialization& packet);
void OnMessage(SESSEION_TYPE& session_id, CSerialization& packet);
void OnClientJoin(SESSEION_TYPE& session_id, st_session& now_session);
void RecvPost(st_session& now_session);
bool ReleaseSession(st_session& now_session);
void OnRelease(SESSEION_TYPE& session_id);
bool Disconnect(SESSEION_TYPE& session_id);