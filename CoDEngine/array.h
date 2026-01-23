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
	CMessage _0x40;
	char _0x60[0x5F];
	bool m_ShouldSkipMessage; //custom variable this location is unused so we should be able to use it

	networkArrayHandler()
	{
		
	}

	virtual ~networkArrayHandler();
	virtual void call1();
	virtual void call2();
	virtual void call3();
	virtual void call4();
	virtual void call5();
	virtual void call6();
	virtual const char* GetHandlerName();
	virtual int GetSizeOfElementIndex();
	virtual void call7();
	virtual int GetElementIndex(int index, int peer);
	virtual void call8();
	virtual void call9();
	virtual void call10();
	virtual bool CanApplyElementData(int index, int peer, bool empty);
	virtual void call11();
	virtual void call12();
	virtual void call13();
	virtual void call14();
	virtual bool WriteUpdate(CMessage* buffer, int index);
	virtual bool ReadUpdate(CMessage* buffer, int index);
	virtual void call15();
	virtual void SkipUpdate(CMessage* buffer, int index);
	virtual void LogElementData(bool r4, int index);
};

class CPlayerInfoArrayHandler : public networkArrayHandler
{
public:
	char _0x78[0x0C];
	CPlayerInfo** m_Players; //0x84
};

class CStaticPickupsArrayHandler : public networkArrayHandler
{
public:
	char _0x78[0x70C];
	CStaticPickupsInfo* m_Pickups; //0x784
};

class CDynamicPickupsArrayHandler : public networkArrayHandler 
{
public:
	char _0x78[0xA29];
	uint8_t m_CreationSeq[100]; //0xAA1
	char _padding;
	uint16_t m_PedOwner[100]; //0xB06
	short _padding1;
	CDynamicPickupsInfo* m_Pickups; //0xBD0
	uint8_t m_ScriptCreationSeq; //0xBD4
};

class CScriptVariablesArrayHandler : public networkArrayHandler
{
public:
	char _0x78[0x20014];
	int* m_Variables; //0x2008C
	int m_VariableLength; //0x20090 - hard set to 4
};

class CPedGroupsArrayHandler : public networkArrayHandler
{
public:
	CPedGroup* m_PedGroups; //0x78
};

struct CDispatchInfo //0x50
{
	char _0x00[0x11];
	char m_OrderType; //0x11
	char _0x12[0x0E];
	__vector4 m_Position; //0x20
	char _0x30[0x20];

	CPed* GetPedDispatch()
	{
		return ((CPed*(*)(CDispatchInfo*))0x8215BA78)(this);
	}
};

class CDispatchArrayHandler : public networkArrayHandler
{
public:
	char _0x78[0xB80];
	CDispatchInfo* m_Dispatches;
};

int CNetworkArrayHandler_GetElementIndex(networkArrayHandler* handler, int index, int peer);
extern Detour<bool> CNetworkArrayHandler_CanApplyElementData_detour;
bool CNetworkArrayHandler_CanApplyElementData(networkArrayHandler* handler, int index, int peer, bool empty);
extern Detour<bool> CPedGroupsArrayHandler_CanApplyElementData_detour;
bool CPedGroupsArrayHandler_CanApplyElement(CPedGroupsArrayHandler* handler, int index, int peer, bool empty);
extern Detour<bool> CPedGroupsArrayHandler_ReadUpdate_detour;
bool CPedGroupsArrayHandler_ReadUpdate(CPedGroupsArrayHandler* handler, CMessage* buffer, int index);
extern Detour<bool> CScriptClientVariablesArrayHandler_ReadUpdate_detour;
bool CScriptClientVariablesArrayHandler_ReadUpdate(CScriptVariablesArrayHandler* handler, CMessage* message, int index);