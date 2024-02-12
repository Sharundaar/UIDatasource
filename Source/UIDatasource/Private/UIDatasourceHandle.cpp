// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasourceHandle.h"
#include "UIDatasource.h"
#include "UIDatasourceSubsystem.h"

FUIDatasourceHandle::FUIDatasourceHandle(const FUIDatasource* Datasource)
	: Id(UIDatasource_PackId(Datasource->Generation, Datasource->Id))
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
