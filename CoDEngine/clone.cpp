#include <algorithm>
#include "stdafx.h"
#include "clone.h"
#include "natives.h"
#include "Detour.h"
#include <vector>
#include "util.h"

enum eObjectCreatedBy : int
{
	CREATEDBY_INVALID,
	CREATEDBY_RANDOM,
	CREATEDBY_MISSION,
	CREATEDBY_TEMP,
	CREATEDBY_FRAGMENT_CACHE,
	CREATEDBY_UNUSED,
	CREATEDBY_GAME
};

uint32_t should_process_clone_bitset = 0x3BF; //All clones enabled
PED_MODELS ped_blacklist_models[] = { 
	PED_PLAYER, PED_SUPERLOD, PED_CS_ANDREI, PED_CS_ANGIE, PED_CS_BADMAN, PED_CS_BLEDAR, PED_CS_BULGARIN,
	PED_CS_BULGARINHENCH, PED_CS_CIA, PED_CS_DARDAN, PED_CS_DAVETHEMATE, PED_CS_DMITRI,
	PED_CS_EDTHEMATE, PED_CS_FAUSTIN, PED_CS_FRANCIS, PED_CS_HOSSAN, PED_CS_ILYENA,
	PED_CS_IVAN, PED_CS_JAY, PED_CS_JIMMY_PEGORINO, PED_CS_MEL, PED_CS_MICHELLE,
	PED_CS_MICKEY, PED_CS_OFFICIAL, PED_CS_RAY_BOCCINO, PED_CS_SERGEI, PED_CS_VLAD,
	PED_CS_WHIPPINGGIRL, PED_CS_MANNY, PED_CS_ANTHONY, PED_CS_ASHLEY, PED_CS_ASSISTANT,
	PED_CS_CAPTAIN, PED_CS_CHARLIEUC, PED_CS_DARKO, PED_CS_DWAYNE, PED_CS_ELI_JESTER,
	PED_CS_ELIZABETA, PED_CS_GAYTONY, PED_CS_GERRYMC, PED_CS_GORDON, PED_CS_ISSAC,
	PED_CS_JOHNNYTHEBIKER, PED_CS_JONGRAVELLI, PED_CS_JORGE, PED_CS_KAT, PED_CS_KILLER,
	PED_CS_LUIS, PED_CS_MAGICIAN, PED_CS_MAMC, PED_CS_MELODY, PED_CS_MITCHCOP,
	PED_CS_MORI, PED_CS_PBXGIRL2, PED_CS_PHILB, PED_CS_PLAYBOYX, PED_CS_PRIEST,
	PED_CS_RICKY, PED_CS_TOMMY, PED_CS_TRAMP, PED_CS_BRIAN, PED_CS_CHARISE,
	PED_CS_CLARENCE, PED_CS_EDDIELOW, PED_CS_GRACIE, PED_CS_JEFF, PED_CS_MARNIE,
	PED_CS_MARSHAL, PED_CS_PATHOS, PED_CS_SARAH, PED_CS_ROMAN_D, PED_CS_ROMAN_T,
	PED_CS_ROMAN_W, PED_CS_BRUCIE_B, PED_CS_BRUCIE_T, PED_CS_BRUCIE_W, PED_CS_BERNIE_CRANEC,
	PED_CS_BERNIE_CRANET, PED_CS_BERNIE_CRANEW, PED_CS_LILJACOB_B, PED_CS_LILJACOB_J, PED_CS_MALLORIE_D,
	PED_CS_MALLORIE_J, PED_CS_MALLORIE_W, PED_CS_DERRICKMC_B, PED_CS_DERRICKMC_D, PED_CS_MICHAEL_B,
	PED_CS_MICHAEL_D, PED_CS_PACKIEMC_B, PED_CS_PACKIEMC_D, PED_CS_KATEMC_D, PED_CS_KATEMC_W,
	PED_CS_NIKO, PED_CS_ANDREAS, PED_CS_ANGELGUY, PED_CS_ANGUS, PED_CS_ASHLEYA,
	PED_CS_BILLY, PED_CS_BIKESTRIPPER, PED_CS_BRIANJ, PED_CS_CLAY, PED_CS_DAVE_GROSSMAN,
	PED_CS_DESEAN, PED_CS_ELIZABETAT, PED_CS_FRANKIE, PED_CS_JASON, PED_CS_JIM_FITZ,
	PED_CS_MAITRE_D, PED_CS_MALC, PED_CS_MITCHB, PED_CS_MARTA, PED_CS_MASSEUSE,
	PED_CS_MATTHEWS, PED_CS_MCCORNISH, PED_CS_PAUL_LAMONT, PED_CS_PRISONDLC_01, PED_CS_PRISONGUARD,
	PED_CS_RAYGOON, PED_CS_RAY_LEATHER, PED_CS_RONCERO, PED_CS_RUSSIAN_GOON, PED_CS_RUSSIANGOON2,
	PED_CS_TERRY, PED_CS_SKANK, PED_CS_STUBBS, PED_CS_STUBBS_SUIT, PED_CS_BAZ, PED_CS_JEREMY, PED_CS_ABDUL, PED_CS_ABODYGUARD, PED_CS_ADRIANA,
	PED_CS_AHMAD, PED_CS_ARMANDO, PED_CS_ARNAUD, PED_CS_BATHROOM, PED_CS_BILLY2,
	PED_CS_BULGARIN2, PED_CS_BRUCIE2, PED_CS_DAISY, PED_CS_DERRICK2, PED_CS_DESSIE,
	PED_CS_DOCKGOON, PED_CS_EUGENE, PED_CS_EVAN2, PED_CS_GPARTY_01, PED_CS_GRACIE2,
	PED_CS_HENRIQUE, PED_CS_ISSAC2, PED_CS_JOHNNY2, PED_CS_MARGOT, PED_CS_MAURICE,
	PED_CS_MORI_K, PED_CS_MR_SANTOS, PED_CS_NIKO2, PED_CS_OYVEY, PED_CS_PACKIE2,
	PED_CS_PAPI, PED_CS_ROCCO, PED_CS_ROMAN2, PED_CS_RUS_KN, PED_CS_SHARON,
	PED_CS_TAHIR, PED_CS_TIMUR, PED_CS_TONY, PED_CS_TONYAQ, PED_CS_TRAMP2,
	PED_CS_TRIAD, PED_CS_TROY, PED_CS_VIC, PED_CS_VICGIRL, PED_CS_VINCE, PED_CS_YUSEF };

std::vector<PED_MODELS> ped_blacklist;
std::vector<uint32_t> object_blacklist;

struct clone_queue //Untested remove if it causes issues
{
	CMessageBuffer message;
	eNetworkObjectType objectType;
	uint8_t peer;
	short objectID;
	uint8_t objectFlags;

	clone_queue(eNetworkObjectType objectType, uint8_t peer, short objectID, uint8_t objectFlags, uint8_t* buffer, int bits)
		: objectType(objectType), peer(peer), objectID(objectID), objectFlags(objectFlags)
	{
		message.Setup(buffer, bits << 3, 0, true);
	}
};

std::vector<clone_queue> clone_create_queue;

void setup_blacklists()
{
	for (int i = 0; i < ARRAYSIZE(ped_blacklist_models); i++)
		ped_blacklist.push_back(ped_blacklist_models[i]);
}

inline bool IsVehicleObjectType(eNetworkObjectType type)
{
	return type > NET_OBJ_TYPE_DUMMY_PED && type != NET_OBJ_TYPE_OBJECT;
}

/*
* ----- Message Breakdown -----
* CNetObjPlayer:
*	- Player Index 5 bits
*	- Model Hash 32 bits
*	- Has Target Vehicle 1 bit
* 
* CNetObjPed:
*	- Created By 2 bits
*	- Model Hash 32 bits
*	- Some Flag 16 bits
*	if SCRIPTOBJECT
*		- Script Creation Sequence 8 bits
*		- Has Attachment 1 bit
*		- Has Money 1 bit
*		if HAS MONEY
*			- Money 10 bits
*		- Dies When Injured 1 bit
*		- Flee When Driving Vehicle 1 bit
*		- Can Only Be Damaged By Relationship Group 1 bit
*	- Has Prop 1 bit
*	if HAS PROP
*		- Prop Model Hash 32 bits
*	- Is In Vehicle 1 bit
*	- Some Flag 1 bit
* 
* CNetObjDummyPed:
*	- Model Hash 32 bits
*	- Has Prop 1 bit
*	if HAS PROP
*		- Prop Model Hash 32 bits
*		- Weapon Index 6 bits
*	- Packed Props 20 bits
*	- Packed Variation 1 32 bits
*	- Packed Variation 2 32 bits
* 
* CNetObjAutomobile: (Automobile, Plane)
*	CNetObjVehicle unpack
*	- Are All Doors Closed 1 bit
*	if !ALL DOORS CLOSED
*		for loop 6
*			- Is Door Closed 1 bit
* 
* CNetObjObject:
*	- Created By 3 bits
*	if SCRIPTOBJECT || CREATEDBY_GAME || CREATEDBY_TEMP
*		- Model Hash 32 bits
*		if SCRIPTOBJECT
*			- Script Creation Sequence 8 bits
*		- Has Attachment 1 bit
*	else
*		- Dummy Position 19 bits
*		if CREATEDBY_FRAGEMENT_CACHE
*			- Frag Group Index 5 bits
*		else
*			- Player Wants Control 1 bit
*		- Some flag (if Player Wants Control wasn't already read)
* 
* CNetObjHeli:
*	CNetObjVehicle unpack
*	- Player Owner 5 bits
*	- Raised Height 2 bits
* 
* CNetObjVehicle: (Bike, Train, Boat)
*	- Model Hash 32 bits
*	- Population Type 3 bits
*	if SCRIPTOBJECT
*		- Script Creation Sequence 8 bits
*		- Has Attachment 1 bit
*		- Take Out Of Parked Car Budget 1 bit
*		- Can Only Be Damaged By Relationship Group 1 bit
*	- Status 3 bits
*	- Some Flag 1 bit
*	- Siren 1 bit
*	- Needs To Be Hotwired 1 bit
*	- Tires Don't Burst 1 bit
*/

Detour<void> CNetworkObjectMgr_ProcessCloneCreateData_detour;
void CNetworkObjectMgr_ProcessCloneCreateData(CNetworkObjectMgr* netObjMgr, uint8_t peer, eNetworkObjectType objectType, short objectID, uint8_t objectFlags, CMessageBuffer* message)
{
	bool should_we_create = false;
	BOOL is_mission_object = (objectFlags & CNetworkObject::NETOBJGLOBALFLAG_SCRIPTOBJECT) >> 3;
	static rate_limiter clone_create_limiter(seconds_to_ms(5), 5);

	if (IsVehicleObjectType(objectType)) //We only queue Vehicles
	{
		for (size_t i = 0; i < clone_create_queue.size(); i++) //Check our queue for objects we already sanitized
		{
			clone_queue q = clone_create_queue.at(i);
			if (q.objectID == objectID) //We have the current message in queue so lets process it
			{
				CNetworkObjectMgr_ProcessCloneCreateData_detour.CallOriginal(netObjMgr, peer, objectType, objectID, objectFlags, message);
				return;
			}
		}
	}

	if (objectType >= NET_OBJ_TYPE_PLAYER && objectType < NET_OBJ_TYPE_COUNT //Object type we are trying to spawn is a valid type
		&& ((1 << objectType) & should_process_clone_bitset) != 0) //We are allowing this type to be spawned
	{
		int seek_bits = message->GetPos();

		switch (objectType)
		{
		case NET_OBJ_TYPE_PLAYER:
		{
			int player_index = static_cast<int>(message->PeekInt(5, seek_bits));
			uint32_t model_hash = message->PeekInt(32, seek_bits + 5);

			if (player_index > 16 || player_index < 0 || player_index == ms_PeerMgr.GetMyPeer()->GetPeerID()) //Invalid peer id
			{
				player_index = CWorld::FindSlotForNewPlayer(); //Just call original function game calls for getting slot when host

				if (player_index != -1)
					message->PokeInt(player_index, 5, seek_bits); //Write in new player index
			}

			if (!CWorld::GetPlayerInfo(peer)->GetPlayerPed() //Check if they already have a ped (duplicate ped)
				&& player_index != -1) //Check if function returned valid index
				should_we_create = true;

			if (model_hash == NULL || (model_hash != PED_M_Y_MULTIPLAYER && model_hash != PED_F_Y_MULTIPLAYER)) //They are trying to spawn not as default multiplayer model, lets fix that
				message->PokeInt(PED_M_Y_MULTIPLAYER, 32, seek_bits + 5);

			break;
		}
		case NET_OBJ_TYPE_PED:
		{
			uint32_t model_hash = message->PeekInt(32, seek_bits + 2);
			seek_bits += 2 + 32 + 16;

			if (is_mission_object) //Read extra data for mission objects
			{
				seek_bits += 8 + 1;
				seek_bits += message->PeekBool(seek_bits) ? 10 + 1 : 1;
				seek_bits += 1 + 1 + 1;
			}

			if (message->PeekBool(seek_bits)) //Message flag if we have a prop
				message->PokeInt(NULL, 32, seek_bits + 1); //Set prop model to NULL

			if (model_hash != NULL && IS_THIS_MODEL_A_PED(model_hash) //Model is a valid PED model
				&& std::find(ped_blacklist.begin(), ped_blacklist.end(), model_hash) == ped_blacklist.end()) //Model isn't in our blacklist
				should_we_create = true;

			break;
		}
		case NET_OBJ_TYPE_DUMMY_PED: //Same as above basically
		{
			uint32_t model_hash = message->PeekInt(32, seek_bits);

			if (message->PeekBool(seek_bits + 32))
				message->PokeInt(NULL, 32, seek_bits + 32 + 1);

			if (model_hash != NULL && IS_THIS_MODEL_A_PED(model_hash) 
				&& std::find(ped_blacklist.begin(), ped_blacklist.end(), model_hash) == ped_blacklist.end()) //Model is valid, is a ped, and doesn't exist in our blacklist
				should_we_create = true;

			break;
		}
		case NET_OBJ_TYPE_AUTOMOBILE:
		{
			uint32_t model_hash = message->PeekInt(32, seek_bits);

			if (model_hash != NULL && IS_THIS_MODEL_A_CAR(model_hash)) //Valid model for a car
				should_we_create = true;

			break;
		}
		case NET_OBJ_TYPE_OBJECT:
		{
			eObjectCreatedBy created_by = static_cast<eObjectCreatedBy>(message->PeekInt(3, seek_bits) + 1);

			if (is_mission_object || created_by == CREATEDBY_GAME || created_by == CREATEDBY_TEMP)
			{
				uint32_t model_hash = message->PeekInt(32, seek_bits + 3);

				//check trains in object pool! Cars are OK..
				if (model_hash != NULL && !IS_THIS_MODEL_A_PED(model_hash)
					&& std::find(object_blacklist.begin(), object_blacklist.end(), model_hash) == object_blacklist.end()) //Can't check if model is an object so lets check its not ped or vehicle and isn't in our blacklist
					should_we_create = true;
			}
			else
				should_we_create = true; //Its not reading a model so we can assume its safe

			break;
		}
		case NET_OBJ_TYPE_BIKE:
		{
			uint32_t model_hash = message->PeekInt(32, seek_bits);

			if (model_hash != NULL && IS_THIS_MODEL_A_BIKE(model_hash)) //Model is a bike
				should_we_create = true;

			break;
		}
		case NET_OBJ_TYPE_TRAIN:
		{
			uint32_t model_hash = message->PeekInt(32, seek_bits);
			
			if (model_hash != NULL && IS_THIS_MODEL_A_TRAIN(model_hash)) //Model is a train
				should_we_create = true;

			break;
		}
		case NET_OBJ_TYPE_HELI:
		{
			uint32_t model_hash = message->PeekInt(32, seek_bits);

			if (model_hash != NULL && IS_THIS_MODEL_A_HELI(model_hash)) //Model is a heli
				should_we_create = true;

			break;
		}
		case NET_OBJ_TYPE_BOAT:
		{
			uint32_t model_hash = message->PeekInt(32, seek_bits);

			if (model_hash != NULL && IS_THIS_MODEL_A_BOAT(model_hash)) //Model is a boat
				should_we_create = true;

			break;
		}
		case NET_OBJ_TYPE_PLANE:
		{
			uint32_t model_hash = message->PeekInt(32, seek_bits);

			if (model_hash != NULL && IS_THIS_MODEL_A_VEHICLE(model_hash)) //Can't check if model is a plane (always returns false) so haven't found any issues if its any vehicle
				should_we_create = true;

			break;
		}
		default:
			break;
		}
	}

	if (!should_we_create)
	{
		if (objectType < NET_OBJ_TYPE_COUNT && objectType >= NET_OBJ_TYPE_PLAYER)
			printf("[Clone Create] - Prevented %s from spawning by player %s!\n", 
				CNetworkObjectMgr::GetObjectTypeName(objectType, false), 
				CWorld::GetPlayerInfo(peer)->GetPlayerName());

		return;
	}

	if (IsVehicleObjectType(objectType) && clone_create_limiter.process()) //Lets only queue Vehicles and check if we exceeded our rate limiter
	{
		uint8_t* clone_buffer = new uint8_t[message->GetSize()];

		if (clone_buffer)
		{
			clone_queue clone = clone_queue(objectType, peer, objectID, objectFlags, clone_buffer, message->GetSize());
			if(clone_create_limiter.exceeded_last_process())
				printf("[Clone Create] - Rate Limiter has kicked in!\n");

			clone_create_queue.push_back(clone);
			return; //Add to the queue and then return so we don't ack it
		}
	}

	CNetworkObjectMgr_ProcessCloneCreateData_detour.CallOriginal(netObjMgr, peer, objectType, objectID, objectFlags, message); //Continue original code since its safe to spawn
}

Detour<bool> CNetObjHeli_CreateClone_detour;
bool CNetObjHeli_CreateClone(CNetworkObject* net_heli, CMessageBuffer* message)
{
	bool ret = CNetObjHeli_CreateClone_detour.CallOriginal(net_heli, message);

	if (ret)
	{
		CHeli* heli = net_heli->GetBaseHeli();
		if (heli && heli->m_OwnerPlayer < -1 || heli->m_OwnerPlayer > 15)
		{
			printf("[Network Heli] - Corrected invalid Owner Player crash!\n");
			heli->m_OwnerPlayer = -1;
		}
	}

	return ret;
}

Detour<void> CNetworkObjectMgr_Update_detour;
void CNetworkObjectMgr_Update(CNetworkObjectMgr* mgr, bool bUpdateNetworkObjects)
{
	static uint32_t queue_tick = GetTickCount();

	CNetworkObjectMgr_Update_detour.CallOriginal(mgr, bUpdateNetworkObjects); //Run original code

	if (!clone_create_queue.empty() && (queue_tick + seconds_to_ms(2) < GetTickCount())) //Queue isn't empty and we waited 2 seconds lets process one
	{
		clone_queue q = clone_create_queue.back();
		if (q.message.m_buffer.m_ReadBits != nullptr) //We have a valid message
		{
			printf("[Clone Queue] - Processing %s (%i) from %s\n", 
				mgr->GetObjectTypeName(q.objectType, false), 
				q.objectID, 
				CWorld::GetPlayerInfo(q.peer)->GetPlayerName());
			mgr->ProcessCloneCreateData(q.peer, q.objectType, q.objectID, q.objectFlags, &q.message);
			delete q.message.m_buffer.m_ReadBits; //Processed message lets delete it
		}

		clone_create_queue.pop_back(); //remove it from the queue
	}
}