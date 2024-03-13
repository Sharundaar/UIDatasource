// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasource.h"

#include "UIDatasourceSubsystem.h"

FUIDatasource* FUIDatasource::FindOrCreateFromPath(const FString& Path)
{
	return GetPool()->FindOrCreateDatasource(this, Path);
}

FUIDatasource* FUIDatasource::FindFromPath(const FString& Path) const
{
	return GetPool()->FindDatasource(this, Path);
}

FUIDatasource& FUIDatasource::operator[](const FString& Path)
{
	FUIDatasource* Datasource = FindOrCreateFromPath(Path);
	checkf(Datasource, TEXT("Failed to create child datasource for path %s.%s."), *Name.ToString(), *Path);
	return *Datasource;
}

FUIDatasource& FUIDatasource::operator[](const FString& Path) const
{
	FUIDatasource* Datasource = FindFromPath(Path);
	checkf(Datasource, TEXT("Failed to find child datasource for path %s.%s."), *Name.ToString(), *Path);
	return *Datasource;
}

void FUIDatasource::OnValueChanged() const
{
	UIDATASOURCE_FUNC_TRACE()
	
	OnDatasourceChanged.Broadcast({
		EUIDatasourceChangeEventKind::ValueSet,
		this
	});
}

FUIDatasourcePool* FUIDatasource::GetPool() const
{
	return reinterpret_cast<const FUIDatasourceHeader*>(this - static_cast<int>(Id))->Pool;
}
