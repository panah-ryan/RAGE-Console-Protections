#include "stdafx.h"
#include "sync.h"
#include "Defines.h"
#include "natives.h"

int& animation_task_count = *reinterpret_cast<int*>(0x82BF9AA0);
short& weapon_anim_task_count = *reinterpret_cast<short*>(0x82BF9AB0);

Detour<int> CNetObjPhysical_SerializeAttachment_detour;
int CNetObjPhysical_SerializeAttachment(CNetworkObject* obj, SYNC_SERIALIZE type, CMessage* message) //Only runs with non peds
{
	int seek_bits = message->m_buffer.m_CursorPos + message->m_buffer.m_BaseBitOffset;
	CNetworkObject* net_ped = NetworkInterface::GetLocalPlayerPed()->GetNetworkObject();

	if (net_ped && type == SERIALIZE_READ && obj->IsGlobalFlagSet(GLOBAL_FLAG_SCRIPT_OBJECT))
	{
		if (message->PeekBool(seek_bits)) //Check if obj is marked as attachment
		{
			uint16_t net_id = message->PeekShort(seek_bits + 1); //Read network id for what obj is being attached to
			CNetworkObject* base = NetworkInterface::GetObjectManager()->GetNetworkObjectFromID(net_id);
			CNetGamePlayer* owner = obj->GetPlayerOwner();

			if (net_id == net_ped->GetNetworkID()) //if network id is our network id we are being attached to
			{
				if (owner->IsPhysical())
					printf("[Attachment] - %s tried to attach something to you!\n", owner->GetPlayerInfo()->GetPlayerName());

				if (obj->GetObjectType() == NET_OBJ_TYPE_OBJECT)
				{
					int object_index;

					GET_OBJECT_FROM_NETWORK_ID(obj->GetNetworkID(), &object_index);
					SET_OBJECT_COLLISION(object_index, false);
					SET_OBJECT_LIGHTS(object_index, false);
				}

				return CNetObjPhysical_SerializeAttachment_detour.CallOriginal(obj, SERIALIZE_SKIP, message);
			}
			else if (base == obj) //Obj is trying to attach to itself
			{
				if(owner->IsPhysical())
					printf("[Attachment] - %s is infinite attaching!\n", owner->GetPlayerInfo()->GetPlayerName());

				return CNetObjPhysical_SerializeAttachment_detour.CallOriginal(obj, SERIALIZE_SKIP, message);
			}
		}
	}

	return CNetObjPhysical_SerializeAttachment_detour.CallOriginal(obj, type, message);
}

Detour<int> CNetObjPed_SerializeMovementAnimTask_detour;
int CNetObjPed_SerializeMovementAnimTask(CNetworkObject* obj, SYNC_SERIALIZE type, CMessage* message)
{
	int seek_bits = message->m_buffer.m_CursorPos + message->m_buffer.m_BaseBitOffset;
	if (type == SERIALIZE_READ)
	{
		int anim_id = static_cast<int>(message->PeekInt(16, seek_bits)); //read some id
		if ((anim_id > animation_task_count || anim_id < 0) && anim_id != -1) //id is out side of the bounds
		{
			if (obj->GetPlayerOwner()->IsPhysical())
				printf("[Ped Animation Task] - %s tried to set animation task out of bounds! [%i]\n", obj->GetPlayerOwner()->GetPlayerInfo()->GetPlayerName(), anim_id);
			
			return CNetObjPed_SerializeMovementAnimTask_detour.CallOriginal(obj, SERIALIZE_SKIP, message);
		}
	}
	
	return CNetObjPed_SerializeMovementAnimTask_detour.CallOriginal(obj, type, message);
}

Detour<int> CNetObjPed_SerializeAIData_detour;
int CNetObjPed_SerializeAIData(CNetworkObject* obj, SYNC_SERIALIZE type, CMessage* message)
{
	int seek_bits = message->m_buffer.m_CursorPos + message->m_buffer.m_BaseBitOffset;
	if (type == SERIALIZE_READ && obj->IsGlobalFlagSet(GLOBAL_FLAG_SCRIPT_OBJECT))
	{
		int relationship_id = static_cast<int>(message->PeekInt(6, seek_bits));
		if (relationship_id > 55 || relationship_id < 0)
		{
			if (obj->GetPlayerOwner()->IsPhysical())
				printf("[Ped AI Data] - %s tried to set relationship group out of bounds! [%i]\n", obj->GetPlayerOwner()->GetPlayerInfo()->GetPlayerName(), relationship_id);

			return CNetObjPed_SerializeAIData_detour.CallOriginal(obj, SERIALIZE_SKIP, message);
		}
	}

	return CNetObjPed_SerializeAIData_detour.CallOriginal(obj, type, message);
}

Detour<void*> CTaskQuery_Allocate_detour;
void* CTaskQuery_Allocate(int task_type)
{
	*reinterpret_cast<uint32_t*>(0x823E6E60) = 0x60000000; //NOP writing value to task (crashes because of nullptr if pool is full)

	void* task = CTaskQuery_Allocate_detour.CallOriginal(task_type);

	if (task == nullptr)
	{
		printf("[Task Query] - We ran out of pool slots!\n");
		return nullptr;
	}

	*reinterpret_cast<int*>(reinterpret_cast<uint32_t>(task) + 4) = task_type; //Write task_type if we got task pointer
	return task;
}

Detour<BOOL> CNetCloneTaskSimplePlayerAimProjectile_CreateLocalTask_detour;
BOOL CNetCloneTaskSimplePlayerAimProjectile_CreateLocalTask(void* _this, CPed* ped)
{
	CWeaponItem* item = ped->GetInventory()->GetCurrentWeapon();
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
				CNetGamePlayer* player = ped->GetNetworkObject()->GetPlayerOwner();
				if (player->IsPhysical())
					printf("[Aim Projectile Task] - %s's ped tried to throw a rocket!\n", player->GetPlayerInfo()->GetPlayerName());
			}
		}
	}

	return CNetCloneTaskSimplePlayerAimProjectile_CreateLocalTask_detour.CallOriginal(_this, ped);
}

Detour<int> CNetObjPed_SerializeGameStateData_detour;
int CNetObjPed_SerializeGameStateData(CNetworkObject* obj, SYNC_SERIALIZE type, CMessage* message)
{
	//Lets call other Serialize Gamestate Data
	int ret = ((int(*)(CNetworkObject*, SYNC_SERIALIZE, CMessage*))0x82720298)(obj, type, message);
	*reinterpret_cast<uint32_t*>(0x8270DB88) = 0x38600000 | ret; //li %r3, ret

	int seek_bits = message->m_buffer.m_CursorPos + message->m_buffer.m_BaseBitOffset + 1 + 2 + 7;
	if (type == SERIALIZE_READ)
	{
		WEAPON_TYPE weapon_index = static_cast<WEAPON_TYPE>(message->PeekInt(6, seek_bits));
		if (weapon_index >= WEAPON_ANYWEAPON || weapon_index < WEAPON_UNARMED)
		{
			if (obj->GetPlayerOwner()->IsPhysical())
				printf("[Ped Game State Data] - %s tried to set out of bounds weapon index! [%i]\n", obj->GetPlayerOwner()->GetPlayerInfo()->GetPlayerName(), weapon_index);

			return CNetObjPed_SerializeGameStateData_detour.CallOriginal(obj, SERIALIZE_SKIP, message);
		}

		if (obj->IsGlobalFlagSet(GLOBAL_FLAG_SCRIPT_OBJECT))
		{
			seek_bits += 6 + 1;
			seek_bits += weapon_index == WEAPON_OBJECT ? 12 : 1;
			seek_bits += message->PeekBool(seek_bits) ? 13 : 1;
			seek_bits += message->PeekBool(seek_bits) ? 16 : 1;
			seek_bits += 2 + 13 + 8;

			int animation_group = static_cast<int>(message->PeekInt(16, seek_bits));
			if (animation_group >= animation_task_count || animation_group < 0)
			{
				if (obj->GetPlayerOwner()->IsPhysical())
					printf("[Ped Game State Data] - %s tried to set out of bounds default animation group index! [%i]\n", obj->GetPlayerOwner()->GetPlayerInfo()->GetPlayerName(), animation_group);

				return CNetObjPed_SerializeGameStateData_detour.CallOriginal(obj, SERIALIZE_SKIP, message);
			}
		}
	}

	return CNetObjPed_SerializeGameStateData_detour.CallOriginal(obj, type, message);
}

Detour<int> CNetObjPed_SerializeAttachment_detour;
int CNetObjPed_SerializeAttachment(CNetworkObject* obj, SYNC_SERIALIZE type, CMessage* message) //Only runs with peds
{
	int seek_bits = message->m_buffer.m_CursorPos + message->m_buffer.m_BaseBitOffset;
	CNetworkObject* net_ped = NetworkInterface::GetLocalPlayerPed()->GetNetworkObject();

	if (net_ped && type == SERIALIZE_READ && obj->IsGlobalFlagSet(GLOBAL_FLAG_SCRIPT_OBJECT))
	{
		if (message->PeekBool(seek_bits)) //Check if ped is attaching to something
		{
			uint16_t net_id = message->PeekShort(seek_bits + 1); //Read network id for what obj is being attached to
			CNetworkObject* base = NetworkInterface::GetObjectManager()->GetNetworkObjectFromID(net_id);
			CNetGamePlayer* owner = obj->GetPlayerOwner();

			if (net_id == net_ped->GetNetworkID()) //if network id is our network id we are being attached to
			{
				if (owner->IsPhysical())
					printf("[Attachment] - %s tried to attach something to you!\n", owner->GetPlayerInfo()->GetPlayerName());

				return CNetObjPed_SerializeAttachment_detour.CallOriginal(obj, SERIALIZE_SKIP, message);
			}
			else if (base == obj) //Obj is trying to attach to itself
			{
				if(owner->IsPhysical())
					printf("[Attachment] - %s is infinite attaching!\n", owner->GetPlayerInfo()->GetPlayerName());

				return CNetObjPhysical_SerializeAttachment_detour.CallOriginal(obj, SERIALIZE_SKIP, message);
			}
		}
	}

	return CNetObjPed_SerializeAttachment_detour.CallOriginal(obj, type, message);
}

Detour<int> CNetObjPlayer_SerializeAppearanceData_detour;
int CNetObjPlayer_SerializeAppearanceData(CNetworkObject* obj, SYNC_SERIALIZE type, CMessage* message, int unused)
{
	float distance = FLT_MAX;
	int seek_bits = message->m_buffer.m_CursorPos + message->m_buffer.m_BaseBitOffset;

	if (type == SERIALIZE_READ)
	{
		uint32_t model_hash = message->PeekInt(32, seek_bits);
		if (!IS_THIS_MODEL_A_PED(model_hash) || model_hash == PED_PLAYER || model_hash == PED_SUPERLOD)
		{
			if (obj->GetPlayerOwner()->IsPhysical())
			{
				CEntity* our_ent = NetworkInterface::GetLocalPlayerPed()->m_Vehicle ? NetworkInterface::GetLocalPlayerPed()->m_Vehicle : dynamic_cast<CEntity*>(NetworkInterface::GetLocalPlayerPed());
				if (our_ent && obj->GetEntity())
				{
					GET_DISTANCE_BETWEEN_COORDS_3D(obj->GetEntity()->GetPosition()[0], obj->GetEntity()->GetPosition()[1], obj->GetEntity()->GetPosition()[2], our_ent->GetPosition()[0], our_ent->GetPosition()[1], our_ent->GetPosition()[2], &distance);

					if (distance < 70.0f)
						printf("[Player Apperance Data] - %s tried to set model to crash model! [0x%X]\n", obj->GetPlayerOwner()->GetPlayerInfo()->GetPlayerName(), model_hash);
				}
			}

			return CNetObjPlayer_SerializeAppearanceData_detour.CallOriginal(obj, SERIALIZE_SKIP, message, unused);
		}
	}

	return CNetObjPlayer_SerializeAppearanceData_detour.CallOriginal(obj, type, message, unused);
}

Detour<int> CDummyTask_SerializeWeaponThrow_detour;
int CDummyTask_SerializeWeaponThrow(void* _this, SYNC_SERIALIZE type, CMessage* message)
{
	int seek_bits = message->m_buffer.m_CursorPos + message->m_buffer.m_BaseBitOffset;

	if (type == SERIALIZE_READ)
	{
		short anim_index = static_cast<short>(message->PeekInt(16, seek_bits));
		if (anim_index < 0 || anim_index > weapon_anim_task_count)
		{
			printf("[Dummy Task] - Invalid Weapon Throw Animation Index!\n");
			return CDummyTask_SerializeWeaponThrow_detour.CallOriginal(_this, SERIALIZE_SKIP, message);
		}
	}

	return CDummyTask_SerializeWeaponThrow_detour.CallOriginal(_this, type, message);
}