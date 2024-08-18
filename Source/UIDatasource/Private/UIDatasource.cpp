// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasource.h"
#include "UIDatasourceSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UIDatasource)

namespace UIDatasourceHelpers
{
	FName GetDisplayName(FName Base, int32 Index)
	{
		Base.SetNumber(Index + 1); // FName numbers starts at 1, so the first item should have Number == 1 (which will display as Item_0, oh well...)
		return Base;
	}
}

FUIDatasource* FUIDatasource::FindOrCreateFromPath(FWideStringView Path) { return GetPool()->FindOrCreateDatasource(this, Path); }
FUIDatasource* FUIDatasource::FindOrCreateFromPath(FAnsiStringView Path) { return GetPool()->FindOrCreateDatasource(this, Path); }
FUIDatasource* FUIDatasource::FindFromPath(FWideStringView Path) const { return GetPool()->FindDatasource(this, Path); }
FUIDatasource* FUIDatasource::FindFromPath(FAnsiStringView Path) const { return GetPool()->FindDatasource(this, Path); }

void FUIDatasource::GetPath(FString& OutPath)
{
	EUIDatasourceId Chain[64];
	constexpr int32 ChainBufferCapacity = UE_ARRAY_COUNT(Chain);
	FUIDatasourcePool* Pool = GetPool();
	int32 ChainLength = 0;
	FUIDatasource* It = this;
	while(It)
	{
		Chain[ChainBufferCapacity - ChainLength - 1] = It->Id;
		ChainLength++;
		It = Pool->GetDatasourceById(It->Parent);
	}
	
	OutPath.Reset();
	while(ChainLength)
	{
		Pool->GetDatasourceById(Chain[ChainBufferCapacity - ChainLength])->Name.AppendString(OutPath);
		ChainLength--;
		if(ChainLength > 0)
		{
			OutPath += TEXT('.');
		}
	}
}

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

#if WITH_UIDATASOURCE_MONITOR
	UUIDatasourceSubsystem::Get()->Monitor.QueueDatasourceEvent({
		EUIDatasourceChangeEventKind::ValueSet,
		this
	});
#else
	OnDatasourceChanged.Broadcast({
		EUIDatasourceChangeEventKind::ValueSet,
		this
	});
#endif
}

FUIDatasourcePool* FUIDatasource::GetPool() const
{
	return reinterpret_cast<const FUIDatasourceHeader*>(this - static_cast<int>(Id))->Pool;
}

const FName FUIArrayDatasource::ItemBaseName = "Item";

FUIDatasource* FUIArrayDatasource::Append()
{
	const int32 Num = GetNum();
	const FName ChildName = UIDatasourceHelpers::GetDisplayName(ItemBaseName, Num);
	if(FUIDatasource* Child = GetPool()->FindOrCreateChildDatasource(this, ChildName))
	{
		Set(Num + 1);
		return Child;
	}
	return nullptr; // @NOTE: Pool is full...
}

FUIDatasource* FUIArrayDatasource::AppendFront()
{
	FUIDatasourcePool* Pool = GetPool();
	const int32 Num = GetNum();
	const FName ChildName = UIDatasourceHelpers::GetDisplayName(ItemBaseName, Num);
	if(FUIDatasource* Child = Pool->FindOrCreateChildDatasource(this, ChildName))
	{
		for(FUIDatasource* ChildIt = Pool->GetDatasourceById(FirstChild); ChildIt; ChildIt = Pool->GetDatasourceById(ChildIt->NextSibling))
		{
			if (ChildIt->Name.IsEqual(ItemBaseName, ENameCase::IgnoreCase, false))
			{
				ChildIt->Name.SetNumber(ChildIt->Name.GetNumber()+1);
			}
		}
		Child->Name = UIDatasourceHelpers::GetDisplayName(ItemBaseName, 0);
		
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

FUIDatasource* FUIArrayDatasource::GetChildAt(int32 Index) const
{
	ensureMsgf(0 <= Index && Index < GetNum(), TEXT("Tried to access out of range datasource index (%d/%d)."), Index, GetNum());
	const FName ChildName = UIDatasourceHelpers::GetDisplayName(ItemBaseName, Index);
	return GetPool()->FindChildDatasource(this, ChildName);
}

FUIArrayDatasource* FUIArrayDatasource::Make(FUIDatasource* Datasource, bool bDestroyChildren)
{
	if (!Datasource)
	{
		return nullptr;
	}
	
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

