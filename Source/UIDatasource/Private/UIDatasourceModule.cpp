// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasourceModule.h"
#include "UIDatasource.h"

DEFINE_LOG_CATEGORY(LogDatasource);
UE_TRACE_CHANNEL_DEFINE(UIDatasourceTraceChannel);

#define LOCTEXT_NAMESPACE "FUIDatasourceModule"

void FUIDatasourceModule::StartupModule()
{
}

void FUIDatasourceModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUIDatasourceModule, UIDatasource)