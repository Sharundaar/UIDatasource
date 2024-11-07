#include "UIDatasourceMonitor.h"

namespace FUIDatasourceMonitor_Local
{
	static TAutoConsoleVariable<bool> CVarProcessEventsImmediate(
	TEXT("UIDatasource.Monitor.ProcessEventsImmediate"),
	false,
	TEXT("If enabled, process all queued events immediately instead of waiting for tick update."),
	ECVF_Cheat);

}

void FUIDatasourceMonitor::QueueDatasourceEvent(FUIDatasourceChangeEventArgs Event)
{
	UIDATASOURCE_FUNC_TRACE()

	if(!FUIDatasourceMonitor_Local::CVarProcessEventsImmediate.GetValueOnAnyThread())
	{
		if (!QueuedEvents.Contains(Event))
		{
			QueuedEvents.Add(Event);
		}
	}
	else
	{
		if(const FOnDatasourceChangedDelegate* Delegates = EventHandlers.Find(Event.Handle))
		{
			Delegates->Broadcast(Event);
		}
	}
}

void FUIDatasourceMonitor::BindDatasourceEvent(FUIDatasourceHandle Handle, const FOnDatasourceChangedDelegateBP& Delegate)
{
	UIDATASOURCE_FUNC_TRACE()
	
	EventHandlers.FindOrAdd(Handle).AddUnique(Delegate);
}

void FUIDatasourceMonitor::UnbindDatasourceEvent(FUIDatasourceHandle Handle, const FOnDatasourceChangedDelegateBP& Delegate)
{
	if (FOnDatasourceChangedDelegate* Delegates = EventHandlers.Find(Handle))
	{
		Delegates->Remove(Delegate);
		if (!Delegates->IsBound())
		{
			bCleanupDelegates = true;
		}
	}
}

void FUIDatasourceMonitor::ProcessEvents()
{
	UIDATASOURCE_FUNC_TRACE()
	bProcessingEvents = true;
	QueuedEventsBuffer = QueuedEvents;
	QueuedEvents.Reset();
	for (const FUIDatasourceChangeEventArgs& Event : QueuedEventsBuffer)
	{
		if(const FOnDatasourceChangedDelegate* Delegates = EventHandlers.Find(Event.Handle))
		{
			// @TODO: Evaluate performance impact of this copy here, broadcasting this event might
			// trigger some widgets to bind to some new datasources, this would potentially change EventHandlers size
			// which means reallocating the underlying delegates, which we hold a pointer to in this scope
			// making a local copy before broadcasting ensures we keep it valid.
			FOnDatasourceChangedDelegate TmpDelegates = *Delegates;
			TmpDelegates.Broadcast(Event);
		}
	}

	if (bCleanupDelegates)
	{
		for(auto It = EventHandlers.CreateIterator(); It; ++It)
		{
			if(!It->Value.IsBound())
			{
				It.RemoveCurrent();
			}
		}
		bCleanupDelegates = false;
	}
	bProcessingEvents = false;
}

void FUIDatasourceMonitor::Clear()
{
	QueuedEvents.Empty();
	EventHandlers.Empty();
}
