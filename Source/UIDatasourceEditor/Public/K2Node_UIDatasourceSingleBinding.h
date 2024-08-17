// Copyright Sharundaar. All Rights Reserved.

#pragma once

#include "K2Node.h"
#include "UIDatasource.h"
#include "UIDatasourceUserWidgetExtension.h"
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
	// Datasource path, relative to the BindType
	UPROPERTY(EditAnywhere)
	FString Path;

	// Self means the path is resolved from the datasource set on this widget, Global means the path is resolved from the root datasource
	UPROPERTY(EditAnywhere)
	EDatasourceBindType BindType = EDatasourceBindType::Self;

	// Show the event kind pin, useful to resolve initial bind or value changed events differently
	UPROPERTY(EditAnywhere)
	bool bShowBindEventPin = false;

	// Type of the datasource, automatically extract the inner data to the desired type
	UPROPERTY(EditAnywhere)
	EUIDatasourceValueType Type;

	// Enum held by the datasource
	UPROPERTY(EditAnywhere, meta=(EditCondition="Type==EUIDatasourceValueType::Enum", EditConditionHides, GetOptions="UIDatasource.UIDatasourceArchetype.GetEnumChoices"))
	FString EnumPath;

	// Type of image in case of an Image datasource type
	UPROPERTY(EditAnywhere, meta=(EditCondition="Type==EUIDatasourceValueType::Image", EditConditionHides))
	EUIDatasourceImageType ImageType;

	// Source Archetype, more for debugging purposes
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<const UUIDatasourceArchetype> SourceArchetype;

	static FName PN_EventKind;
};
