// asd.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "nethelps.h"

// EXT_DECLARE_GLOBALS must be used to instantiate
// the framework's assumed globals.
EXT_DECLARE_GLOBALS();

EXT_COMMAND(DumpSyncBlk,
    "Dump SynkBlock",
    "{;en=(10);index;Sync Block Index}")
{
    const auto syncBlockIndex = static_cast<unsigned int>(GetUnnamedArgU64(0));

    WDBGEXTS_CLR_DATA_INTERFACE Query;
    Query.Iid = &__uuidof(IXCLRDataProcess);

    WINDBG_EXTENSION_APIS64 ExtensionApis;
    ExtensionApis.nSize = sizeof(ExtensionApis);
    m_Control->GetWindbgExtensionApis64(&ExtensionApis);

    if (!ExtensionApis.lpIoctlRoutine(IG_GET_CLR_DATA_INTERFACE, &Query, sizeof(Query)))
    {
        return;
    }

    auto clrData = static_cast<IXCLRDataProcess *>(Query.Iface);

    ISOSDacInterface *g_sos = nullptr;
    auto hr = clrData->QueryInterface(__uuidof(ISOSDacInterface), reinterpret_cast<void**>(&g_sos));
    if (FAILED(hr))
    {
        g_sos = nullptr;
        return;
    }

    DacpSyncBlockData syncBlockData;
    g_sos->GetSyncBlockData(syncBlockIndex, &syncBlockData);
    Out("SyncBlock pointer %p\n", syncBlockData.SyncBlockPointer);

    SyncBlock syncBlock;

    hr = m_Data->ReadVirtual(syncBlockData.SyncBlockPointer, static_cast<PVOID>(&syncBlock), sizeof(SyncBlock), nullptr);
    if (FAILED(hr))
        return;

    syncBlock.PrintData(m_Control, syncBlockIndex);
}

EXT_COMMAND(DumpObjHeader,
    "Dump Object Header",
    "{;e,r;address;Object Address}")
{
    const CLRDATA_ADDRESS objectAddress = GetUnnamedArgU64(0);
    ObjHeader objectHeader;

    auto hr = m_Data->ReadVirtual(objectAddress - 8, &objectHeader, sizeof(ObjHeader), nullptr);
    if (FAILED(hr))
        return;

    const auto headerValue = objectHeader.m_SyncBlockValue;
    m_Control->Output(DEBUG_OUTPUT_NORMAL,
        "ObjectHeader %08x. ", headerValue);

    if (headerValue == 0)
    {
        m_Control->Output(DEBUG_OUTPUT_NORMAL, "Empty\n\n");
        return;
    }
    m_Control->Output(DEBUG_OUTPUT_NORMAL, "\n");


    if (headerValue & BIT_SBLK_IS_HASH_OR_SYNCBLKINDEX)
    {
        if (headerValue & BIT_SBLK_IS_HASHCODE)
        {
            const auto hashCode = headerValue & MASK_HASHCODE;
            m_Control->Output(DEBUG_OUTPUT_NORMAL, "Stores only HashCode %08x\n", hashCode);
        }
        else
        {
            const auto syncBlockIndex = headerValue & MASK_SYNCBLOCKINDEX;
            m_Control->Output(DEBUG_OUTPUT_NORMAL, "Stores SyncBlock #%u\n\n", syncBlockIndex);

        }
    }
    else
    {
        const auto lockThreadId = headerValue & SBLK_MASK_LOCK_THREADID;
        const auto recursionLevel = (headerValue & SBLK_MASK_LOCK_RECLEVEL) >> SBLK_RECLEVEL_SHIFT;
        const auto appDomainIndex = (headerValue & SBLK_MASK_APPDOMAININDEX) >> SBLK_APPDOMAIN_SHIFT;

        m_Control->Output(DEBUG_OUTPUT_NORMAL,
            "Stores Thinlock\n"
            "\tThreadId %lu\n"
            "\tRecursion level %lu\n"
            "\tAppDomain index %lu\n",
            lockThreadId, recursionLevel, appDomainIndex);
    }

}