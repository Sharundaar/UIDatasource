// Copyright Sharundaar. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "UIDatasourceDefines.h"
#include "UIDatasourceHandle.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"

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
	Struct, // FInstancedStruct
	Archetype, // @NOTE: This is for archetype UX purposes, devolves to Void at runtime
};

// A struct capable of holding UImage compatible assets
USTRUCT(BlueprintType)
struct FUIDatasourceImage
{
	GENERATED_BODY()

	FUIDatasourceImage() {}
	FUIDatasourceImage(TSoftObjectPtr<UTexture2D> Texture2D) : Image(Texture2D) {}
	FUIDatasourceImage(TSoftObjectPtr<UMaterialInterface> Material) : Image(Material) {}

	// Soft Ptr of an image compatible with UImage, see UImage Image property
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess="true", DisplayThumbnail="true", DisplayName="Image", AllowedClasses="/Script/Engine.Texture,/Script/Engine.MaterialInterface,/Script/Engine.SlateTextureAtlasInterface", DisallowedClasses = "/Script/MediaAssets.MediaTexture"))
	TSoftObjectPtr<UObject> Image = {};

	// Cast the UObject soft ptr to Texture2D soft ptr
	TSoftObjectPtr<UTexture2D> AsTexture() const { return TSoftObjectPtr<UTexture2D>(Image.ToSoftObjectPath()); };
	
	// Cast the UObject soft ptr to MaterialInterface soft ptr
	TSoftObjectPtr<UMaterialInterface> AsMaterial() const { return TSoftObjectPtr<UMaterialInterface>(Image.ToSoftObjectPath()); };

	bool operator==(const FUIDatasourceImage& Other) const { return Image == Other.Image; }
};

struct FUIDatasourceValue
{
	struct FVoidType {};
	using FValueType = TVariant<FVoidType, int32, float, bool, FName, FText, FString, FUIDatasourceImage, FGameplayTag, FInstancedStruct>;

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
		if(Value.IsType<FUIDatasourceImage>()) return EUIDatasourceValueType::Image;
		if(Value.IsType<FGameplayTag>()) return EUIDatasourceValueType::GameplayTag;
		if(Value.IsType<FInstancedStruct>()) return EUIDatasourceValueType::Struct;
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
	const T& Get_Ref() const
	{
		UIDATASOURCE_FUNC_TRACE()

		static_assert(FValueType::IndexOfType<T>() != static_cast<SIZE_T>(-1), "Tried to get datasource to invalid type.");
		
		if(Value.IsType<T>())
		{
			return Value.Get<T>();
		}

		UE_LOG(LogDatasource, Warning, TEXT("Tried to get a value of incompatible type (Found %llu, Expected %llu), returning default"), FValueType::IndexOfType<T>(), Value.GetIndex());
		static T StaticObjectInstance = T {}; // this allows us to return an "invalid" ref
		return StaticObjectInstance;
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
	IsSink = 1 << 0, // Sink datasource returns themselves when querying children, no-op on Set, and return default values on Get 
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

	void GetPath(FString& OutPath);
	
	template<typename T>
	bool Set(T InValue)
	{
		if(!EnumHasAllFlags(Flags, EUIDatasourceFlag::IsSink)
			&& Value.Set<T>(InValue))
		{
			OnValueChanged();
			return true;
		}
		return false;
	}

	template<typename T>
	const T& Get_Ref() const
	{
		return Value.Get_Ref<T>();
	}
	
	template<typename T>
	T Get() const
	{
		return EnumHasAllFlags(Flags, EUIDatasourceFlag::IsSink) ? T{} : Value.Get<T>();
	}

	template<typename T>
	bool TryGet(T& OutValue) const
	{
		return EnumHasAllFlags(Flags, EUIDatasourceFlag::IsSink) ? false : Value.TryGet<T>(OutValue);
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
	FUIDatasource* Append(); // Append a datasource to the end of the array
	FUIDatasource* AppendFront(); // Append a datasource to the front of the array, reshuffle the IDs of subsequent elements
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
