// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasourceBlueprintLibrary.h"

#include "UIDatasource.h"
#include "UIDatasourceSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UIDatasourceBlueprintLibrary)

FUIDatasourceHandle UUIDatasourceBlueprintLibrary::FindOrCreateDatasource(FUIDatasourceHandle Parent, FString Path)
{
	return UUIDatasourceSubsystem::Get()->Pool.FindOrCreateDatasource(Parent.Get(), *Path);
}

FUIDatasourceHandle UUIDatasourceBlueprintLibrary::FindDatasource(FUIDatasourceHandle Parent, FString Path)
{
	return UUIDatasourceSubsystem::Get()->Pool.FindDatasource(Parent.Get(), *Path);
}

FUIDatasourceHandle UUIDatasourceBlueprintLibrary::FindChildDatasource(FUIDatasourceHandle ParentHandle, FName ChildName)
{
	if(FUIDatasource* Parent = ParentHandle.Get())
	{
		return UUIDatasourceSubsystem::Get()->Pool.FindChildDatasource(Parent, ChildName);
	}
	return {};
}

FUIDatasourceHandle UUIDatasourceBlueprintLibrary::FindOrCreateChildDatasource(FUIDatasourceHandle ParentHandle, FName ChildName)
{
	if(FUIDatasource* Parent = ParentHandle.Get())
	{
		return UUIDatasourceSubsystem::Get()->Pool.FindOrCreateChildDatasource(Parent, ChildName);
	}
	return {};
}

bool UUIDatasourceBlueprintLibrary::IsValid(FUIDatasourceHandle Handle)
{
	return Handle.IsValid();
}

bool UUIDatasourceBlueprintLibrary::EqualEqual_DatasourceHandle(FUIDatasourceHandle HandleA, FUIDatasourceHandle HandleB)
{
	return HandleA == HandleB;
}

FUIDatasourceHandle UUIDatasourceBlueprintLibrary::ArrayDatasource_MakeArray(FUIDatasourceHandle Handle)
{
	if(FUIDatasource* Datasource = Handle.Get())
	{
		return FUIArrayDatasource::Make(Datasource);
	}
	return {};
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

TArray<FUIDatasourceHandle> UUIDatasourceBlueprintLibrary::ArrayDatasource_ToArray(FUIDatasourceHandle ArrayHandle, int32 PadArrayUpTo, int32 KeepNumMultipleOf)
{
	TArray<FUIDatasourceHandle> Children;
	
	if (FUIArrayDatasource* ArrayDatasource = FUIArrayDatasource::Cast(ArrayHandle.Get()))
	{
		const int32 Num = ArrayDatasource->GetNum();
		for (int32 Index=0; Index < Num; ++Index)
		{
			Children.Add(ArrayDatasource->GetChildAt(Index));
		}

		for (int Index=Num; Index < PadArrayUpTo; ++Index)
		{
			Children.Add(FUIDatasourceHandle{});
		}

		if (KeepNumMultipleOf > 0)
		{
			for(int Index=0; Index < Children.Num() % KeepNumMultipleOf; ++Index)
			{
				Children.Add(FUIDatasourceHandle{});
			}
		}
	}

	return Children;
}

TArray<UUIDatasourceWrapper*> UUIDatasourceBlueprintLibrary::ArrayDatasource_ToObjectArray(FUIDatasourceHandle ArrayHandle, int32 PadArrayUpTo, int32 KeepNumMultipleOf)
{
	TArray<UUIDatasourceWrapper*> Children;
	
	if(FUIArrayDatasource* ArrayDatasource = FUIArrayDatasource::Cast(ArrayHandle.Get()))
	{
		const int32 Num = ArrayDatasource->GetNum();
		for(int32 Index=0; Index < Num; ++Index)
		{
			UUIDatasourceWrapper* Wrapper = NewObject<UUIDatasourceWrapper>();
			Wrapper->SetHandle(ArrayDatasource->GetChildAt(Index));
			Children.Add(Wrapper);
		}

		for (int Index=Num; Index < PadArrayUpTo; ++Index)
		{
			Children.Add(NewObject<UUIDatasourceWrapper>());
		}

		if (KeepNumMultipleOf > 0)
		{
			for(int Index=0; Index < Children.Num() % KeepNumMultipleOf; ++Index)
			{
				Children.Add(NewObject<UUIDatasourceWrapper>());
			}
		}
	}

	return Children;
}

FUIDatasourceHandle UUIDatasourceBlueprintLibrary::ArrayDatasource_GetChildAt(FUIDatasourceHandle ArrayHandle, int32 Index)
{
	if(const FUIArrayDatasource* ArrayDatasource = FUIArrayDatasource::Cast(ArrayHandle.Get()))
	{
		return 0 <= Index && Index < ArrayDatasource->GetNum() ? ArrayDatasource->GetChildAt(Index) : nullptr;
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

FUIDatasourceHandle UUIDatasourceBlueprintLibrary::ArrayDatasource_AppendFront(FUIDatasourceHandle ArrayHandle)
{
	if(FUIArrayDatasource* ArrayDatasource = FUIArrayDatasource::Cast(ArrayHandle.Get()))
	{
		return ArrayDatasource->AppendFront();
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
IMPL_LIB_FUNC(FUIDatasourceImage, Image)

IMPL_LIB_FUNC(FGameplayTag, GameplayTag)
IMPL_LIB_FUNC(FInstancedStruct, Struct)

#undef IMPL_LIB_FUNC

TSoftObjectPtr<UTexture2D> UUIDatasourceBlueprintLibrary::GetTexture(FUIDatasourceHandle Handle)
{
	return GetDatasourceValue<FUIDatasourceImage>(Handle).AsTexture();
}

TSoftObjectPtr<UMaterialInterface> UUIDatasourceBlueprintLibrary::GetMaterial(FUIDatasourceHandle Handle)
{
	return GetDatasourceValue<FUIDatasourceImage>(Handle).AsMaterial();
}

bool UUIDatasourceBlueprintLibrary::SetTexture(FUIDatasourceHandle Handle, TSoftObjectPtr<UTexture2D> Value)
{
	if(FUIDatasource* Datasource = Handle.Get())
	{
		return Datasource->Set(FUIDatasourceImage(Value));
	}
	return false;
}

bool UUIDatasourceBlueprintLibrary::SetMaterial(FUIDatasourceHandle Handle, TSoftObjectPtr<UMaterialInterface> Value)
{
	if(FUIDatasource* Datasource = Handle.Get())
	{
		return Datasource->Set(FUIDatasourceImage(Value));
	}
	return false;
}
