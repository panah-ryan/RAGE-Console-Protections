#include "stdafx.h"
#include "sync.h"
#include "Defines.h"
#include "natives.h"

int& animation_task_count = *reinterpret_cast<int*>(0x82BF9AA0);
short& weapon_anim_task_count = *reinterpret_cast<short*>(0x82BF9AB0);

Detour<int> CNetObjPhysical_SyncAttach_detour;
int CNetObjPhysical_SyncAttach(CNetworkObject* obj, SyncType type, CMessageBuffer* message) //Only runs with non peds
{
	int seek_bits = message->m_buffer.m_CursorPos + message->m_buffer.m_BaseBitOffset;
	CNetworkObject* net_ped = CWorld::GetMainPlayerPed()->GetNetworkObject();

	if (net_ped && type == SYNC_READ && obj->IsGlobalFlagSet(CNetworkObject::NETOBJGLOBALFLAG_SCRIPTOBJECT))
	{
		if (message->PeekBool(seek_bits)) //Check if obj is marked as attachment
		{
			uint16_t net_id = message->PeekObjectId(seek_bits + 1); //Read network id for what obj is being attached to
			CNetworkObject* base = ms_objectMgr.GetNetworkObject(net_id);
			CNetworkPeer* owner = obj->GetPeerOwner();

			if (net_id == net_ped->GetNetworkID()) //if network id is our network id we are being attached to
			{
				if (owner->IsValid())
					printf("[Attachment] - %s tried to attach something to you!\n", owner->GetGamerInfo()->GetPlayerName());

				if (obj->GetObjectType() == NET_OBJ_TYPE_OBJECT)
				{
					int object_index;

					GET_OBJECT_FROM_NETWORK_ID(obj->GetNetworkID(), &object_index);
					SET_OBJECT_COLLISION(object_index, false);
					SET_OBJECT_LIGHTS(object_index, false);
				}

				return CNetObjPhysical_SyncAttach_detour.CallOriginal(obj, SYNC_SKIP, message);
			}
			else if (base == obj) //Obj is trying to attach to itself
			{
				if(owner->IsValid())
					printf("[Attachment] - %s is infinite attaching!\n", owner->GetGamerInfo()->GetPlayerName());

				return CNetObjPhysical_SyncAttach_detour.CallOriginal(obj, SYNC_SKIP, message);
			}
		}
	}

	return CNetObjPhysical_SyncAttach_detour.CallOriginal(obj, type, message);
}

Detour<int> CNetObjPed_SyncMovementGroup_detour;
int CNetObjPed_SyncMovementGroup(CNetworkObject* obj, SyncType type, CMessageBuffer* message)
{
	int seek_bits = message->m_buffer.m_CursorPos + message->m_buffer.m_BaseBitOffset;
	if (type == SYNC_READ)
	{
		int anim_group_id = static_cast<int>(message->PeekInt(16, seek_bits)); //read animation group id
		if ((anim_group_id > animation_task_count || anim_group_id < 0) && anim_group_id != -1) //id is out side of the bounds
		{
			if (obj->GetPeerOwner()->IsValid())
				printf("[Ped Movement Group] - %s tried to set animation group ID out of bounds! [%i]\n", obj->GetPeerOwner()->GetGamerInfo()->GetPlayerName(), anim_group_id);
			
			return CNetObjPed_SyncMovementGroup_detour.CallOriginal(obj, SYNC_SKIP, message);
		}
	}
	
	return CNetObjPed_SyncMovementGroup_detour.CallOriginal(obj, type, message);
}

Detour<int> CNetObjPed_SyncPedAI_detour;
int CNetObjPed_SyncPedAI(CNetworkObject* obj, SyncType type, CMessageBuffer* message)
{
	int seek_bits = message->m_buffer.m_CursorPos + message->m_buffer.m_BaseBitOffset;
	if (type == SYNC_READ && obj->IsGlobalFlagSet(CNetworkObject::NETOBJGLOBALFLAG_SCRIPTOBJECT))
	{
		int relationship_group = static_cast<int>(message->PeekInt(6, seek_bits));
		if (relationship_group > 55 || relationship_group < 0)
		{
			if (obj->GetPeerOwner()->IsValid())
				printf("[Ped AI Data] - %s tried to set relationship group out of bounds! [%i]\n", obj->GetPeerOwner()->GetGamerInfo()->GetPlayerName(), relationship_group);

			return CNetObjPed_SyncPedAI_detour.CallOriginal(obj, SYNC_SKIP, message);
		}
	}

	return CNetObjPed_SyncPedAI_detour.CallOriginal(obj, type, message);
}

Detour<void*> CQueriableInterface_CreateEmptyTaskInfo_detour;
void* CQueriableInterface_CreateEmptyTaskInfo(int task_type)
{
	*reinterpret_cast<uint32_t*>(0x823E6E60) = 0x60000000; //NOP writing value to task (crashes because of nullptr if pool is full)

	void* task = CQueriableInterface_CreateEmptyTaskInfo_detour.CallOriginal(task_type);

	if (task == nullptr)
	{
		printf("[Task Query] - We ran out of pool slots!\n");
		return nullptr;
	}

	*reinterpret_cast<int*>(reinterpret_cast<uint32_t>(task) + 4) = task_type; //Write task_type if we got task pointer
	return task;
}

Detour<BOOL> CNetCloneTaskSimplePlayerAimProjectile_Update_detour;
BOOL CNetCloneTaskSimplePlayerAimProjectile_Update(void* _this, CPed* ped)
{
	CWeapon* item = ped->GetInventory()->GetWeaponUsable();
	if (item && item->m_WeaponIndex == WEAPON_ROCKET)
	{
		if (ped->IsPlayer())
		{
			if (ped->GetPlayerInfo())
				printf("[Aim Projectile Task] - %s tried to throw a rocket!\n", ped->GetPlayerInfo()->GetPlayerName());
		}
		else
		{
			if (ped->GetNetworkObject())
			{
				CNetworkPeer* player = ped->GetNetworkObject()->GetPeerOwner();
				if (player->IsValid())
					printf("[Aim Projectile Task] - %s's ped tried to throw a rocket!\n", player->GetGamerInfo()->GetPlayerName());
			}
		}
	}

	return CNetCloneTaskSimplePlayerAimProjectile_Update_detour.CallOriginal(_this, ped);
}

Detour<int> CNetObjPed_SyncGameState_detour;
int CNetObjPed_SyncGameState(CNetworkObject* obj, SyncType type, CMessageBuffer* message)
{
	//Lets call other Sync Gamestate Data
	int ret = ((int(*)(CNetworkObject*, SyncType, CMessageBuffer*))0x82720298)(obj, type, message);
	*reinterpret_cast<uint32_t*>(0x8270DB88) = 0x38600000 | ret; //li %r3, ret

	int seek_bits = message->m_buffer.m_CursorPos + message->m_buffer.m_BaseBitOffset + 1 + 2 + 7;
	if (type == SYNC_READ)
	{
		eWeaponType weapon_index = static_cast<eWeaponType>(message->PeekInt(6, seek_bits));
		if (weapon_index >= WEAPON_ANYWEAPON || weapon_index < WEAPON_UNARMED)
		{
			if (obj->GetPeerOwner()->IsValid())
				printf("[Ped Game State Data] - %s tried to set out of bounds weapon index! [%i]\n", obj->GetPeerOwner()->GetGamerInfo()->GetPlayerName(), weapon_index);

			return CNetObjPed_SyncGameState_detour.CallOriginal(obj, SYNC_SKIP, message);
		}

		if (obj->IsGlobalFlagSet(CNetworkObject::NETOBJGLOBALFLAG_SCRIPTOBJECT))
		{
			seek_bits += 6 + 1;
			seek_bits += weapon_index == WEAPON_OBJECT ? 12 : 1;
			seek_bits += message->PeekBool(seek_bits) ? 13 : 1;
			seek_bits += message->PeekBool(seek_bits) ? 16 : 1;
			seek_bits += 2 + 13 + 8;

			int animation_group = static_cast<int>(message->PeekInt(16, seek_bits));
			if (animation_group >= animation_task_count || animation_group < 0)
			{
				if (obj->GetPeerOwner()->IsValid())
					printf("[Ped Game State Data] - %s tried to set out of bounds default animation group index! [%i]\n", obj->GetPeerOwner()->GetGamerInfo()->GetPlayerName(), animation_group);

				return CNetObjPed_SyncGameState_detour.CallOriginal(obj, SYNC_SKIP, message);
			}
		}
	}

	return CNetObjPed_SyncGameState_detour.CallOriginal(obj, type, message);
}

Detour<int> CNetObjPed_SyncAttach_detour;
int CNetObjPed_SyncAttach(CNetworkObject* obj, SyncType type, CMessageBuffer* message) //Only runs with peds
{
	int seek_bits = message->m_buffer.m_CursorPos + message->m_buffer.m_BaseBitOffset;
	CNetworkObject* net_ped = CWorld::GetMainPlayerPed()->GetNetworkObject();

	if (net_ped && type == SYNC_READ && obj->IsGlobalFlagSet(CNetworkObject::NETOBJGLOBALFLAG_SCRIPTOBJECT))
	{
		if (message->PeekBool(seek_bits)) //Check if ped is attaching to something
		{
			uint16_t net_id = message->PeekObjectId(seek_bits + 1); //Read network id for what obj is being attached to
			CNetworkObject* base = ms_objectMgr.GetNetworkObject(net_id);
			CNetworkPeer* owner = obj->GetPeerOwner();

			if (net_id == net_ped->GetNetworkID()) //if network id is our network id we are being attached to
			{
				if (owner->IsValid())
					printf("[Attachment] - %s tried to attach something to you!\n", owner->GetGamerInfo()->GetPlayerName());

				return CNetObjPed_SyncAttach_detour.CallOriginal(obj, SYNC_SKIP, message);
			}
			else if (base == obj) //Obj is trying to attach to itself
			{
				if(owner->IsValid())
					printf("[Attachment] - %s is infinite attaching!\n", owner->GetGamerInfo()->GetPlayerName());

				return CNetObjPhysical_SyncAttach_detour.CallOriginal(obj, SYNC_SKIP, message);
			}
		}
	}

	return CNetObjPed_SyncAttach_detour.CallOriginal(obj, type, message);
}

Detour<int> CNetObjPlayer_SyncPedAppearance_detour;
int CNetObjPlayer_SyncPedAppearance(CNetworkObject* obj, SyncType type, CMessageBuffer* message, int unused)
{
	float distance = FLT_MAX;
	int seek_bits = message->m_buffer.m_CursorPos + message->m_buffer.m_BaseBitOffset;

	if (type == SYNC_READ)
	{
		uint32_t model_hash = message->PeekInt(32, seek_bits);
		if (!IS_THIS_MODEL_A_PED(model_hash) || model_hash == PED_PLAYER || model_hash == PED_SUPERLOD)
		{
			if (obj->GetPeerOwner()->IsValid())
			{
				CPlayerPed* main_ped = CWorld::GetMainPlayerPed();
				if (main_ped)
				{
					CEntity* our_ent = main_ped->m_Vehicle ? main_ped->m_Vehicle : dynamic_cast<CEntity*>(main_ped);

					if (our_ent && obj->GetEntity())
					{
						GET_DISTANCE_BETWEEN_COORDS_3D(obj->GetEntity()->GetPosition()[0], obj->GetEntity()->GetPosition()[1], obj->GetEntity()->GetPosition()[2], our_ent->GetPosition()[0], our_ent->GetPosition()[1], our_ent->GetPosition()[2], &distance);

						if (distance < 70.0f)
							printf("[Player Apperance Data] - %s tried to set model to crash model! [0x%X]\n", obj->GetPeerOwner()->GetGamerInfo()->GetPlayerName(), model_hash);
					}
				}
			}

			return CNetObjPlayer_SyncPedAppearance_detour.CallOriginal(obj, SYNC_SKIP, message, unused);
		}
	}

	return CNetObjPlayer_SyncPedAppearance_detour.CallOriginal(obj, type, message, unused);
}

Detour<int> CDummyTask_PlayAnim_SyncNetworkData_detour;
int CDummyTask_PlayAnim_SyncNetworkData(void* _this, SyncType type, CMessageBuffer* message)
{
	int seek_bits = message->m_buffer.m_CursorPos + message->m_buffer.m_BaseBitOffset;

	if (type == SYNC_READ)
	{
		short anim_index = static_cast<short>(message->PeekInt(16, seek_bits));
		if (anim_index < 0 || anim_index > weapon_anim_task_count)
		{
			printf("[Dummy Task] - Invalid Weapon Throw Animation Index!\n");
			return CDummyTask_PlayAnim_SyncNetworkData_detour.CallOriginal(_this, SYNC_SKIP, message);
		}
	}

	return CDummyTask_PlayAnim_SyncNetworkData_detour.CallOriginal(_this, type, message);
}