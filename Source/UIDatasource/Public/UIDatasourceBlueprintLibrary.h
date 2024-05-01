// Copyright Sharundaar. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "UIDatasource.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UIDatasourceHandle.h"
#include "Components/Image.h"

#include "UIDatasourceBlueprintLibrary.generated.h"

class UImage;
struct FUIDatasourceImage;

UCLASS(BlueprintType)
class UIDATASOURCE_API UUIDatasourceWrapper : public UObject
{
	GENERATED_BODY()

public:
	UUIDatasourceWrapper() : Handle() {}
	
	UFUNCTION(BlueprintPure, meta=(CompactNodeTitle="GET"))
	FUIDatasourceHandle GetHandle() const { return Handle; }

	void SetHandle(FUIDatasourceHandle InHandle) { Handle = InHandle; }
	
protected:
	FUIDatasourceHandle Handle;
};

UCLASS()
class UIDATASOURCE_API UUIDatasourceBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Returns the datasource at the given Path relative to Parent, creates it if it doesn't exist, if Parent is invalid, creates the datasource from the Root
	UFUNCTION(BlueprintCallable, Category=UIDatasource)
	static FUIDatasourceHandle FindOrCreateDatasource(FUIDatasourceHandle Parent, FString Path);

	// Returns the datasource at the given Path relative to Parent, if Parent is invalid, finds from the root, returns invalid if no datasource is found
	UFUNCTION(BlueprintCallable, Category=UIDatasource)
	static FUIDatasourceHandle FindDatasource(FUIDatasourceHandle Parent, FString Path);
	
	// Returns the child datasource with the given name, returns invalid datasource if not found.
	UFUNCTION(BlueprintPure, Category=UIDatasource)
	static FUIDatasourceHandle FindChildDatasource(FUIDatasourceHandle Parent, FName ChildName);

	// Returns the child datasource with the given name, returns invalid datasource if not found.
	UFUNCTION(BlueprintCallable, Category=UIDatasource)
	static FUIDatasourceHandle FindOrCreateChildDatasource(FUIDatasourceHandle Parent, FName ChildName);

	// Returns if the datasource is valid
	UFUNCTION(BlueprintPure, Category=UIDatasource, meta=(CompactNodeTitle="VALID"))
	static bool IsValid(FUIDatasourceHandle Handle);

	// Check that two datasources are equal
	UFUNCTION(BlueprintPure, Category=UIDatasource, DisplayName="Equal (Datasource)", meta=(CompactNodeTitle="==", Keywords="== equal"))
	static bool EqualEqual_DatasourceHandle(FUIDatasourceHandle HandleA, FUIDatasourceHandle HandleB);
	
	// Setup datasource to be an array
	UFUNCTION(BlueprintCallable, Category=UIArrayDatasource, DisplayName="Make Array")
	static FUIDatasourceHandle ArrayDatasource_MakeArray(FUIDatasourceHandle Handle);
	
	// Check that a datasource is an array
	UFUNCTION(BlueprintPure, Category=UIArrayDatasource, DisplayName="Is Array")
	static bool ArrayDatasource_IsArray(FUIDatasourceHandle ArrayHandle);

	// Returns the number of child elements of an array datasource, 0 if passed datasource isn't valid (or empty array)
	UFUNCTION(BlueprintPure, Category=UIArrayDatasource, meta=(CompactNodeTitle="NUM"))
	static int32 ArrayDatasource_GetNum(FUIDatasourceHandle ArrayHandle);

	// Collect all datasource child of the ArrayHandle parameter into an unreal Array
	// [PadArrayUpTo] : optionally to ensure the resulting array has at least PadArrayUpTo elements
	// [KeepNumMultipleOf] : optionally to ensure the resulting array has a number of element a multiple of KeepNumMultipleOf
	UFUNCTION(BlueprintPure, Category=UIArrayDatasource, DisplayName="Array Datasource To Array", meta=(AdvancedDisplay=2))
	static TArray<FUIDatasourceHandle> ArrayDatasource_ToArray(FUIDatasourceHandle ArrayHandle, int32 PadArrayUpTo = 0, int32 KeepNumMultipleOf = 0);

	// Similar to Array Datasource To Array, but wrap all datasource into a UObject, useful for ListView and consort
	// [PadArrayUpTo] : optionally to ensure the resulting array has at least PadArrayUpTo elements
	// [KeepNumMultipleOf] : optionally to ensure the resulting array has a number of element a multiple of KeepNumMultipleOf
	UFUNCTION(BlueprintPure, Category=UIArrayDatasource, DisplayName="Array Datasource To Object Array", meta=(AdvancedDisplay=2))
	static TArray<UUIDatasourceWrapper*> ArrayDatasource_ToObjectArray(FUIDatasourceHandle ArrayHandle, int32 PadArrayUpTo = 0, int32 KeepNumMultipleOf = 0);
	
	// Returns the child at Index of the passed array datasource, returns invalid datasource if not in range
	UFUNCTION(BlueprintPure, Category=UIArrayDatasource, DisplayName="Get Child At")
	static FUIDatasourceHandle ArrayDatasource_GetChildAt(FUIDatasourceHandle ArrayHandle, int32 Index);

	// Append a datasource at the end of this array and returns it
	UFUNCTION(BlueprintCallable, Category=UIArrayDatasource, DisplayName="Append")
	static FUIDatasourceHandle ArrayDatasource_Append(FUIDatasourceHandle ArrayHandle);

	// @formatter:off
	UFUNCTION(BlueprintPure, Category=UIDatasource) static int32		GetInt(FUIDatasourceHandle Handle);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static uint8		GetIntAsByte(FUIDatasourceHandle Handle);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static float		GetFloat(FUIDatasourceHandle Handle);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static bool			GetBool(FUIDatasourceHandle Handle);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static FString		GetString(FUIDatasourceHandle Handle);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static FName		GetFName(FUIDatasourceHandle Handle);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static FText		GetText(FUIDatasourceHandle Handle);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static FUIDatasourceImage GetImage(FUIDatasourceHandle Handle);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static TSoftObjectPtr<UTexture2D> GetTexture(FUIDatasourceHandle Handle);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static TSoftObjectPtr<UMaterialInterface> GetMaterial(FUIDatasourceHandle Handle);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static FGameplayTag GetGameplayTag(FUIDatasourceHandle Handle);

	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool	SetInt(FUIDatasourceHandle Handle, int32 Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool	SetIntAsByte(FUIDatasourceHandle Handle, uint8 Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool	SetFloat(FUIDatasourceHandle Handle, float Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool	SetBool(FUIDatasourceHandle Handle, bool Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool	SetString(FUIDatasourceHandle Handle, FString Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool	SetFName(FUIDatasourceHandle Handle, FName Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool	SetText(FUIDatasourceHandle Handle, FText Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool SetImage(FUIDatasourceHandle Handle, FUIDatasourceImage Image);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool SetTexture(FUIDatasourceHandle Handle, TSoftObjectPtr<UTexture2D> Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool SetMaterial(FUIDatasourceHandle Handle, TSoftObjectPtr<UMaterialInterface> Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool SetGameplayTag(FUIDatasourceHandle Handle, FGameplayTag Value);
	// @formatter:on
};
