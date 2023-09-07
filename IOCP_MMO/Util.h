#pragma once
void SectorProc_AddCharacter(st_Character* character);
void SectorProc_RemoveCharacter(st_Character* character);
bool SectorProc_UpdateCharacter(st_Character* character);

void GetSectorAround(int sec_x, int sec_y, st_SECTOR_AROUND* sector_around);
void NetworkUtil_SectorOne(int sector_x, int sector_y, CSerialization& buffer, st_Character* exception);

void AddCharacterToSector(st_Character* character);
void RemoveCharacterFromSector(st_Character* character);

void AddUser(st_Character* character);

void	GetUpdateSectorAround
(st_Character* character, st_SECTOR_AROUND* remove_sector, st_SECTOR_AROUND* add_sector);

void CharacterSectorUpdatePacket(st_Character* character);

void
SectorOne_Damage
(int sector_x, int sector_y, st_Character* exception,
	DWORD vic_id, BYTE vic_hp, DWORD a_id);


// Á¤¸®
void SectorOne_CreateOthers(int sector_x, int sector_y, st_Character* exception);
void SectorOne_DeleteOthers(int sector_x, int sector_y, st_Character* exception);
void SectorOne_MoveStartOthers(int sector_x, int sector_y, st_Character* exception);
void SectorOne_MoveStopOthers(int sector_x, int sector_y, st_Character* exception);

void SectorOne_Attack1(int sector_x, int sector_y, st_Character* exception);
void SectorOne_Attack2(int sector_x, int sector_y, st_Character* exception);
void SectorOne_Attack3(int sector_x, int sector_y, st_Character* exception);

void SectorOne_SyncOthers(int sector_x, int sector_y, st_Character* exception);