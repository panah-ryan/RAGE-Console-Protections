#pragma once
#include "Detour.h"
#include "Defines.h"

extern Detour<int> CNetObjPhysical_SerializeAttachment_detour;
int CNetObjPhysical_SerializeAttachment(CNetworkObject* obj, SYNC_SERIALIZE type, CMessage* message);

extern Detour<int> CNetObjPed_SerializeMovementAnimTask_detour;
int CNetObjPed_SerializeMovementAnimTask(CNetworkObject* obj, SYNC_SERIALIZE type, CMessage* message);

extern Detour<int> CNetObjPed_SerializeAIData_detour;
int CNetObjPed_SerializeAIData(CNetworkObject* obj, SYNC_SERIALIZE type, CMessage* message);

extern Detour<void*> CTaskQuery_Allocate_detour;
void* CTaskQuery_Allocate(int task_type);

extern Detour<BOOL> CNetCloneTaskSimplePlayerAimProjectile_CreateLocalTask_detour;
BOOL CNetCloneTaskSimplePlayerAimProjectile_CreateLocalTask(void* _this, CPed* ped);

extern Detour<int> CNetObjPed_SerializeGameStateData_detour;
int CNetObjPed_SerializeGameStateData(CNetworkObject* obj, SYNC_SERIALIZE type, CMessage* message);

extern Detour<int> CNetObjPed_SerializeAttachment_detour;
int CNetObjPed_SerializeAttachment(CNetworkObject* obj, SYNC_SERIALIZE type, CMessage* message);

extern Detour<int> CNetObjPlayer_SerializeAppearanceData_detour;
int CNetObjPlayer_SerializeAppearanceData(CNetworkObject* obj, SYNC_SERIALIZE type, CMessage* message, int unused);

extern Detour<int> CDummyTask_SerializeWeaponThrow_detour;
int CDummyTask_SerializeWeaponThrow(void* _this, SYNC_SERIALIZE type, CMessage* message);