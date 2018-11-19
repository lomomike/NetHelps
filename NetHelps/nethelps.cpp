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