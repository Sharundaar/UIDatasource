﻿// Copyright Sharundaar. All Rights Reserved.

#pragma once

#include "K2Node.h"
#include "UIDatasource.h"
#include "K2Node_UIDatasourceSingleBinding.generated.h"

enum class EUIDatasourceImageType : uint8;
class UUIDatasourceArchetype;

UCLASS()
class UIDATASOURCEEDITOR_API UK2Node_UIDatasourceSingleBinding : public UK2Node
{
	GENERATED_BODY()

public:
	void CreateProperlyTypedModelOutputPin();
	virtual void AllocateDefaultPins() override;
	FName GetGeneratedEventName() const;
	bool CheckForErrors(const FKismetCompilerContext& CompilerContext) const;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool IsActionFilteredOut(const FBlueprintActionFilter& Filter) override;
	virtual bool IsCompatibleWithGraph(const UEdGraph* Graph) const override;
	virtual bool ShouldShowNodeProperties() const override { return true; }
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual FLinearColor GetNodeTitleColor() const override;

public:
	UPROPERTY(EditAnywhere)
	FString Path;
	
	UPROPERTY(EditAnywhere)
	EUIDatasourceValueType Type;
	
	UPROPERTY(EditAnywhere, meta=(EditCondition="Type==EUIDatasourceValueType::Enum", EditConditionHides, GetOptions="UIDatasource.UIDatasourceArchetype.GetEnumChoices"))
	FString EnumPath;

	UPROPERTY(EditAnywhere, meta=(EditCondition="Type==EUIDatasourceValueType::Image", EditConditionHides))
	EUIDatasourceImageType ImageType;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<const UUIDatasourceArchetype> SourceArchetype;
};
