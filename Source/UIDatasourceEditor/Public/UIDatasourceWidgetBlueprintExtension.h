// Copyright Sharundaar. All Rights Reserved.

#pragma once

#include "WidgetBlueprintExtension.h"

#include "UIDatasourceWidgetBlueprintExtension.generated.h"

UCLASS()
class UUIDatasourceWidgetBlueprintExtension : public UWidgetBlueprintExtension
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FOnBindingChanged);
	FOnBindingChanged OnBindingChanged;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UUIDatasourceArchetype> Archetype;
};