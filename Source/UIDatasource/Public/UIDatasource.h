// Copyright Sharundaar. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "UIDatasourceDefines.h"
#include "UIDatasourceHandle.h"

#include "UIDatasource.generated.h"

struct FUIDatasourcePool;
struct FUIDatasourceHeader;
struct FUIDatasource;
class FTexture2D;

UENUM()
enum class EUIDatasourceValueType : uint8
{
	Void,
	Int,
	Enum, // @NOTE: This is for archetype UX purposes, stored as Int at runtime
	Float,
	Bool,
	Name,
	Text,
	String,
	Image,
	GameplayTag,
	Archetype, // @NOTE: This is for archetype UX purposes, devolves to Void at runtime
};

struct FUIDatasourceValue
{
	struct FVoidType {};
	
	using FValueType = TVariant<FVoidType, int32, float, bool, FName, FText, FString, TSoftObjectPtr<UTexture2D>, FGameplayTag>;

	FValueType Value;

	void Clear()
	{
		Value.Set<FVoidType>({});
	}

	EUIDatasourceValueType GetType() const
	{
		if(Value.IsType<int32>())		return EUIDatasourceValueType::Int;
		if(Value.IsType<float>())		return EUIDatasourceValueType::Float;
		if(Value.IsType<bool>())		return EUIDatasourceValueType::Bool;
		if(Value.IsType<FName>())		return EUIDatasourceValueType::Name;
		if(Value.IsType<FText>())		return EUIDatasourceValueType::Text;
		if(Value.IsType<FString>())		return EUIDatasourceValueType::String;
		if(Value.IsType<TSoftObjectPtr<UTexture2D>>()) return EUIDatasourceValueType::Image;
		if(Value.IsType<FGameplayTag>()) return EUIDatasourceValueType::GameplayTag;
		return EUIDatasourceValueType::Void;
	}
	
	template<typename T>
	bool ValueEqual(const T& Val) const
	{
		UIDATASOURCE_FUNC_TRACE()

		if constexpr (std::is_same<FText, T>())
		{
			return Value.Get<T>().IdenticalTo(Val);
		}
		else
		{
			return Value.Get<T>() == Val;
		}
	}

	template<typename T>
	bool Set(const T& NewValue)
	{
		UIDATASOURCE_FUNC_TRACE()

		static_assert(!std::is_same<FVoidType, T>(), "Cannot set value to FVoidType, use Clear instead.");
		static_assert(FValueType::IndexOfType<T>() != static_cast<SIZE_T>(-1), "Tried to set datasource to invalid type.");

		if(Value.IsType<T>())
		{
			if(!ValueEqual<T>(NewValue))
			{
				Value.Set<T>(NewValue);
				return true;
			}
			return false;
		}
		
		if(Value.IsType<FVoidType>())
		{
			Value.Set<T>(NewValue);
			return true;
		}
		
		UE_LOG(LogDatasource, Warning, TEXT("Tried to assign a value of incompatible type in datasource value (Found %llu, Expected %llu)"), FValueType::IndexOfType<T>(), Value.GetIndex());
		return false;
	}

	template<typename T>
	T Get() const
	{
		UIDATASOURCE_FUNC_TRACE()

		static_assert(FValueType::IndexOfType<T>() != static_cast<SIZE_T>(-1), "Tried to get datasource to invalid type.");
		
		if(Value.IsType<T>())
		{
			return Value.Get<T>();
		}

		UE_LOG(LogDatasource, Warning, TEXT("Tried to get a value of incompatible type (Found %llu, Expected %llu), returning default"), FValueType::IndexOfType<T>(), Value.GetIndex());
		return {};
	}
	
	template<typename T>
	bool TryGet(T& OutValue) const
	{
		UIDATASOURCE_FUNC_TRACE()

		if(Value.IsType<T>())
		{
			OutValue = Value.Get<T>();
			return true;
		}

		return false;
	}
};

struct FUIDatasourceHeader
{
	FUIDatasourcePool* Pool;
};

enum class EUIDatasourceFlag : uint8
{
	None      = 0,
	IsInvalid = 1 << 0,
	IsArray   = 1 << 1,
};
ENUM_CLASS_FLAGS(EUIDatasourceFlag)

UENUM(BlueprintType)
enum class EUIDatasourceChangeEventKind : uint8
{
	InitialBind,
	ValueSet,
};

USTRUCT(BlueprintType)
struct FUIDatasourceChangeEventArgs
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	EUIDatasourceChangeEventKind Kind = EUIDatasourceChangeEventKind::InitialBind;

	UPROPERTY(BlueprintReadOnly)
	FUIDatasourceHandle Handle = {};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDatasourceChangedDelegate, FUIDatasourceChangeEventArgs, EventArgs);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDatasourceChangedDelegateBP, FUIDatasourceChangeEventArgs, EventArgs);

struct UIDATASOURCE_API FUIDatasource
{
	FName Name;
	
	FUIDatasourceGeneration Generation;
	EUIDatasourceId Id;

	EUIDatasourceId Parent;
	EUIDatasourceId FirstChild;
	EUIDatasourceId NextSibling;
	EUIDatasourceId PrevSibling;

	EUIDatasourceFlag Flags;
	FUIDatasourceValue Value;
	FOnDatasourceChangedDelegate OnDatasourceChanged;

	FUIDatasourcePool* GetPool() const;

	void OnValueChanged() const;

	FUIDatasource* FindOrCreateFromPath(FWideStringView Path);
	FUIDatasource* FindOrCreateFromPath(FAnsiStringView Path);
	FUIDatasource* FindFromPath(FWideStringView Path) const;
	FUIDatasource* FindFromPath(FAnsiStringView Path) const;
	
	template<typename T>
	bool Set(T InValue)
	{
		if(Value.Set<T>(InValue))
		{
			OnValueChanged();
			return true;
		}
		return false;
	}

	template<typename T>
	T Get() const
	{
		return Value.Get<T>();
	}

	template<typename T>
	bool TryGet(T& OutValue) const
	{
		return Value.TryGet<T>(OutValue);
	}

	FUIDatasource& operator[](FWideStringView Path);
	FUIDatasource& operator[](FAnsiStringView Path);
	FUIDatasource& operator[](FWideStringView Path) const;
	FUIDatasource& operator[](FAnsiStringView Path) const;
};
static_assert(sizeof(FUIDatasourceHeader) <= sizeof(FUIDatasource), "We need to be able to fit a Header in the space of a normal Datasource.");

struct UIDATASOURCE_API FUIArrayDatasource : FUIDatasource
{
	FUIArrayDatasource() = delete;
	
	int32 GetNum() const { return Get<int32>(); }
	FUIDatasource* Append();
	void Empty(bool bDestroyChildren = false);
	FUIDatasource* GetChildAt(int32 Index) const;

	static       bool                IsArray(const FUIDatasource* Datasource) { return EnumHasAllFlags(Datasource->Flags, EUIDatasourceFlag::IsArray); }
	static       FUIArrayDatasource* Make(FUIDatasource* Datasource, bool bDestroyChildren = false);
	static       FUIArrayDatasource& Make(FUIDatasource& Datasource, bool bDestroyChildren = false);
	static       FUIArrayDatasource* Cast(FUIDatasource* Datasource) { return Datasource && IsArray(Datasource) ? static_cast<FUIArrayDatasource*>(Datasource) : nullptr; }
	static const FUIArrayDatasource* Cast(const FUIDatasource* Datasource) { return Datasource && IsArray(Datasource) ? static_cast<const FUIArrayDatasource*>(Datasource) : nullptr; }
	static const FName ItemBaseName;
};
static_assert(sizeof(FUIArrayDatasource) == sizeof(FUIDatasource), "The array datasource class is just a type discretized UIDatasource, it needs be binary equivalent.");
