// Fill out your copyright notice in the Description page of Project Settings.

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
	UFUNCTION(BlueprintPure, Category=UIDatasource)
	static FUIDatasourceHandle FindOrCreateDatasource(FUIDatasourceHandle Parent, FString Path);

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

	UFUNCTION(BlueprintPure, Category=UIDatasource) static bool	SetInt(FUIDatasourceHandle Handle, int32 Value);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static bool	SetIntAsByte(FUIDatasourceHandle Handle, uint8 Value);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static bool	SetFloat(FUIDatasourceHandle Handle, float Value);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static bool	SetBool(FUIDatasourceHandle Handle, bool Value);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static bool	SetString(FUIDatasourceHandle Handle, FString Value);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static bool	SetFName(FUIDatasourceHandle Handle, FName Value);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static bool	SetText(FUIDatasourceHandle Handle, FText Value);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static bool SetImage(FUIDatasourceHandle Handle, TSoftObjectPtr<UTexture2D> Image);
	UFUNCTION(BlueprintPure, Category=UIDatasource) static bool SetGameplayTag(FUIDatasourceHandle Handle, FGameplayTag Value);
	// @formatter:on
};
