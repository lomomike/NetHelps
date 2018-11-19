#pragma once

#include "stdafx.h"
#include "dbgeng.h"
#include "wdbgexts.h"
#include "engextcpp.hpp"
#include "corhdr.h"
#include "xclrdata.h"
#include "crosscomp.h"
#include "sospriv.h"
#include "syncblock.h"

template <class T>
struct ZeroInit
{
	ZeroInit()
	{
		memset(static_cast<T*>(this), 0, sizeof(T));
	}
};

struct DacpSyncBlockData : ZeroInit<DacpSyncBlockData>
{
	CLRDATA_ADDRESS Object;
	BOOL            bFree; // if set, no other fields are useful

						   // fields below provide data from this, so it's just for display
	CLRDATA_ADDRESS SyncBlockPointer;
	DWORD           COMFlags;
	UINT            MonitorHeld;
	UINT            Recursion;
	CLRDATA_ADDRESS HoldingThread;
	UINT            AdditionalThreadCount;
	CLRDATA_ADDRESS appDomainPtr;

	// SyncBlockCount will always be filled in with the number of SyncBlocks.
	// SyncBlocks may be requested from [1,SyncBlockCount]
	UINT            SyncBlockCount;

	// SyncBlockNumber must be from [1,SyncBlockCount]    
	// If there are no SyncBlocks, a call to Request with SyncBlockCount = 1
	// will return E_FAIL.
	HRESULT Request(ISOSDacInterface *sos, UINT SyncBlockNumber)
	{
		return sos->GetSyncBlockData(SyncBlockNumber, this);
	}
};

class EXT_CLASS : public ExtExtension
{
public:
	EXT_COMMAND_METHOD(DumpSyncBlk);
};

