#include "stdafx.h"
#include "Defines.h"
#include "natives.h"
#include "caller.h"
#include "Detour.h"

struct explosion_timer
{
	uint32_t tick[2];
	int count[2];
};

uint32_t should_process_event_bitset;
explosion_timer timer[16];

bool ReturnEventSuccess() //Nullsub to replace our Handle and Decide virtual calls
{
	return true;
}

Detour<void> CNetworkEventMgr_HandleEvent_detour;
void CNetworkEventMgr_HandleEvent(CNetworkEventMgr* manager, CNetworkEvent* pEvent, CMessageBuffer* message, int peer, short messageSeq, int eventId)
{
	NetGameEventTypes type = pEvent->m_EventType;
	uint32_t* vftable = *reinterpret_cast<uint32_t**>(pEvent);
	int seek_bits = message->GetPos();
	bool skip_processing = false;
	static uint32_t modified_vftable[24];

	if (((1 << type) & should_process_event_bitset) != 0) //Event should be processed so lets do our checks
	{
		switch (type)
		{
		case REQUEST_PICKUP_EVENT: 
		{
			int pickup_index = static_cast<int>(message->PeekInt(11, seek_bits));
			int player_index = static_cast<int>(message->PeekInt(5, seek_bits + 11));

			if (pickup_index > 1514 || pickup_index < 0 || player_index > 15 || player_index < 0) //Pickup index or player index is out of bounds
				skip_processing = true;

			break;
		}
		case RESURRECT_PLAYER_EVENT: 
		{
			int player_index = static_cast<int>(message->PeekInt(8, seek_bits));

			if (player_index > 15 || player_index < 0) //Player index is out of bounds
				skip_processing = true;

			break;
		}
		case REMOVE_WEAPON_EVENT: 
		case REMOVE_ALL_WEAPONS_EVENT: 
		{
			short net_id = static_cast<short>(message->PeekShort(seek_bits));

			CNetworkObject* net_obj = CNetwork::GetObjectManager()->GetNetworkObject(net_id, false);
			if (net_obj == nullptr) //Invalid network id
				skip_processing = true;

			break;
		}
		case REQUEST_EXPLOSION_EVENT:
		case START_EXPLOSION_EVENT: 
		{
			float pos[3];
			int our_char = NULL;
			float dist = FLT_MAX;
			CPlayerPed* our_ped = CNetwork::GetMyPeerPed();

			EXPLOSION_TAG explosion_type = static_cast<EXPLOSION_TAG>(message->PeekSignedInt(6, seek_bits + 12 + 12));
			message->PeekVector3(19, seek_bits + 12 + 12 + 6 + 32, pos);
			bool ship_destroy = explosion_type == EXPLOSION_SHIP_DESTROY;

			uint32_t time = GetTickCount() - timer[peer].tick[ship_destroy];
			if (time > 60000)
			{
				timer[peer].tick[ship_destroy] = 0;
				timer[peer].count[ship_destroy] = 0;
			}
			
			if (our_ped)
			{
				CEntity* our_ent = our_ped->m_Vehicle ? our_ped->m_Vehicle : dynamic_cast<CEntity*>(our_ped); 
				float mx, my, mz;
				int myPed;
				GET_PLAYER_CHAR(GET_PLAYER_ID(), &myPed);
				GET_CHAR_COORDINATES(myPed, &mx, &my, &mz);
				GET_DISTANCE_BETWEEN_COORDS_3D(pos[0], pos[1], pos[2], mx, my, mz, &dist);

				if (ship_destroy && dist < 60.0f || !ship_destroy && dist < 45.0f)
				{
					timer[peer].count[ship_destroy]++;
					timer[peer].tick[ship_destroy] = GetTickCount();
				}
			}

			if (timer[peer].count[0] > 40 || timer[peer].count[1] > 2) //Skip after more than 2 explosions in range of 60.0f all within a minute
				skip_processing = true;

			break;
		}
		case ALTER_WANTED_LEVEL_EVENT: 
		{
			int player_index = static_cast<int>(message->PeekInt(5, seek_bits));

			if (player_index > 15 || player_index < 0) //Player index is out of bounds
				skip_processing = true;

			break;
		}
		case CREATE_PICKUP_EVENT: 
		{
			uint32_t model_hash = message->PeekInt(32, seek_bits + 57);

			if (IS_THIS_MODEL_A_PED(model_hash) || IS_THIS_MODEL_A_VEHICLE(model_hash)) //Pickup model is a ped or vehicle
				skip_processing = true;

			break;
		}
		case CHANGE_RADIO_STATION_EVENT:
		{
			int station_id = static_cast<int>(message->PeekInt(8, seek_bits + 8 + 8 + 24));

			if (station_id > 30 || station_id < 0) //Station ID is out of bounds
				skip_processing = true;

			break;
		}
		default: //Event doesn't have any issues so lets process like normal
			break;
		}
	}
	
	if((((1 << type) & should_process_event_bitset) == 0) || skip_processing)
	{
		if(skip_processing)
			printf("[Event - %s] - %s tried to send invalid event data!\n", pEvent->GetEventName(), CNetwork::GetPlayerInfo(peer)->GetPlayerName());

		XMemCpy(modified_vftable, vftable, sizeof(modified_vftable)); //copy over vftable of current class

		//HandleExtraData doesn't actually do anything here
		modified_vftable[7] = reinterpret_cast<uint32_t>(ReturnEventSuccess); //Overwrite Handle to not do anything
		modified_vftable[8] = reinterpret_cast<uint32_t>(ReturnEventSuccess); //Overwrite Decide to not do anything
		*reinterpret_cast<uint32_t**>(pEvent) = modified_vftable; //Set our modified vftable to the class
	}

	CNetworkEventMgr_HandleEvent_detour.CallOriginal(manager, pEvent, message, peer, messageSeq, eventId); //We are good to process the event now
}