#include "stdafx.h"
#include "helpers.h"
#include "Defines.h"
#include "caller.h"
#include "natives.h"
#include "array.h"
#include "sync.h"
#include "other.h"
#include "event.h"
#include "clone.h"
//test

using namespace::std;

#define GTAIV_TITLE_ID 0x545407F2

uint32_t title_id = NULL;
BOOL term_process;
BOOL shutdown_title_hooks;
BOOL booted_game;
LDR_DATA_TABLE_ENTRY* module_handle;
BOOL is_devkit;
BOOL constant_loop = TRUE;
BOOL external_unload;

Caller* call = nullptr;

unsigned int scrComputeHash(const char *key)
{
	size_t len = strlen(key);
	unsigned int hash, i;

	for (hash = i = 0; i < len; ++i)
	{
		hash += tolower(key[i]);
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
}

Detour<void> CWeaponInfo_LoadWeaponXMLFile_detour;
void CWeaponInfo_LoadWeaponXMLFile(const char* weaponxml)
{
	CWeaponInfo_LoadWeaponXMLFile_detour.CallOriginal(weaponxml);

	CWeaponInfo* rocket = CWeaponInfo::GetWeaponInfo(WEAPON_ROCKET);
	if (rocket)
	{
		rocket->m_AimFlags |= 0xC005; //CAN_AIM - CAN_FREE_AIM - ANIM_RELOAD - ANIM_CROUCH_FIRE
		rocket->m_AnimationIndex = 29; //grenade animation group
	}

	CWeaponInfo* e2_rocket = CWeaponInfo::GetWeaponInfo(WEAPON_EPISODIC_18);
	if (e2_rocket)
	{
		e2_rocket->m_AimFlags |= 0xC005; //CAN_AIM - CAN_FREE_AIM - ANIM_RELOAD - ANIM_CROUCH_FIRE
		e2_rocket->m_AnimationIndex = 29; //grenade animation group
	}
}

BOOL Patches()
{
	call = new Caller();

	setup_blacklists();

	*reinterpret_cast<uint16_t*>(0x821B55E0) = 0x7F03; //File Bypass

	//Array Protections
	//Function is to small to hook so lets just overwrite the virtual call table
	*reinterpret_cast<void**>(0x82058964) = CNetworkArrayHandler_GetElementIndex; //CPlayerInfo
	*reinterpret_cast<void**>(0x82058A0C) = CNetworkArrayHandler_GetElementIndex; //CStaticPickups
	*reinterpret_cast<void**>(0x82058AC4) = CNetworkArrayHandler_GetElementIndex; //CDynamicPickups
	*reinterpret_cast<void**>(0x82058C54) = CNetworkArrayHandler_GetElementIndex; //CScriptHostVariables
	*reinterpret_cast<void**>(0x82058D1C) = CNetworkArrayHandler_GetElementIndex; //CScriptClientVariables
	*reinterpret_cast<void**>(0x82058DCC) = CNetworkArrayHandler_GetElementIndex; //CPedGroups
	*reinterpret_cast<void**>(0x820BCD9C) = CNetworkArrayHandler_GetElementIndex; //CDispatch

	CNetworkArrayHandler_DoesPeerHaveAuthorityOverThisElement_detour.SetupDetour(0x82705878, CNetworkArrayHandler_DoesPeerHaveAuthorityOverThisElement);
	CPedGroupsArrayHandler_DoesPeerHaveAuthorityOverThisElement_detour.SetupDetour(0x82717330, CPedGroupsArrayHandler_DoesPeerHaveAuthorityOverThisElement);
	CPedGroupsArrayHandler_ReadElement_detour.SetupDetour(0x82717798, CPedGroupsArrayHandler_ReadElement);
	CScriptClientVariablesArrayHandler_ReadElement_detour.SetupDetour(0x82716998, CScriptClientVariablesArrayHandler_ReadElement);

	//Sync Protections
	CNetObjPhysical_SyncAttach_detour.SetupDetour(0x82722638, CNetObjPhysical_SyncAttach);
	CNetObjPed_SyncMovementGroup_detour.SetupDetour(0x8270F8F0, CNetObjPed_SyncMovementGroup);
	CNetObjPed_SyncPedAI_detour.SetupDetour(0x8270BA90, CNetObjPed_SyncPedAI);
	CQueriableInterface_CreateEmptyTaskInfo_detour.SetupDetour(0x823E5860, CQueriableInterface_CreateEmptyTaskInfo);
	CNetCloneTaskSimplePlayerAimProjectile_Update_detour.SetupDetour(0x8271B5A8, CNetCloneTaskSimplePlayerAimProjectile_Update);
	CNetObjPed_SyncGameState_detour.SetupDetour(0x8270DB70, CNetObjPed_SyncGameState);
	CNetObjPed_SyncAttach_detour.SetupDetour(0x8270EEA8, CNetObjPed_SyncAttach);
	CNetObjPlayer_SyncPedAppearance_detour.SetupDetour(0x82779A90, CNetObjPlayer_SyncPedAppearance);
	CDummyTask_PlayAnim_SyncNetworkData_detour.SetupDetour(0x8254B350, CDummyTask_PlayAnim_SyncNetworkData);

	//Other Protections
	CNetworkPeerMgr_HandleGetReadyToStartPlaying_detour.SetupDetour(0x826FF198, CNetworkPeerMgr_HandleGetReadyToStartPlaying);
	CMsgPeerData_Import_detour.SetupDetour(0x826FFAE0, CMsgPeerData_Import);
	CMsgReassignConfirm_Import_detour.SetupDetour(0x82784138, CMsgReassignConfirm_Import);
	CMsgReassignNegotiate_Import_detour.SetupDetour(0x82783F98, CMsgReassignNegotiate_Import);
	CMsgReassignResponse_Import_detour.SetupDetour(0x827842D8, CMsgReassignResponse_Import);
	netComplaintMsg_Import_detour.SetupDetour(0x829EC4B8, netComplaintMsg_Import);
	CNetworkPeerMgr_AddRemotePeer_detour.SetupDetour(0x82700108, CNetworkPeerMgr_AddRemotePeer);
	CWeaponInfo_LoadWeaponXMLFile_detour.SetupDetour(0x8229B548, CWeaponInfo_LoadWeaponXMLFile);

	//Event Protections
	CNetworkEventMgr_HandleEvent_detour.SetupDetour(0x826E39C8, CNetworkEventMgr_HandleEvent);

	//Clone Protections
	CNetworkObjectMgr_ProcessCloneCreateData_detour.SetupDetour(0x826F1670, CNetworkObjectMgr_ProcessCloneCreateData);
	CNetObjHeli_CreateClone_detour.SetupDetour(0x8272A428, CNetObjHeli_CreateClone);
	CNetworkObjectMgr_Update_detour.SetupDetour(0x826F3DD0, CNetworkObjectMgr_Update);

	//Other Hooks
	CMsgPeerData_Export_detour.SetupDetour(0x826FFA28, CMsgPeerData_Export);

	booted_game = TRUE;
	return TRUE;
}

void Uninit()
{
	if (call)
		delete call;

	booted_game = FALSE;
	shutdown_title_hooks = FALSE;
}

DWORD WINAPI MainThread(LPVOID)
{
	while (!term_process)
	{
		title_id = XamGetCurrentTitleId();

		if (!shutdown_title_hooks && title_id == GTAIV_TITLE_ID)
		{
			if (!booted_game && MmIsAddressValid(reinterpret_cast<PVOID>(0x821F2BF8)) && *reinterpret_cast<uint32_t*>(0x821F2BF8) == 0x38610060)
			{
				if (!Patches())
					Sleep(1000);
			}
		}
		else
			Uninit();
			
		Sleep(33);
	}

	Uninit();
	constant_loop = FALSE;

	if (!external_unload) //Lets unload ourself
	{
		module_handle->LoadCount = 1;
		XexUnloadImageAndExitThread(module_handle, GetCurrentThread());
	}

	return TRUE;
}


BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	//Disc tray check
	uint8_t buffer[16];

	buffer[0] = 10;
	HalSendSMCMessage(buffer, buffer);

	if (buffer[1] == 0x60)
		return E_ABORT;

	is_devkit = XboxHardwareInfo->BldrMagic == 0x5E4E;

	if (dwReason == DLL_PROCESS_DETACH)
	{
		external_unload = TRUE;
		term_process = TRUE;
		shutdown_title_hooks = TRUE;

		while (constant_loop)
			Sleep(33);

		Sleep(500);
		return TRUE;
	}

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		while (!MmIsAddressValid((PVOID)0x81A3E030)) //wait till we load xam before doing anything
			Sleep(100);

		ExCreateThread(0, 0, 0, 0, MainThread, 0, EX_CREATE_FLAG_CORE5 | EX_CREATE_FLAG_SYSTEM);
		module_handle = reinterpret_cast<LDR_DATA_TABLE_ENTRY*>(hModule);
	}

	return TRUE;
}