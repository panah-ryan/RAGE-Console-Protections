#include "stdafx.h"
#include "Defines.h"
#include "natives.h"
#include "caller.h"
#include "Detour.h"
#include "util.h"

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

/*
* ----- Message Breakdown -----
* CRequestControl
* Handle
*	- Peer Id 5 bits
*	- Object Id 12 bits
* Reply
*	- Granted 1 bit
* 
* CGiveControl
* Handle
*	- Control Count 4 bits
*	for loop CONTROL COUNT
*		- Object Id 12 bits
*		- Object Type 4 bits
*		- Proximity Migrate 1 bit
* Reply
*	for loop CONTROL COUNT
*		- Took Control 1 bit
*		- Exists 1 bit
*	- Too Many Objects 1 bit
* ExtraData
*	for loop CONTROL COUNT
*		- Exists 1 bit
*		if EXISTS
*			- Script Object 1 bit
* 
* CObjectIdFreed
*	- Object Id Count 6 bits
*	for OBJECT ID COUNT
*		- Object Id 12 bits
* 
* CWeaponDamage
* Handle
*	- Type 2 bits
*	- Weapon Type 6 bits
*	- Has Damage Override 1 bit
*	if HAS DAMAGE OVERRIDE
*		- Damage Override 8 or 10 bits (Depends on WEAPON TYPE)
*	if TYPE EQUALS 3
*		- Has Damage Time 1 bit
*		if DAMAGE TIME
*			- Damage Time 32 bits
*	else
*		- Parent Object 12 bits
*		- Hit Object 12 bits
*	if TYPE EQUALS 2
*		- Component 5 bits 
*	else
*		- Local Impact X 13 bits
*		- Local Impact Y 13 bits
*		- Local Impact Z 13 bits
*		if TYPE EQUALS 1
*			- Has Damaged Vehicle 1 bit
*			if DAMAGED VEHICLE
*				- Tires Damaged 3 bits
*				- Suspension Damaged 3 bits
* Reply
*	- Player Health 13 bits
* 
* CRequestPickup
* Handle
*	- Pickup Slot 11 bits
*	- Player Id 5 bits
*	- Refernce Index 16 bits
* 
* Reply
*	- Picked Up 1 bit
* 
* CGameClockAndWeather
* Handle
*	- Days 3 bits
*	- Hours 6 bits
*	- Minutes 7 bits
*	- Seconds 7 bits
*	- Old Weather 5 bits
*	- Has New Weather 1 bit
*	if New Weather
*		- New Weather 7 bits
* 
* CResurrectPlayer
* Handle
*	- Player Id 8 bits
*	- Position 19 bits
*	- Heading 8 bits
* 
* CResurrectLocalPlayer
* Handle
*	- Position 19 bits
*	- Timestamp 32 bits
* 
* CGiveWeapon
* Handle
*	- Object Id 12 bits
*	- Weapon Index 6 bits
*	- Ammo 12 bits
*	- Play Audio 1 bit
* 
* CRemoveWeapon
* Handle
*	- Object Id 12 bits
*	- Weapon Index 6 bits
* 
* CRemoveAllWeapons
* Handle
*	- Object Id 12 bits
* 
* CVehicleComponent
* Handle
*	- Vehicle Object Id 12 bits
*	- Controller Object Id 12 bits
*	- Component Index 3 bits
*	- Requesting Control 1 bit
*	- Remote 1 bit
*	if !REQUESTING CONTROL and REMOTE
*		- Remote Occupier 12 bits
* 
* Reply
*	- Granted 1 bit
*	if GRANTED
*		- In Seat 1 bit
*		if IN SEAT
*			- Object Id 12 bits
* 
* CRequestFire CStartFire
* Handle
*	- Fire Count 3 bits
*	for loop FIRE COUNT
*		- On Entity 1 bit
*		if ON ENTITY
*			- Object Id 12 bits
*		else
*			- Position 18 bits
*			- Type 2 bits
*		- Object Id 12 bits
*		- Some Bool 1 bit
*		- Some Flag 6 bits
*		- Some Float 16 bits
*		- Some Float 16 bits
* 
* CRequestExplosion CStartExplosion
* Handle
*	- Exploding Object Id 12 bits
*	- Owner Object Id 12 bits
*	- Type 6 bits
*	- Size Scale 32 bits
*	- Position 19 bits
*	- Is Dummy Object Explosion 1 bit
*	- Activation Delay 16 bits
*	- Make Sound 1 bit
*	- Cam Shake 8 bits
*	- No Damage 1 bit
*	- No Fx 1 bit
*	- Attached Object Id 12 bits
*	- Direction 19 bits
* 
* CStartProjectile
* Handle
*	- Owner Object Id 12 bits
*	- Type 7 bits
*	- Initial Position 19 bits
*	- Target Object Id 12 bits
*	- Has Direction 1 bit
*	if DIRECTION
*		- Direction 19 bits
*	- Projectile Object 1 bit
*	if PROJECTILE OBJECT
*		- Throw Task Sequence 32 bits
*		- Thrown Object Id 12 bits
* 
* CSessionSettingsChanged
* Handle
*	- Game Type 32 bits
*	- Parameter 1 32 bits
*	- Parameter 2 32 bits 
*	- Parameter 3 32 bits 
*	- Parameter 4 32 bits 
*	- Parameter 5 32 bits 
*	- Parameter 6 32 bits
* 
* CAlterWantedLevel
* Handle 
*	- Player Id 5 bits
*	- Is Wanted Level Change 1 bit
*	if WANTED LEVEL CHANGE
*		- Wanted Level 14 bits
*	- Crime Type 5 bits
*	- Add Multiplier 1 bit
* 
* CCreatePickup
* Handle
*	- Position 19 bits
*	- Model Hash 32 bits
*	- Type 5 bits
*	- Owner Object Id 12 bits
*	- Blipped 1 bit
*	- Has Value 1 bit 
*	if VALUE
*		- Value 16 bits
*	- Has Room Hash Key 1 bit
*	if ROOM HASH KEY
*		- Room Hash Key 32 bits
* 
* CChangeRadioStation
* Handle
*	- Track Playtime 24 bits
*	- Track Category 8 bits
*	- Track Index 8 bits
*	- Station ID 8 bits
* 
* CUpdateGPSDestination
* Handle
*	- X Position 19 bits
*	- Y Position 19 bits
*	- GPS 1 bit
*	- Timestamp 32 bits
* 
* CRagdollRequest
* Handle
*	- Object Id 12 bits
* 
* CMarkAsNoLongerNeeded
* Handle
*	- Count 4 bits
*	for loop COUNT
*		- Object Id 12 bits
* 
* CPlayerTaunt
* Handle
*	- Context String 32 characters
*	- Some Flag 32 bits
* 
* CDoorBreak
* Handle
*	- Vehicle Object Id 12 bits
*	- Door 3 bits
* 
* CHostVariablesVerify
* Handle
*	- Handler Id 6 bits
*	- Checksum 32 bits
* 
* Reply
*	- Verified 1 bit
*	- Checksums Differed 1 bit
*/

Detour<void> CNetworkEventMgr_HandleEvent_detour;
void CNetworkEventMgr_HandleEvent(void* manager, CNetworkEvent* pEvent, CMessageBuffer* message, int peer, short messageSeq, int eventId)
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
			int pickup_slot = static_cast<int>(message->PeekInt(11, seek_bits));
			int player_index = static_cast<int>(message->PeekInt(5, seek_bits + 11));

			if (pickup_slot > 1514 || pickup_slot < 0 || player_index > 15 || player_index < 0) //Pickup slot or player index is out of bounds
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
			short net_id = static_cast<short>(message->PeekObjectId(seek_bits));

			CNetworkObject* net_obj = ms_objectMgr.GetNetworkObject(net_id, false);
			if (net_obj == nullptr) //Invalid network id
				skip_processing = true;

			break;
		}
		case REQUEST_EXPLOSION_EVENT:
		case START_EXPLOSION_EVENT: 
		{
			static rate_limiter explosion_limiter(seconds_to_ms(5), 5);

			EXPLOSION_TAG explosion_type = static_cast<EXPLOSION_TAG>(message->PeekSignedInt(6, seek_bits + 12 + 12));

			//seek_bits + 12 is Explosion Owner Unsigned32 read 12 bits
			if (explosion_type == EXPLOSION_SHIP_DESTROY && explosion_limiter.process())
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
			printf("[Event - %s] - %s tried to send invalid event data!\n", 
				pEvent->GetEventName(), 
				CWorld::GetPlayerInfo(peer)->GetPlayerName());

		XMemCpy(modified_vftable, vftable, sizeof(modified_vftable)); //copy over vftable of current class

		//HandleExtraData doesn't actually do anything here
		modified_vftable[7] = reinterpret_cast<uint32_t>(ReturnEventSuccess); //Overwrite Handle to not do anything
		modified_vftable[8] = reinterpret_cast<uint32_t>(ReturnEventSuccess); //Overwrite Decide to not do anything
		*reinterpret_cast<uint32_t**>(pEvent) = modified_vftable; //Set our modified vftable to the class
	}

	CNetworkEventMgr_HandleEvent_detour.CallOriginal(manager, pEvent, message, peer, messageSeq, eventId); //We are good to process the event now
}