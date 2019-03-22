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

struct ObjHeader
{
    // !!! Notice: m_SyncBlockValue *MUST* be the last field in ObjHeader.
#ifdef _WIN64
    DWORD    m_alignpad;
#endif // _WIN64

    DWORD m_SyncBlockValue;      // the Index and the Bits
};

#define HASHCODE_BITS                   26

#define BIT_SBLK_IS_HASH_OR_SYNCBLKINDEX    0x08000000
#define BIT_SBLK_FINALIZER_RUN              0x40000000
#define BIT_SBLK_SPIN_LOCK                  0x10000000
#define SBLK_MASK_LOCK_THREADID             0x000003FF   // special value of 0 + 1023 thread ids
#define SBLK_MASK_LOCK_RECLEVEL             0x0000FC00   // 64 recursion levels
#define SBLK_APPDOMAIN_SHIFT                16           // shift right this much to get appdomain index
#define SBLK_MASK_APPDOMAININDEX            0x000007FF   // 2048 appdomain indices
#define SBLK_RECLEVEL_SHIFT                 10           // shift right this much to get recursion level
#define BIT_SBLK_IS_HASHCODE            0x04000000
#define MASK_HASHCODE                   ((1<<HASHCODE_BITS)-1)
#define SYNCBLOCKINDEX_BITS             26
#define MASK_SYNCBLOCKINDEX             ((1<<SYNCBLOCKINDEX_BITS)-1)



class EXT_CLASS : public ExtExtension
{
public:
	EXT_COMMAND_METHOD(DumpSyncBlk);
    EXT_COMMAND_METHOD(DumpObjHeader);
};

