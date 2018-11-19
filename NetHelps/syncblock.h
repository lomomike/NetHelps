#pragma once

#include "stdafx.h"

struct SLink;
typedef struct SLink* PTR_SLink;

struct SLink
{
	PTR_SLink m_pNext;
	SLink()
	{
		m_pNext = NULL;
	}

	void InsertAfter(SLink* pLinkToInsert)
	{
		PTR_SLink pTemp = m_pNext;

		m_pNext = PTR_SLink(pLinkToInsert);
		pLinkToInsert->m_pNext = pTemp;
	}

	// find pLink within the list starting at pHead
	// if found remove the link from the list and return the link
	// otherwise return NULL
	static SLink* FindAndRemove(SLink *pHead, SLink* pLink, SLink ** ppPrior)
	{
		SLink* pFreeLink = NULL;
		*ppPrior = NULL;

		while (pHead->m_pNext != NULL)
		{
			if (pHead->m_pNext == pLink)
			{
				pFreeLink = pLink;
				pHead->m_pNext = pLink->m_pNext;
				*ppPrior = pHead;
				break;
			}
			pHead = pHead->m_pNext;
		}

		return pFreeLink;
	}
};

struct ClrEvent
{
	HANDLE m_handle;
	DWORD m_dwFlags;
};

static_assert(sizeof(HANDLE) == 8, "");
static_assert(sizeof(DWORD) == 4, "");

struct AwareLock
{

public:
	LONG  m_MonitorHeld;
	ULONG           m_Recursion;
	ULONG_PTR      m_HoldingThread;


	LONG            m_TransientPrecious;


	// This is a backpointer from the syncblock to the synctable entry.  This allows
	// us to recover the object that holds the syncblock.
	DWORD           m_dwSyncIndex;

	ClrEvent        m_SemEvent;


	enum EnterHelperResult {
		EnterHelperResult_Entered,
		EnterHelperResult_Contention,
		EnterHelperResult_UseSlowPath
	};

	enum LeaveHelperAction {
		LeaveHelperAction_None,
		LeaveHelperAction_Signal,
		LeaveHelperAction_Yield,
		LeaveHelperAction_Contention,
		LeaveHelperAction_Error,
	};


};

struct SyncBlock
{


	AwareLock  m_Monitor;                    // the actual monitor

public:
	// If this object is exposed to unmanaged code, we keep some extra info here.
	ULONG_PTR    m_pInteropInfo;



	// And if the object has new fields added via EnC, this is a list of them
	ULONG_PTR m_pEnCInfo;

	// We thread two different lists through this link.  When the SyncBlock is
	// active, we create a list of waiting threads here.  When the SyncBlock is
	// released (we recycle them), the SyncBlockCache maintains a free list of
	// SyncBlocks here.
	//
	// We can't afford to use an SList<> here because we only want to burn
	// space for the minimum, which is the pointer within an SLink.
	SLink       m_Link;

	// This is the index for the appdomain to which the object belongs. If we
	// can't set it in the object header, then we set it here. Note that an
	// object doesn't always have this filled in. Only for COM interop, 
	// finalizers and objects in handles
	DWORD m_dwAppDomainIndex;

	// This is the hash code for the object. It can either have been transfered
	// from the header dword, in which case it will be limited to 26 bits, or
	// have been generated right into this member variable here, when it will
	// be a full 32 bits.

	// A 0 in this variable means no hash code has been set yet - this saves having
	// another flag to express this state, and it enables us to use a 32-bit interlocked
	// operation to set the hash code, on the other hand it means that hash codes
	// can never be 0. ObjectNative::GetHashCode in COMObject.cpp makes sure to enforce this.
	DWORD m_dwHashCode;

#if CHECK_APP_DOMAIN_LEAKS 
	DWORD m_dwFlags;

	enum {
		IsObjectAppDomainAgile = 1,
		IsObjectCheckedForAppDomainAgile = 2,
	};
#endif
	// In some early version of VB when there were no arrays developers used to use BSTR as arrays
	// The way this was done was by adding a trail byte at the end of the BSTR
	// To support this scenario, we need to use the sync block for this special case and
	// save the trail character in here. 
	// This stores the trail character when a BSTR is used as an array
	WCHAR m_BSTRTrailByte;
	
public:


	enum
	{
		// This bit indicates that the syncblock is valuable and can neither be discarded
		// nor re-created.
		SyncBlockPrecious = 0x80000000,
	};


	void PrintData(IDebugControl* pDebugControl, int index) const
	{
		pDebugControl->Output(DEBUG_OUTPUT_NORMAL,
			"SyncBlock #%u\n"
			"\tMonitor\n"
			"\t\tMonitor held %llu\n"
			"\t\tRecursion %lu\n"
			"\t\tHolding Thread %p\n"
			"\t\tTransient Precious %u\n"
			"\t\tSync index %llu\n"
			"\t\tEvent %llx\n"
			"\tInteropInfo %p\n"
			"\tEnC Info %p\n"
			"\tThreads list head %p\n"
			"\tAppDomain index %lu\n"
			"\tHashCode %lx (%lu)\n\n",
			index,
			m_Monitor.m_MonitorHeld,
			m_Monitor.m_Recursion,
			m_Monitor.m_HoldingThread,
			m_Monitor.m_TransientPrecious,
			m_Monitor.m_dwSyncIndex,
			m_Monitor.m_SemEvent.m_handle,
			m_pInteropInfo,
			m_pEnCInfo,
			m_Link,
			m_dwAppDomainIndex,
			m_dwHashCode, m_dwHashCode);
	}

};

static_assert(sizeof(SyncBlock) == 80, "SyncBlock size should be 80 bytes");