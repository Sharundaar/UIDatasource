// Copyright Sharundaar, Inc. All Rights Reserved.

#include "K2Node_UIDatasourceSetModelValue.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CastByteToEnum.h"
#include "K2Node_EnumLiteral.h"
#include "KismetCompiler.h"
#include "UIDatasourceBlueprintLibrary.h"
#include "UIDatasourceEditorHelpers.h"
#include "WidgetBlueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/Texture2D.h"

bool UK2Node_UIDatasourceSetModelValue::CheckForErrors(const FKismetCompilerContext& CompilerContext)
{
	// @TODO: We should check for recursive errors and make sure all connected pins make sense...
	return false;
}

void UK2Node_UIDatasourceSetModelValue::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	if (CheckForErrors(CompilerContext))
	{
		BreakAllNodeLinks();
		return;
	}

	UEdGraphPin* OwnThenPin = GetThenPin();
	UEdGraphPin* OwnExecPin = GetExecPin();
	UEdGraphPin* DatasourceInputPin = FindPin(TEXT("Datasource"), EGPD_Input);

	UEdGraphPin* CurrentExecPin = OwnExecPin;
	
	CreatedPin = CollectCreatedPins();
	for (int i = 0; i < CreatedPin.Num(); ++i)
	{
		const FPinData& PinData = CreatedPin[i];
		UEdGraphPin* Pin = PinData.Pin;
		const FUIDatasourceDescriptor& PinDescriptor = PinData.Descriptor;

		if(Pin->Direction == EGPD_Input) // Handle input pins by destructuring the datasource to its children and calling the proper SetX function call
		{
			UEdGraphPin* CurrentPinDatasourcePin = DatasourceInputPin;
			if(PinDescriptor.Type != EUIDatasourceValueType::Archetype)
			{
				if (!PinDescriptor.Path.IsEmpty())
				{
					UK2Node_CallFunction* FindDatasourceFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
					FindDatasourceFunction->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, FindOrCreateChildDatasource), UUIDatasourceBlueprintLibrary::StaticClass());
					FindDatasourceFunction->AllocateDefaultPins();

					{
						UEdGraphPin* FindDatasourceNamePin = FindDatasourceFunction->FindPin(FName("ChildName"));
						FindDatasourceNamePin->DefaultValue = PinDescriptor.Path;
					}
					
					{
						UEdGraphPin* FindDatasourceDatasourceHandlePin = FindDatasourceFunction->FindPin(FName("Parent"));
						CompilerContext.CopyPinLinksToIntermediate(*CurrentPinDatasourcePin, *FindDatasourceDatasourceHandlePin);
						CurrentPinDatasourcePin = FindDatasourceFunction->GetReturnValuePin();
					}

					// Wire up the Exec pin, needs to do special treatment if the pin is our original one
					{
						if(CurrentExecPin == OwnExecPin)
						{
							CompilerContext.MovePinLinksToIntermediate(*CurrentExecPin, *FindDatasourceFunction->GetExecPin());
						}
						else
						{
							CurrentExecPin->MakeLinkTo(FindDatasourceFunction->GetExecPin());
						}
						CurrentExecPin = FindDatasourceFunction->GetThenPin();
					}
				}

				UK2Node_CallFunction* SetDatasourceValueNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
				SetDatasourceValueNode->FunctionReference = UIDatasourceEditorHelpers::GetSetterFunctionForDescriptor(PinDescriptor);
				SetDatasourceValueNode->AllocateDefaultPins();
				UEdGraphPin* InputPin = SetDatasourceValueNode->FindPinChecked(TEXT("Value"), EGPD_Input);
				UEdGraphPin* DatasourceHandlePin = SetDatasourceValueNode->FindPin(FName(TEXT("Handle")), EGPD_Input); // @Note: Name here is assumed to be the same for all GetDatasourceValueFunc
				if(CurrentPinDatasourcePin == DatasourceInputPin)
				{
					CompilerContext.CopyPinLinksToIntermediate(*CurrentPinDatasourcePin, *DatasourceHandlePin);
				}
				else
				{
					CurrentPinDatasourcePin->MakeLinkTo(DatasourceHandlePin);
				}

				if(PinDescriptor.Type != EUIDatasourceValueType::Enum)
				{
					CompilerContext.MovePinLinksToIntermediate(*Pin, *InputPin);
				}
				else
				{
					UK2Node_EnumLiteral* EnumLiteral = CompilerContext.SpawnIntermediateNode<UK2Node_EnumLiteral>(this, SourceGraph);
					EnumLiteral->Enum = Cast<UEnum>(Pin->PinType.PinSubCategoryObject);
					EnumLiteral->AllocateDefaultPins();
					UEdGraphPin* EnumPin = EnumLiteral->FindPinChecked(UK2Node_EnumLiteral::GetEnumInputPinName());
					CompilerContext.MovePinLinksToIntermediate(*Pin, *EnumPin);
					UEdGraphPin* EnumResultPin = EnumLiteral->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue);
					EnumResultPin->MakeLinkTo(InputPin);
				}

				UEdGraphPin* NextExecPin = SetDatasourceValueNode->GetExecPin();
				CurrentExecPin->MakeLinkTo(NextExecPin);
				CurrentExecPin = SetDatasourceValueNode->GetThenPin();
			}
		}
		else // if Direction == EGPD_Output
		{
			checkf(Pin->PinType.PinSubCategoryObject == FUIDatasourceHandle::StaticStruct(), TEXT("If this hit we broke some assumptions on how those output nodes are created"));
			if (!PinDescriptor.Path.IsEmpty())
			{
				UEdGraphPin* CurrentPinDatasourcePin = DatasourceInputPin;
				
				UK2Node_CallFunction* FindDatasourceFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
				FindDatasourceFunction->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, FindOrCreateChildDatasource), UUIDatasourceBlueprintLibrary::StaticClass());
				FindDatasourceFunction->AllocateDefaultPins();

				{
					UEdGraphPin* FindDatasourceNamePin = FindDatasourceFunction->FindPin(FName("ChildName"));
					FindDatasourceNamePin->DefaultValue = PinDescriptor.Path;
				}
					
				{
					UEdGraphPin* FindDatasourceDatasourceHandlePin = FindDatasourceFunction->FindPin(FName("Parent"));
					CompilerContext.CopyPinLinksToIntermediate(*CurrentPinDatasourcePin, *FindDatasourceDatasourceHandlePin);
					CurrentPinDatasourcePin = FindDatasourceFunction->GetReturnValuePin();
				}

				// Wire up the Exec pin, needs to do special treatment if the pin is our original one
				{
					if(CurrentExecPin == OwnExecPin)
					{
						CompilerContext.MovePinLinksToIntermediate(*CurrentExecPin, *FindDatasourceFunction->GetExecPin());
					}
					else
					{
						CurrentExecPin->MakeLinkTo(FindDatasourceFunction->GetExecPin());
					}
					CurrentExecPin = FindDatasourceFunction->GetThenPin();
				}

				{ // Wire up a Make Array Datasource node if necessary
					if(PinDescriptor.ImportMethod == EUIDatasourceArchetypeImportMethod::AsArray)
					{
						UK2Node_CallFunction* MakeArrayDatasourceNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
						MakeArrayDatasourceNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, ArrayDatasource_MakeArray), UUIDatasourceBlueprintLibrary::StaticClass());
						MakeArrayDatasourceNode->AllocateDefaultPins();

						UEdGraphPin* HandlePin = MakeArrayDatasourceNode->FindPin(FName("Handle"));
						CurrentPinDatasourcePin->MakeLinkTo(HandlePin);
						CurrentPinDatasourcePin = MakeArrayDatasourceNode->GetReturnValuePin();
						
						CurrentExecPin->MakeLinkTo(MakeArrayDatasourceNode->GetExecPin());
						CurrentExecPin = MakeArrayDatasourceNode->GetThenPin();
					}
				}
				
				{ // Wire up output pin
					CompilerContext.MovePinLinksToIntermediate(*Pin, *CurrentPinDatasourcePin);
				}
			}
		}
	}

	CompilerContext.MovePinLinksToIntermediate(*OwnThenPin, *CurrentExecPin);
	DatasourceInputPin->BreakAllPinLinks();
}

void UK2Node_UIDatasourceSetModelValue::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);

		NodeSpawner->DefaultMenuSignature.Category = INVTEXT("UIDatasource");
		NodeSpawner->DefaultMenuSignature.MenuName = INVTEXT("Set Model Values");

		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_UIDatasourceSetModelValue::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return INVTEXT("Set Model Values");
}

UEdGraphPin* UK2Node_UIDatasourceSetModelValue::GeneratePinForDescriptor(const FUIDatasourceDescriptor& Element)
{
	const FString HandleName = Element.Path.IsEmpty() ? TEXT("Value") : *Element.Path;
	FEdGraphPinType PinType = UIDatasourceEditorHelpers::GetPinTypeForDescriptor(Element);
	if(PinType.PinSubCategoryObject == FUIDatasourceHandle::StaticStruct()) // @NOTE: We'll just output the child datasources that we can't set directly so the user can do what they want with them
	{
		return CreatePin(EGPD_Output, PinType, FName(HandleName));
	}
	else
	{
		return CreatePin(EGPD_Input, PinType, FName(HandleName));
	}
}

void UK2Node_UIDatasourceSetModelValue::PreloadArchetypeAndDependencies(UUIDatasourceArchetype* InArchetype)
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

void UK2Node_UIDatasourceSetModelValue::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, FUIDatasourceHandle::StaticStruct(), "Datasource");

	if (Pins.Num() > 3)
	{
		Pins.RemoveAt(3, Pins.Num() - 3, false);
	}

	if (Descriptor.Type == EUIDatasourceValueType::Archetype && IsValid(Descriptor.Archetype))
	{
		PreloadArchetypeAndDependencies(Descriptor.Archetype);
		for (const FUIDatasourceDescriptor& Child : Descriptor.Archetype->GetDescriptors())
		{
			if (Child.IsInlineArchetype())
			{
				// @TODO: Support this case
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

bool UK2Node_UIDatasourceSetModelValue::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
	return Super::IsCompatibleWithGraph(TargetGraph);
}

FNodeHandlingFunctor* UK2Node_UIDatasourceSetModelValue::CreateNodeHandler(FKismetCompilerContext& CompilerContext) const
{
	return new FNodeHandlingFunctor(CompilerContext);
}

void UK2Node_UIDatasourceSetModelValue::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UK2Node_UIDatasourceSetModelValue, Descriptor))
	{
		ReconstructNode();
	}
}

TArray<UK2Node_UIDatasourceSetModelValue::FPinData> UK2Node_UIDatasourceSetModelValue::CollectCreatedPins() const
{
	if(Descriptor.Type == EUIDatasourceValueType::Archetype && Descriptor.Archetype)
	{
		TArray<FPinData> PinData;

		for(const FUIDatasourceDescriptor& Elem : Descriptor.Archetype->GetDescriptors())
		{
			PinData.Add({
				FindPinChecked(Elem.Path),
				Elem,
			});
		}
		
		return PinData;
	}
	
	if(Descriptor.Path.IsEmpty())
	{
		return { { FindPinChecked(TEXT("Value")), Descriptor } };
	}

	return { { FindPinChecked(Descriptor.Path), Descriptor } };
}