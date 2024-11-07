#pragma once

#include "UIDatasourceDefines.h"
#include "UIDatasource.h"
#include "UIDatasourceUserWidgetExtension.h"

struct FUIDatasourceLogEntry
{
	FUIDatasourceHandle Handle;
};

struct FUIDatasourceMonitor
{
	TArray<FUIDatasourceLogEntry> Logs;
	TArray<FUIDatasourceChangeEventArgs> QueuedEvents;
	TArray<FUIDatasourceChangeEventArgs> QueuedEventsBuffer;
	TMap<FUIDatasourceHandle, FOnDatasourceChangedDelegate> EventHandlers;

	bool bProcessingEvents = false;
	bool bCleanupDelegates = false;

	void QueueDatasourceEvent(FUIDatasourceChangeEventArgs Event);
	void BindDatasourceEvent(FUIDatasourceHandle Handle, const FOnDatasourceChangedDelegateBP& Delegate);
	void UnbindDatasourceEvent(FUIDatasourceHandle Handle, const FOnDatasourceChangedDelegateBP& Delegate);
	void ProcessEvents();
	void Clear();

	DECLARE_MULTICAST_DELEGATE(FMonitorEventHandler)
	FMonitorEventHandler OnMonitorEvent;
};
