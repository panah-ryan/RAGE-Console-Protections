#pragma once
#include "stdafx.h"
#include "Detour.h"
#include "Defines.h"

extern Detour<void> CNetworkEventMgr_HandleEvent_detour;
void CNetworkEventMgr_HandleEvent(CNetworkEventMgr* manager, CNetworkEvent* pEvent, CMessageBuffer* message, int peer, short messageSeq, int eventId);