// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasourceBlueprintLibrary.h"

#include "UIDatasource.h"
#include "UIDatasourceSubsystem.h"

FUIDatasourceHandle UUIDatasourceBlueprintLibrary::FindOrCreateDatasource(FUIDatasourceHandle Parent, FString Path)
{
	return UUIDatasourceSubsystem::Get()->Pool.FindOrCreateDatasource(Parent.Get(), *Path);
}

bool UUIDatasourceBlueprintLibrary::IsValid(FUIDatasourceHandle Handle)
{
	return Handle.IsValid();
}

bool UUIDatasourceBlueprintLibrary::EqualEqual_DatasourceHandle(FUIDatasourceHandle HandleA, FUIDatasourceHandle HandleB)
{
	return HandleA == HandleB;
}

bool UUIDatasourceBlueprintLibrary::ArrayDatasource_IsArray(FUIDatasourceHandle ArrayHandle)
{
	return FUIArrayDatasource::Cast(ArrayHandle.Get()) != nullptr;
}

int32 UUIDatasourceBlueprintLibrary::ArrayDatasource_GetNum(FUIDatasourceHandle ArrayHandle)
{
	if(const FUIArrayDatasource* ArrayDatasource = FUIArrayDatasource::Cast(ArrayHandle.Get()))
	{
		return ArrayDatasource->GetNum();
	}
	return 0;
}

FUIDatasourceHandle UUIDatasourceBlueprintLibrary::ArrayDatasource_GetChildAt(FUIDatasourceHandle ArrayHandle, int32 Index)
{
	if(const FUIArrayDatasource* ArrayDatasource = FUIArrayDatasource::Cast(ArrayHandle.Get()))
	{
		return ArrayDatasource->GetChildAt(Index);
	}
	return {};
}

FUIDatasourceHandle UUIDatasourceBlueprintLibrary::ArrayDatasource_Append(FUIDatasourceHandle ArrayHandle)
{
	if(FUIArrayDatasource* ArrayDatasource = FUIArrayDatasource::Cast(ArrayHandle.Get()))
	{
		return ArrayDatasource->Append();
	}
	return {};
}

template<typename T>
T GetDatasourceValue(FUIDatasourceHandle Handle)
{
	T ReturnValue = {};
	if(FUIDatasource* Datasource = Handle.Get())
	{
		Datasource->TryGet(ReturnValue);
	}
	return ReturnValue;
}

uint8 UUIDatasourceBlueprintLibrary::GetIntAsByte(FUIDatasourceHandle Handle)
{
	return static_cast<uint8>(GetInt(Handle));
}

bool UUIDatasourceBlueprintLibrary::SetIntAsByte(FUIDatasourceHandle Handle, uint8 Value)
{
	return SetInt(Handle, Value);
}

#define IMPL_LIB_FUNC(Type, Name)											\
Type UUIDatasourceBlueprintLibrary::Get##Name(FUIDatasourceHandle Handle)	\
{																			\
	return GetDatasourceValue<Type>(Handle);								\
}																			\
bool UUIDatasourceBlueprintLibrary::Set##Name(FUIDatasourceHandle Handle, Type Value) \
{																			\
	if(FUIDatasource* Datasource = Handle.Get())							\
	{																		\
		return Datasource->Set(Value);										\
	}																		\
	return false;															\
}

IMPL_LIB_FUNC(int32,		Int);
IMPL_LIB_FUNC(float,		Float);
IMPL_LIB_FUNC(bool,			Bool);
IMPL_LIB_FUNC(FString,		String);
IMPL_LIB_FUNC(FName,		FName);
IMPL_LIB_FUNC(FText,		Text);
IMPL_LIB_FUNC(TSoftObjectPtr<UTexture2D>, Image)
IMPL_LIB_FUNC(FGameplayTag, GameplayTag)
#undef IMPL_LIB_FUNC
