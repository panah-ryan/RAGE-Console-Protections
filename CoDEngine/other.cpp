#include "stdafx.h"
#include "Defines.h"
#include "other.h"
#include <algorithm>
#include "natives.h"

int& session_flags = *reinterpret_cast<int*>(0x830FAFB0);

Detour<void> netPeerMgr_HandleGetReadyToStartPlaying_detour;
void netPeerMgr_HandleGetReadyToStartPlaying(void* netPeerMgr, netEvent* evt)
{
	if (session_flags > 7 && session_flags < 11) //Session flags show we are already playing
	{
		CNetGamePlayer* peer = NetworkInterface::GetPlayerFromConnectionID(netPeerMgr, evt->m_CxnId);
		if (peer->IsPhysical())
			printf("[Get Ready to Start Playing] - %s tried to send GetReadyToStartPlaying message when we are already playing!\n", peer->GetPlayerInfo()->GetPlayerName());

		return;
	}

	netPeerMgr_HandleGetReadyToStartPlaying_detour.CallOriginal(netPeerMgr, evt);
}

Detour<bool> CMsgPeerData_Import_detour;
bool CMsgPeerData_Import(peerDataMsg* msg, uint32_t key, char* buffer, size_t size, int* count)
{
	bool ret = CMsgPeerData_Import_detour.CallOriginal(msg, key, buffer, size, count);

	if (msg->m_PeerID > 15 || msg->m_PeerID < 0)
	{
		printf("[Peer Data] - Host sent us an invalid peer id. Fixing to prevent crash but we should leave game to be safe.\n");

		for (int i = 0; i < 16; i++) //We can't ever be sent to a game with no open slots (host wouldn't allow it)
		{
			if (!NetworkInterface::GetNetPlayer(i)->IsPhysical()) //We found an open slot in the game lets set our peer id to that slot
				msg->m_PeerID = i;
		}
	}

	return ret;
}

Detour<bool> CMsgPeerData_Export_detour;
bool CMsgPeerData_Export(peerDataMsg* msg, uint32_t key, char* buffer, size_t size, int* count)
{
	msg->m_NatType = XONLINE_NAT_OPEN;

	return CMsgPeerData_Export_detour.CallOriginal(msg, key, buffer, size, count);
}

Detour<bool> CMsgReassignConfirm_Import_detour;
bool CMsgReassignConfirm_Import(reassignConfirmMsg* msg, uint32_t key, char* buffer, size_t size, int* count)
{
	bool ret = CMsgReassignConfirm_Import_detour.CallOriginal(msg, key, buffer, size, count);

	if (msg->m_PeerID > 15 || msg->m_PeerID < 0) //Check if peer id is valid return false if it isn't this causes the message to not be used
	{
		printf("[Reassign Confirm] - Got invalid peer id for reassignment.\n");
		return false;
	}

	return ret;
}

Detour<bool> CMsgReassignNegotiate_Import_detour;
bool CMsgReassignNegotiate_Import(reassignNegotiateMsg* msg, uint32_t key, char* buffer, size_t size, int* count)
{
	bool ret = CMsgReassignNegotiate_Import_detour.CallOriginal(msg, key, buffer, size, count);

	if (msg->m_PeerID > 15 || msg->m_PeerID < 0) //Check if peer id is valid return false if it isn't this causes the message to not be used
	{
		printf("[Reassign Negotiate] - Got invalid peer id for reassignment.\n");
		return false;
	}

	return ret;
}

Detour<bool> CMsgReassignResponse_Import_detour;
bool CMsgReassignResponse_Import(reassignResponseMsg* msg, uint32_t key, char* buffer, size_t size, int* count)
{
	bool ret = CMsgReassignResponse_Import_detour.CallOriginal(msg, key, buffer, size, count);

	if (msg->m_PeerID > 15 || msg->m_PeerID < 0) 
	{
		printf("[Reassign Response] - Got invalid peer id for reassignment.\n");
		msg->m_PeerID = 0; //set valid peer id
		msg->m_ResponseType = 5; //set to invalid response type so code doesn't do anything
	}

	return ret; 
}

Detour<bool> netComplaintMsg_Import_detour;
bool netComplaintMsg_Import(netComplaintMsg* msg, uint32_t key, char* buffer, size_t size, int* count)
{
	//Lets just not even reply to this
	return false;
}

Detour<bool> CNetworkPlayerMgr_AddTemporaryPlayer_detour;
bool CNetworkPlayerMgr_AddTemporaryPlayer(void* mgr, CPlayerInfo* info, void* endpoint, peerDataMsg* data, int player_status)
{
	std::string player_name = std::string(info->m_Gamertag);
	if (std::count(player_name.begin(), player_name.end(), '~') % 2) //We found odd amount of ~ lets remove them
	{
		std::replace(player_name.begin(), player_name.end(), '~', '-'); //Lets replace all ~ with - then write new gamertag
		strncpy(info->m_Gamertag, player_name.c_str(), sizeof(info->m_Gamertag));
	}

	return CNetworkPlayerMgr_AddTemporaryPlayer_detour.CallOriginal(mgr, info, endpoint, data, player_status);
}