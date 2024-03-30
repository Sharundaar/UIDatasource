// Copyright Sharundaar. All Rights Reserved.

#pragma once

#include "UIDatasourceDefines.h"
#include "UIDatasourceHandle.generated.h"

struct FUIDatasource;

USTRUCT(BlueprintType)
struct UIDATASOURCE_API FUIDatasourceHandle
{
	GENERATED_BODY()

	FUIDatasourceHandle() : Id(UIDatasource_PackId(0, EUIDatasourceId::Invalid)) {};
	FUIDatasourceHandle(const FUIDatasource* Datasource);

	bool IsValid() const { return Id & UIDATASOURCE_ID_MASK; }
	FUIDatasource* Get() const;
	FUIDatasource& Get_Ref() const;
	bool operator==(const FUIDatasourceHandle& Handle) const;
	bool operator!=(const FUIDatasourceHandle& Handle) const;

	FUIDatasourcePackedId Id;
};

inline uint32 GetTypeHash(const FUIDatasourceHandle& Handle)
{
	return GetTypeHash(Handle.Id);
}
