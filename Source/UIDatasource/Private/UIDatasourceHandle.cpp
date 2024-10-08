// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasourceHandle.h"
#include "UIDatasource.h"
#include "UIDatasourceSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UIDatasourceHandle)

FUIDatasourceHandle::FUIDatasourceHandle(const FUIDatasource* Datasource)
	: Id(Datasource ? UIDatasource_PackId(Datasource->Generation, Datasource->Id) : 0)
{
}

FUIDatasource* FUIDatasourceHandle::Get() const
{
	FUIDatasourceGeneration Generation;
	EUIDatasourceId UnpackedId;
	UIDatasource_UnpackId(Id, Generation, UnpackedId);
	FUIDatasource* Datasource = UUIDatasourceSubsystem::Get()->Pool.GetDatasourceById(UnpackedId);
	return Datasource && Datasource->Generation == Generation ? Datasource : nullptr;
}

FUIDatasource& FUIDatasourceHandle::Get_Ref() const
{
	FUIDatasourceGeneration Generation;
	EUIDatasourceId UnpackedId;
	UIDatasource_UnpackId(Id, Generation, UnpackedId);
	FUIDatasource* Datasource = UUIDatasourceSubsystem::Get()->Pool.GetDatasourceById(UnpackedId);
	return Datasource && Datasource->Generation == Generation ? *Datasource : FUIDatasourcePool::SinkDatasource;
}

bool FUIDatasourceHandle::operator==(const FUIDatasourceHandle& Handle) const
{
	return Id == Handle.Id;
}

bool FUIDatasourceHandle::operator!=(const FUIDatasourceHandle& Handle) const
{
	return Id != Handle.Id;
}

bool FUIDatasourceHandle::ExportTextItem(FString& ValueStr, FUIDatasourceHandle const& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const
{
	if (FUIDatasource* Datasource = Get())
	{
		Datasource->GetPath(ValueStr);
	}
	else
	{
		ValueStr = TEXT("Invalid");
	}

	return true;
}
