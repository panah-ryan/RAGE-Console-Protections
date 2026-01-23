#pragma once
#include "stdafx.h"
#include "Defines.h"
#include "Detour.h"

struct peerDataMsg
{
	uint8_t m_PeerID;
	char _padding1[3];
	int m_playerColor;
	int m_playerTeam;
	int _0x0C;
	uint64_t m_SessionID;
	XONLINE_NAT_TYPE m_NatType;
	int _0x1C;
	int _0x20;
	bool _0x24;
	char _padding2[3];
};

struct reassignConfirmMsg
{
	uint8_t m_PeerID;
	char _padding1[3];
	int m_NumRemoteObjects;
	int m_NumLocalObjects;
	int _0x0C;
	char* m_MessageBuffer;
	size_t m_ByteSizeOfBuffer;
	size_t m_MaxByteSizeOfBuffer;
};

struct reassignNegotiateMsg
{
	uint8_t m_PeerID;
	char _padding1[3];
	int m_NumObjects;
	int _0x08;
	char* m_MessageBuffer;
	size_t m_BytesSizeOfBuffer;
	size_t m_MaxByteSizeOfBuffer;
};

struct reassignResponseMsg
{
	uint8_t m_PeerID;
	bool m_GotNegotiateResponse;
	char _padding[2];
	int m_ResponseType;
};

struct netComplaintMsg
{
	uint64_t m_ComplainerId;
	uint64_t m_ComplaineeIds[64];
	int m_NumComplainees;
};

extern Detour<void> netPeerMgr_HandleGetReadyToStartPlaying_detour;
void netPeerMgr_HandleGetReadyToStartPlaying(void* netPeerMgr, netEvent* evt);

extern Detour<bool> CMsgPeerData_Import_detour;
bool CMsgPeerData_Import(peerDataMsg* msg, uint32_t key, char* buffer, size_t size, int* count);

extern Detour<bool> CMsgReassignConfirm_Import_detour;
bool CMsgReassignConfirm_Import(reassignConfirmMsg* msg, uint32_t key, char* buffer, size_t size, int* count);

extern Detour<bool> CMsgReassignNegotiate_Import_detour;
bool CMsgReassignNegotiate_Import(reassignNegotiateMsg* msg, uint32_t key, char* buffer, size_t size, int* count);

extern Detour<bool> CMsgReassignResponse_Import_detour;
bool CMsgReassignResponse_Import(reassignResponseMsg* msg, uint32_t key, char* buffer, size_t size, int* count);

extern Detour<bool> netComplaintMsg_Import_detour;
bool netComplaintMsg_Import(netComplaintMsg* msg, uint32_t key, char* buffer, size_t size, int* count);

extern Detour<bool> CNetworkPlayerMgr_AddTemporaryPlayer_detour;
bool CNetworkPlayerMgr_AddTemporaryPlayer(void* mgr, CPlayerInfo* info, void* endpoint, peerDataMsg* data, int player_status);

extern Detour<bool> CMsgPeerData_Export_detour;
bool CMsgPeerData_Export(peerDataMsg* msg, uint32_t key, char* buffer, size_t size, int* count);

