#pragma once
#include "stdafx.h"
#include "Detour.h"
#include "Defines.h"

extern Detour<void> netEventMgr_HandleEvent_detour;
void netEventMgr_HandleEvent(netEventMgr* manager, CNetworkEvent* pEvent, CMessage* message, int peer, short messageSeq, int eventId);