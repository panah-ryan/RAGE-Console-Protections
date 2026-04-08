#pragma once
#include "Defines.h"
#include "Detour.h"

enum NetworkGameArrayTypes : int //identifier is always 0
{
	PLAYER_INFO_ARRAY_HANDLER,
	STATIC_PICKUPS_ARRAY_HANDLER,
	DYNAMIC_PICKUPS_ARRAY_HANDLER,
	HOST_VARIABLES_ARRAY_HANDLER,
	CLIENT_VARIABLES_ARRAY_HANDLER,
	PED_GROUPS_ARRAY_HANDLER,
	DISPATCH_ORDER_ARRAY_HANDLER
};

class networkArrayHandler //0x78
{
public:
	int _0x04;
	int _0x08;
	NetworkGameArrayTypes m_Type; //0x0C
	int m_Identifier; //Game ID for REGISTER_CLIENT_BROADCAST_VARS 0x10
	int m_ArraySize; //0x14
	int _0x18[2];
	int _0x20[2];
	int _0x28[2];
	int _0x30[2];
	int _0x38[2];
	CMessageBuffer _0x40;
	char _0x60[0x5F];
	bool m_ShouldSkipMessage; //custom variable this location is unused so we should be able to use it

	networkArrayHandler()
	{
		
	}

	virtual ~networkArrayHandler();
	virtual void Init();
	virtual void Shutdown();
	virtual void call3();
	virtual void PeerHasJoined();
	virtual void PeerHasLeft();
	virtual void NewHost();
	virtual const char* GetHandlerName();
	virtual int GetSizeOfElementIndex();
	virtual void call7();
	virtual int GetElementIndex(int index, int peer);
	virtual void SetElementAuthority();
	virtual void ClearElementAuthority();
	virtual void SetElementDirty();
	virtual bool DoesPeerHaveAuthorityOverThisElement(int index, int peer, bool empty);
	virtual void IsReadyToSync();
	virtual void call12();
	virtual void call13();
	virtual void call14();
	virtual bool WriteElement(CMessageBuffer* buffer, int index);
	virtual bool ReadElement(CMessageBuffer* buffer, int index);
	virtual void CompareElement();
	virtual void SkipElement(CMessageBuffer* buffer, int index);
};

class CScriptVariablesArrayHandler : public networkArrayHandler
{
public:
	char _0x78[0x20014];
	int* m_Variables; //0x2008C
	int m_VariableLength; //0x20090 - hard set to 4
};

int CNetworkArrayHandler_GetElementIndex(networkArrayHandler* handler, int index, int peer);
extern Detour<bool> CNetworkArrayHandler_DoesPeerHaveAuthorityOverThisElement_detour;
bool CNetworkArrayHandler_DoesPeerHaveAuthorityOverThisElement(networkArrayHandler* handler, int index, int peer, bool empty);
extern Detour<bool> CPedGroupsArrayHandler_DoesPeerHaveAuthorityOverThisElement_detour;
bool CPedGroupsArrayHandler_DoesPeerHaveAuthorityOverThisElement(networkArrayHandler* handler, int index, int peer, bool empty);
extern Detour<bool> CPedGroupsArrayHandler_ReadElement_detour;
bool CPedGroupsArrayHandler_ReadElement(networkArrayHandler* handler, CMessageBuffer* buffer, int index);
extern Detour<bool> CScriptClientVariablesArrayHandler_ReadElement_detour;
bool CScriptClientVariablesArrayHandler_ReadElement(CScriptVariablesArrayHandler* handler, CMessageBuffer* message, int index);