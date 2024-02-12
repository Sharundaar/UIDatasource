// Copyright Sharundaar. All Rights Reserved.

#pragma once

UIDATASOURCE_API DECLARE_LOG_CATEGORY_EXTERN(LogDatasource, Log, All);

#define WITH_UIDATASOURCE_DEBUG 1
#define WITH_UIDATASOURCE_TRACE UE_TRACE_ENABLED

#if WITH_UIDATASOURCE_TRACE
#include "ProfilingDebugging/CpuProfilerTrace.h"
UE_TRACE_CHANNEL_EXTERN(UIDatasourceTraceChannel, UIDATASOURCE_API)
#define UIDATASOURCE_TRACE(TraceName) TRACE_CPUPROFILER_EVENT_SCOPE_ON_CHANNEL(TraceName, UIDatasourceTraceChannel)
#define UIDATASOURCE_FUNC_TRACE() TRACE_CPUPROFILER_EVENT_SCOPE_ON_CHANNEL(__func__, UIDatasourceTraceChannel)
#else
#define UIDATASOURCE_TRACE(TraceName)
#define UIDATASOURCE_FUNC_TRACE()
#endif

#define MAX_DATASOURCE_ID				0xFFFF
#define UIDATASOURCE_ID_MASK			0x0000FFFF
#define UIDATASOURCE_GENERATION_MASK	0xFFFF0000
#define UIDATASOURCE_GENERATION_OFFSET	16

using FUIDatasourceGeneration = uint16;
enum class EUIDatasourceId : uint16
{
	Invalid = 0,
	Header  = 0,
	Root    = 1,
};
constexpr uint16 ToIndex(EUIDatasourceId Id) { return static_cast<uint16>(Id); }
using FUIDatasourcePackedId = uint32; // Generation + Id

constexpr FUIDatasourcePackedId UIDatasource_PackId(const FUIDatasourceGeneration Generation, const EUIDatasourceId Id)
{
	return (static_cast<FUIDatasourcePackedId>(Generation) << UIDATASOURCE_GENERATION_OFFSET) | static_cast<FUIDatasourcePackedId>(Id);
}

constexpr void UIDatasource_UnpackId(const FUIDatasourcePackedId PackedId, FUIDatasourceGeneration& OutGeneration, EUIDatasourceId& OutId)
{
	OutGeneration = static_cast<FUIDatasourceGeneration>(PackedId >> UIDATASOURCE_GENERATION_OFFSET);
	OutId = static_cast<EUIDatasourceId>(PackedId & UIDATASOURCE_ID_MASK);
}

