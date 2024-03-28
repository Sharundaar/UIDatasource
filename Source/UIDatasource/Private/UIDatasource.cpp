// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasource.h"

#include "UIDatasourceSubsystem.h"

FUIDatasource* FUIDatasource::FindOrCreateFromPath(FWideStringView Path) { return GetPool()->FindOrCreateDatasource(this, Path); }
FUIDatasource* FUIDatasource::FindOrCreateFromPath(FAnsiStringView Path) { return GetPool()->FindOrCreateDatasource(this, Path); }
FUIDatasource* FUIDatasource::FindFromPath(FWideStringView Path) const { return GetPool()->FindDatasource(this, Path); }
FUIDatasource* FUIDatasource::FindFromPath(FAnsiStringView Path) const { return GetPool()->FindDatasource(this, Path); }

#define OPERATOR_IMPL(Type, Func, IS_CONST) \
FUIDatasource& FUIDatasource::operator[](Type Path) IS_CONST \
{ \
	FUIDatasource* Datasource = Func(Path); \
	checkf(Datasource, TEXT("Failed to create child datasource for path %s.%s."), *Name.ToString(), *FString(Path)); \
	return *Datasource; \
}

OPERATOR_IMPL(FWideStringView, FindOrCreateFromPath,);
OPERATOR_IMPL(FAnsiStringView, FindOrCreateFromPath,);
OPERATOR_IMPL(FWideStringView, FindFromPath, const);
OPERATOR_IMPL(FAnsiStringView, FindFromPath, const);

#undef OPERATOR_IMPL

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
