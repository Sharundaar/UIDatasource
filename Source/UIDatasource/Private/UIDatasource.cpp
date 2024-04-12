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

OPERATOR_IMPL(FWideStringView, FindOrCreateFromPath,)
OPERATOR_IMPL(FAnsiStringView, FindOrCreateFromPath,)
OPERATOR_IMPL(FWideStringView, FindFromPath, const)
OPERATOR_IMPL(FAnsiStringView, FindFromPath, const)

#undef OPERATOR_IMPL

void FUIDatasource::OnValueChanged() const
{
	UIDATASOURCE_FUNC_TRACE()

	// UUIDatasourceSubsystem::Get()->Monitor.Log()
	OnDatasourceChanged.Broadcast({
		EUIDatasourceChangeEventKind::ValueSet,
		this
	});
}

FUIDatasourcePool* FUIDatasource::GetPool() const
{
	return reinterpret_cast<const FUIDatasourceHeader*>(this - static_cast<int>(Id))->Pool;
}

const FName FUIArrayDatasource::ItemBaseName = "Item";

FUIDatasource* FUIArrayDatasource::Append()
{
	FName ChildName = ItemBaseName;
	const int32 Num = GetNum();
	ChildName.SetNumber(Num + 1); // FName numbers starts at 1, so the first item should have Number == 1 (which will display as Item_0, oh well...)
	if(FUIDatasource* Child = GetPool()->FindOrCreateChildDatasource(this, ChildName))
	{
		Set(Num + 1);
		return Child;
	}
	return nullptr; // @NOTE: Pool is full...
}

void FUIArrayDatasource::Empty(bool bDestroyChildren)
{
	if(bDestroyChildren)
	{
		FUIDatasourcePool* Pool = GetPool();
		for(FUIDatasource* Child = Pool->GetDatasourceById(FirstChild); Child; Child = Pool->GetDatasourceById(Child->NextSibling))
		{
			Pool->DestroyDatasource(Child);
		}
	}
	Set<int32>(0);
}

FUIArrayDatasource* FUIArrayDatasource::Make(FUIDatasource* Datasource, bool bDestroyChildren)
{
	check(Datasource);
	EnumAddFlags(Datasource->Flags, EUIDatasourceFlag::IsArray);
	FUIArrayDatasource* Array = static_cast<FUIArrayDatasource*>(Datasource);
	Array->Empty(bDestroyChildren);
	return Array;
}

FUIArrayDatasource& FUIArrayDatasource::Make(FUIDatasource& Datasource, bool bDestroyChildren)
{
	EnumAddFlags(Datasource.Flags, EUIDatasourceFlag::IsArray);
	FUIArrayDatasource& Array = static_cast<FUIArrayDatasource&>(Datasource);
	Array.Empty(bDestroyChildren);
	return Array;
}

