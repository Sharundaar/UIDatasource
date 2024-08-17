// Copyright Sharundaar, Inc. All Rights Reserved.

#include "K2Node_UIDatasourceGetModelValue.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CastByteToEnum.h"
#include "KismetCompiler.h"
#include "UIDatasourceBlueprintLibrary.h"
#include "UIDatasourceEditorHelpers.h"
#include "WidgetBlueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/Texture2D.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_UIDatasourceGetModelValue)

bool UK2Node_UIDatasourceGetModelValue::CheckForErrors(const FKismetCompilerContext& CompilerContext)
{
	// @TODO: We should check for recursive errors and make sure all connected pins make sense...
	return false;
}

void UK2Node_UIDatasourceGetModelValue::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	if (CheckForErrors(CompilerContext))
	{
		BreakAllNodeLinks();
		return;
	}

	UEdGraphPin* ThenPin = GetThenPin();
	UEdGraphPin* OwnExecPin = GetExecPin();
	UEdGraphPin* DatasourceInputPin = FindPin(TEXT("Datasource"), EGPD_Input);

	if(!ThenPin->LinkedTo.IsEmpty())
	{
		UEdGraphPin* ThenPinkLinked = ThenPin->LinkedTo[0];
		while(!OwnExecPin->LinkedTo.IsEmpty())
		{
			UEdGraphPin* ExecPinLink = OwnExecPin->LinkedTo[0];
			ExecPinLink->BreakLinkTo(OwnExecPin);
			ExecPinLink->MakeLinkTo(ThenPinkLinked);
		}
	}

	OwnExecPin->BreakAllPinLinks();
	ThenPin->BreakAllPinLinks();
	
	CreatedPin = CollectCreatedPins();
	for (int i = 0; i < CreatedPin.Num(); ++i)
	{
		const FPinData& PinData = CreatedPin[i];
		UEdGraphPin* Pin = PinData.Pin;
		const FUIDatasourceDescriptor& PinDescriptor = PinData.Descriptor;

		if (PinData.Pin->LinkedTo.IsEmpty())
			continue;

		UEdGraphPin* CurrentPinDatasourcePin = DatasourceInputPin;

		if (!PinDescriptor.Path.IsEmpty())
		{
			UK2Node_CallFunction* FindDatasourceFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
			FindDatasourceFunction->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, FindChildDatasource), UUIDatasourceBlueprintLibrary::StaticClass());
			FindDatasourceFunction->AllocateDefaultPins();
			UEdGraphPin* FindDatasourceReturnPin = FindDatasourceFunction->GetReturnValuePin();
			UEdGraphPin* FindDatasourceDatasourceHandlePin = FindDatasourceFunction->FindPin(FName("Parent"));
			UEdGraphPin* FindDatasourceNamePin = FindDatasourceFunction->FindPin(FName("ChildName"));
			
			CompilerContext.CopyPinLinksToIntermediate(*CurrentPinDatasourcePin, *FindDatasourceDatasourceHandlePin);
			CurrentPinDatasourcePin = FindDatasourceReturnPin;

			FindDatasourceNamePin->DefaultValue = PinDescriptor.Path;
		}

		UEdGraphPin* HandlePin = CurrentPinDatasourcePin;

		if(PinDescriptor.Type != EUIDatasourceValueType::Archetype && PinDescriptor.Type != EUIDatasourceValueType::Void)
		{
			UK2Node_CallFunction* GetDatasourceValueNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
			GetDatasourceValueNode->FunctionReference = UIDatasourceEditorHelpers::GetGetterFunctionForDescriptor(PinDescriptor);
			GetDatasourceValueNode->AllocateDefaultPins();
			UEdGraphPin* ReturnPin = GetDatasourceValueNode->GetReturnValuePin();
			UEdGraphPin* DatasourceHandlePin = GetDatasourceValueNode->FindPin(FName(TEXT("Handle"))); // @Note: Name here is assumed to be the same for all GetDatasourceValueFunc
			if(HandlePin == DatasourceInputPin)
			{
				CompilerContext.CopyPinLinksToIntermediate(*HandlePin, *DatasourceHandlePin);
			}
			else
			{
				HandlePin->MakeLinkTo(DatasourceHandlePin);
			}
			HandlePin = ReturnPin;
		}

		if (Descriptor.Type == EUIDatasourceValueType::Enum)
		{
			UK2Node_CastByteToEnum* CastByteNode = CompilerContext.SpawnIntermediateNode<UK2Node_CastByteToEnum>(this, SourceGraph);
			CastByteNode->Enum = FindObject<UEnum>(nullptr, *Descriptor.EnumPath);
			CastByteNode->bSafe = true;
			CastByteNode->AllocateDefaultPins();
			UEdGraphPin* InputPin = CastByteNode->FindPin(TEXT("Byte")); // @NOTE: Technically UK2Node_CastByteToEnum::ByteInputPinName, but somehow generates a link error 
			HandlePin->MakeLinkTo(InputPin);
			HandlePin = CastByteNode->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
		}

		CompilerContext.MovePinLinksToIntermediate(*Pin, *HandlePin);
	}

	DatasourceInputPin->BreakAllPinLinks();
}

void UK2Node_UIDatasourceGetModelValue::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);

		NodeSpawner->DefaultMenuSignature.Category = INVTEXT("UIDatasource");
		NodeSpawner->DefaultMenuSignature.MenuName = INVTEXT("Get Model Values");

		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_UIDatasourceGetModelValue::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return INVTEXT("Get Model Values");
}

UEdGraphPin* UK2Node_UIDatasourceGetModelValue::GeneratePinForDescriptor(const FUIDatasourceDescriptor& Element)
{
	const FString HandleName = Element.Path.IsEmpty() ? TEXT("Value") : *Element.Path;
	FEdGraphPinType PinType = UIDatasourceEditorHelpers::GetPinTypeForDescriptor(Element);
	return CreatePin(EGPD_Output, PinType, FName(HandleName));
}

void UK2Node_UIDatasourceGetModelValue::PreloadArchetypeAndDependencies(UUIDatasourceArchetype* InArchetype)
{
	if (!IsValid(InArchetype))
	{
		return;
	}

	PreloadObject(InArchetype);
	for(const FUIDatasourceDescriptor& ArchetypeDescriptor : InArchetype->GetDescriptors())
	{
		if (ArchetypeDescriptor.IsInlineArchetype())
		{
			PreloadArchetypeAndDependencies(ArchetypeDescriptor.Archetype);
		}
	}
}

void UK2Node_UIDatasourceGetModelValue::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, FUIDatasourceHandle::StaticStruct(), "Datasource");

	if (Pins.Num() > 3)
	{
		Pins.RemoveAt(3, Pins.Num() - 3, false);
	}

	if (Descriptor.Type == EUIDatasourceValueType::Archetype && IsValid(Descriptor.Archetype) && Descriptor.ImportMethod != EUIDatasourceArchetypeImportMethod::AsArray)
	{
		PreloadArchetypeAndDependencies(Descriptor.Archetype);
		for (const FUIDatasourceDescriptor& Child : Descriptor.Archetype->GetDescriptors())
		{
			if (Child.IsInlineArchetype())
			{
				for (const FUIDatasourceDescriptor& RecChild : Child.Archetype->GetDescriptors())
				{
					CreatedPin.Add({ GeneratePinForDescriptor(RecChild), RecChild });					
				}
			}
			else
			{
				CreatedPin.Add({ GeneratePinForDescriptor(Child), Child });
			}
		}
	}
	else
	{
		CreatedPin.Add({ GeneratePinForDescriptor(Descriptor), Descriptor });
	}
}

bool UK2Node_UIDatasourceGetModelValue::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
	return Super::IsCompatibleWithGraph(TargetGraph);
}

FNodeHandlingFunctor* UK2Node_UIDatasourceGetModelValue::CreateNodeHandler(FKismetCompilerContext& CompilerContext) const
{
	return new FNodeHandlingFunctor(CompilerContext);
}

void UK2Node_UIDatasourceGetModelValue::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UK2Node_UIDatasourceGetModelValue, Descriptor))
	{
		ReconstructNode();
	}
}

TArray<UK2Node_UIDatasourceGetModelValue::FPinData> UK2Node_UIDatasourceGetModelValue::CollectCreatedPins() const
{
	if(Descriptor.Type == EUIDatasourceValueType::Archetype && Descriptor.Archetype && Descriptor.ImportMethod != EUIDatasourceArchetypeImportMethod::AsArray)
	{
		TArray<UK2Node_UIDatasourceGetModelValue::FPinData> PinData;

		for(const FUIDatasourceDescriptor& Elem : Descriptor.Archetype->GetDescriptors())
		{
			if(Elem.IsInlineArchetype())
			{
				for (const FUIDatasourceDescriptor& RecChild : Elem.Archetype->GetDescriptors())
				{
					PinData.Add({
						FindPinChecked(RecChild.Path),
						RecChild,
					});
				}
			}
			else
			{
				PinData.Add({
					FindPinChecked(Elem.Path),
					Elem,
				});
			}
		}
		
		return PinData;
	}
	
	if(Descriptor.Path.IsEmpty())
	{
		return { { FindPinChecked(TEXT("Value"), EGPD_Output), Descriptor } };
	}

	return { { FindPinChecked(Descriptor.Path, EGPD_Output), Descriptor } };
}
