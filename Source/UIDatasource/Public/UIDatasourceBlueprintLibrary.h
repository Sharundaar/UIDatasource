// Copyright Sharundaar. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UIDatasourceHandle.h"

#include "UIDatasourceBlueprintLibrary.generated.h"

UCLASS()
class UIDATASOURCE_API UUIDatasourceBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category=UIDatasource)
	static FUIDatasourceHandle FindOrCreateDatasource(FUIDatasourceHandle Parent, FString Path);

	UFUNCTION(BlueprintPure, Category=UIDatasource, meta=(CompactNodeTitle="VALID"))
	static bool IsValid(FUIDatasourceHandle Handle);

	UFUNCTION(BlueprintPure, Category=UIDatasource, DisplayName="Equal (Datasource)", meta=(CompactNodeTitle="==", Keywords="== equal"))
	static bool EqualEqual_DatasourceHandle(FUIDatasourceHandle HandleA, FUIDatasourceHandle HandleB);

	UFUNCTION(BlueprintPure, Category=UIArrayDatasource, DisplayName="Is Array")
	static bool ArrayDatasource_IsArray(FUIDatasourceHandle ArrayHandle);
	
	UFUNCTION(BlueprintPure, Category=UIArrayDatasource, meta=(CompactNodeTitle="NUM"))
	static int32 ArrayDatasource_GetNum(FUIDatasourceHandle ArrayHandle);
	
	UFUNCTION(BlueprintPure, Category=UIArrayDatasource, DisplayName="Get Child At")
	static FUIDatasourceHandle ArrayDatasource_GetChildAt(FUIDatasourceHandle ArrayHandle, int32 Index);
	
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
	UFUNCTION(BlueprintPure, Category=UIDatasource) static TSoftObjectPtr<UTexture2D> GetImage(FUIDatasourceHandle Handle);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static FGameplayTag GetGameplayTag(FUIDatasourceHandle Handle);

	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool	SetInt(FUIDatasourceHandle Handle, int32 Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool	SetIntAsByte(FUIDatasourceHandle Handle, uint8 Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool	SetFloat(FUIDatasourceHandle Handle, float Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool	SetBool(FUIDatasourceHandle Handle, bool Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool	SetString(FUIDatasourceHandle Handle, FString Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool	SetFName(FUIDatasourceHandle Handle, FName Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool	SetText(FUIDatasourceHandle Handle, FText Value);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool SetImage(FUIDatasourceHandle Handle, TSoftObjectPtr<UTexture2D> Image);
	UFUNCTION(BlueprintCallable, Category=UIDatasource) static bool SetGameplayTag(FUIDatasourceHandle Handle, FGameplayTag Value);
	// @formatter:on
};
