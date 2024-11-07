// Copyright Sharundaar, Inc. All Rights Reserved.

#pragma once

#include "K2Node.h"
#include "UIDatasourceArchetype.h"
#include "UIDatasourceEditorHelpers.h"
#include "K2Node_UIDatasourceGetModelValue.generated.h"

UCLASS()
class UIDATASOURCEEDITOR_API UK2Node_UIDatasourceGetModelValue : public UK2Node
{
	GENERATED_BODY()

public:
	// Check for error in this node configuration and break compilation
	bool CheckForErrors(const FKismetCompilerContext& CompilerContext);

	// Generate a graph pin for the descriptor
	UEdGraphPin* GeneratePinForDescriptor(const FUIDatasourceDescriptor& Element);
	
	// Make sure all archetypes and children are properly loaded
	void PreloadArchetypeAndDependencies(UUIDatasourceArchetype* InArchetype);

	//~ Begin UK2Node interface
	virtual FNodeHandlingFunctor* CreateNodeHandler(FKismetCompilerContext& CompilerContext) const override;
	virtual bool ShouldShowNodeProperties() const override { return true; }
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void AllocateDefaultPins() override;
	virtual bool IsCompatibleWithGraph(const UEdGraph* TargetGraph) const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ End UK2Node interface

	//~ Begin UObject interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject interface

protected:
	// Descriptor underlying this node
	UPROPERTY(EditAnywhere)
	FUIDatasourceDescriptor Descriptor;

	TArray<UIDatasourceEditorHelpers::FPinData> CreatedPin;
};
