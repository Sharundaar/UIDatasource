#pragma once
#include "UIDatasourceHandle.h"

struct FUIDatasourceLogEntry
{
	FUIDatasourceHandle Handle;
};

struct FUIDatasourceMonitor
{
	TArray<FUIDatasourceLogEntry> Logs;

	DECLARE_MULTICAST_DELEGATE(FMonitorEventHandler)
	FMonitorEventHandler OnMonitorEvent;
};
