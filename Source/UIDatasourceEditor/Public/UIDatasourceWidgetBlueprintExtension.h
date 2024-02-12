﻿// Copyright Sharundaar. All Rights Reserved.

#pragma once

#include "WidgetBlueprintExtension.h"

#include "UIDatasourceWidgetBlueprintExtension.generated.h"

UCLASS()
class UUIDatasourceWidgetBlueprintExtension : public UWidgetBlueprintExtension
{
	GENERATED_BODY()

protected:
	virtual void HandleBeginCompilation(FWidgetBlueprintCompilerContext& InCreationContext) override;
	virtual void HandleFinishCompilingClass(UWidgetBlueprintGeneratedClass* Class) override;
	virtual void HandleEndCompilation() override;

public:
	DECLARE_MULTICAST_DELEGATE(FOnBindingChanged);
	FOnBindingChanged OnBindingChanged;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UUIDatasourceArchetype> Archetype;

private:
	FWidgetBlueprintCompilerContext* CurrentContext;
};
