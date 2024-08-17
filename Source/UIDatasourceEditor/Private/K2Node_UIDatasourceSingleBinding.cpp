// Copyright Sharundaar. All Rights Reserved.

#include "K2Node_UIDatasourceSingleBinding.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "GraphEditorSettings.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CastByteToEnum.h"
#include "K2Node_CustomEvent.h"
#include "KismetCompiler.h"
#include "UIDatasourceArchetype.h"
#include "UIDatasourceBlueprintLibrary.h"
#include "UIDatasourceEditorHelpers.h"
#include "UIDatasourceWidgetBlueprintExtension.h"
#include "WidgetBlueprint.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/Texture2D.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Materials/MaterialInstance.h"
#include "Styling/AppStyle.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_UIDatasourceSingleBinding)

namespace UK2Node_UIDatasourceSingleBinding_Local 
{
	FUIDatasourceDescriptor GenerateDescriptorForNode(const UK2Node_UIDatasourceSingleBinding* Node)
	{
		return FUIDatasourceDescriptor{
			.Path = Node->Path,
			.Type = Node->Type,
			.EnumPath = Node->EnumPath,
			.ImageType = Node->ImageType,
			.Archetype = nullptr,
			.ImportMethod = EUIDatasourceArchetypeImportMethod::AsChild
		};
	}
}

void UK2Node_UIDatasourceSingleBinding::CreateProperlyTypedModelOutputPin()
{
	Modify();

	int32 HandlePinPosition = bShowBindEventPin ? 2 : 1;
	if (Pins.Num() > HandlePinPosition)
	{
		Pins.RemoveAt(HandlePinPosition, 1, false);
	}

	FEdGraphPinType PinType = UIDatasourceEditorHelpers::GetPinTypeForDescriptor(UK2Node_UIDatasourceSingleBinding_Local::GenerateDescriptorForNode(this));

	CreatePin(EGPD_Output, PinType, *Path, HandlePinPosition);

	// Notify the graph that the node has been changed
	if (UEdGraph* Graph = GetGraph())
	{
		Graph->NotifyGraphChanged();
	}
}

FName UK2Node_UIDatasourceSingleBinding::PN_EventKind = "EventKind";

void UK2Node_UIDatasourceSingleBinding::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	if (bShowBindEventPin)
	{
		CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Byte, StaticEnum<EUIDatasourceChangeEventKind>(), PN_EventKind);
	}
	CreateProperlyTypedModelOutputPin();
	
	Super::AllocateDefaultPins();
}

FName UK2Node_UIDatasourceSingleBinding::GetGeneratedEventName() const
{
	return FName(*FString::Printf(TEXT("DataBndEvt_%s_Changed"), *Path));
}

bool UK2Node_UIDatasourceSingleBinding::CheckForErrors(const FKismetCompilerContext& CompilerContext) const
{
	if(Path.IsEmpty())
	{
		CompilerContext.MessageLog.Error(TEXT("Bind Path for node %% is empty"));
		return true;
	}

	return false;
}

void UK2Node_UIDatasourceSingleBinding::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	if (CheckForErrors(CompilerContext))
	{
		BreakAllNodeLinks();
		return;
	}

	UEdGraphPin* ThenPin = GetThenPin();

	UK2Node_CustomEvent* EventNode = CompilerContext.SpawnIntermediateNode<UK2Node_CustomEvent>(this, SourceGraph);
	EventNode->CustomFunctionName = GetGeneratedEventName();
	EventNode->EventReference.SetExternalDelegateMember(FName(TEXT("OnDatasourceChangedDelegateBP__DelegateSignature"))); // UIDatasource.h -> FOnDatasourceChangedDelegateBP
	EventNode->AllocateDefaultPins();

	CompilerContext.MovePinLinksToIntermediate(*ThenPin, *EventNode->GetThenPin());

	UEdGraphPin* EventArgsPin = EventNode->FindPin(FName(TEXT("EventArgs")));
	check(EventNode->CanSplitPin(EventArgsPin));
	EventArgsPin->GetSchema()->SplitPin(EventArgsPin, false);
	UEdGraphPin* HandlePin = EventArgsPin->SubPins[1];
	checkf(HandlePin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct && HandlePin->PinType.PinSubCategoryObject == FUIDatasourceHandle::StaticStruct(), TEXT("If this hits, probably means FUIDatasourceChangeEventArgs field order changed."));
	
	const bool bNeitherNoneOrArchetypeType = !(Type == EUIDatasourceValueType::Void || Type == EUIDatasourceValueType::Archetype);
	if (bNeitherNoneOrArchetypeType)
	{
		UK2Node_CallFunction* GetDatasourceValueNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
		GetDatasourceValueNode->FunctionReference = UIDatasourceEditorHelpers::GetGetterFunctionForDescriptor(UK2Node_UIDatasourceSingleBinding_Local::GenerateDescriptorForNode(this));
		GetDatasourceValueNode->AllocateDefaultPins();
		UEdGraphPin* ReturnPin = GetDatasourceValueNode->GetReturnValuePin();
		UEdGraphPin* DatasourceHandlePin = GetDatasourceValueNode->FindPin(FName(TEXT("Handle"))); // Name here is assumed to be the same for all GetDatasourceValueFunc
		HandlePin->MakeLinkTo(DatasourceHandlePin);
		HandlePin = ReturnPin;

		if (Type == EUIDatasourceValueType::Enum)
		{
			UK2Node_CastByteToEnum* CastByteNode = CompilerContext.SpawnIntermediateNode<UK2Node_CastByteToEnum>(this, SourceGraph);
			CastByteNode->Enum = FindObject<UEnum>(nullptr, *EnumPath);
			CastByteNode->bSafe = true;
			CastByteNode->AllocateDefaultPins();
			UEdGraphPin* InputPin = CastByteNode->FindPin(TEXT("Byte")); // @NOTE: Technically UK2Node_CastByteToEnum::ByteInputPinName, but the symbol isn't exported 
			HandlePin->MakeLinkTo(InputPin);
			HandlePin = CastByteNode->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
		}
	}

	if (bShowBindEventPin)
	{
		CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(PN_EventKind), *EventArgsPin->SubPins[0]);	
	}
	
	checkf(HandlePin, TEXT("HandlePin should be the output of the built graph previously, so we can reconnect to the rest of the ubergraph."));

	const int32 OutputPin = bShowBindEventPin ? 2 : 1;
	ensureMsgf(Pins.IsValidIndex(OutputPin), TEXT("OutputPin here should be valid and point to the typed pin we created for the datasource data."));
	CompilerContext.MovePinLinksToIntermediate(*Pins[bShowBindEventPin ? 2 : 1], *HandlePin);
}

FSlateIcon UK2Node_UIDatasourceSingleBinding::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon(FAppStyle::GetAppStyleSetName(), "GraphEditor.Event_16x");
	return Icon;
}

void UK2Node_UIDatasourceSingleBinding::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	auto CustomizeArchetypeNodeLambda = [](UEdGraphNode* NewNode, bool /*bIsTemplateNode*/, TWeakObjectPtr<const UUIDatasourceArchetype> WeakArchetype, FUIDatasourceDescriptor Descriptor)
	{
		UK2Node_UIDatasourceSingleBinding* BindingNode = CastChecked<UK2Node_UIDatasourceSingleBinding>(NewNode);
		BindingNode->Path = Descriptor.Path;
		BindingNode->EnumPath = Descriptor.EnumPath;
		BindingNode->Type = Descriptor.Type;
		BindingNode->SourceArchetype = WeakArchetype.Get();
	};

	if(ActionRegistrar.IsOpenForRegistration(GetClass()))
	{
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		static bool bRegisterOnce = true;
		if (bRegisterOnce)
		{
			bRegisterOnce = false;
			if (AssetRegistry.IsLoadingAssets())
			{
				AssetRegistry.OnFilesLoaded().AddLambda([]() { FBlueprintActionDatabase::Get().RefreshClassActions(StaticClass()); });
			}
		}
		
		{
			UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
			NodeSpawner->DefaultMenuSignature.Category = INVTEXT("UIDatasource Bindings");
			NodeSpawner->DefaultMenuSignature.MenuName = INVTEXT("Bind to Custom Model...");
			check(NodeSpawner != nullptr);
						
			NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(CustomizeArchetypeNodeLambda, TWeakObjectPtr<const UUIDatasourceArchetype>(), FUIDatasourceDescriptor{});
			ActionRegistrar.AddBlueprintAction(GetClass(), NodeSpawner);
		}

		TArray<FAssetData> ActionAssets;
		AssetRegistry.GetAssetsByClass(UUIDatasourceArchetype::StaticClass()->GetClassPathName(), ActionAssets, true);
		for (const FAssetData& ActionAsset : ActionAssets)
		{
			if (FPackageName::GetPackageMountPoint(ActionAsset.PackageName.ToString()) != NAME_None)
			{
				if (const UUIDatasourceArchetype* Archetype = Cast<const UUIDatasourceArchetype>(ActionAsset.GetAsset()))
				{
					for(auto& Descriptor: Archetype->GetDescriptors())
					{
						UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
						NodeSpawner->DefaultMenuSignature.Category = INVTEXT("UIDatasource Bindings");
						NodeSpawner->DefaultMenuSignature.MenuName = FText::FormatOrdered(INVTEXT("Bind to {0} Changed"), FText::FromString(Descriptor.Path));
						check(NodeSpawner != nullptr);
						
						NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(CustomizeArchetypeNodeLambda, TWeakObjectPtr<const UUIDatasourceArchetype>(Archetype), Descriptor);
						ActionRegistrar.AddBlueprintAction(Archetype, NodeSpawner);
					}
				}
			}
		}
	}
	else if (const UUIDatasourceArchetype* Archetype = Cast<const UUIDatasourceArchetype>(ActionRegistrar.GetActionKeyFilter()))
	{
		for(auto& Descriptor: Archetype->GetDescriptors())
		{
			UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
			NodeSpawner->DefaultMenuSignature.Category = INVTEXT("UIDatasource Bindings");
			NodeSpawner->DefaultMenuSignature.MenuName = FText::FormatOrdered(INVTEXT("Bind to {0} Changed"), FText::FromString(Descriptor.Path));
			check(NodeSpawner != nullptr);
			
			NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(CustomizeArchetypeNodeLambda, TWeakObjectPtr<const UUIDatasourceArchetype>(Archetype), Descriptor);
			ActionRegistrar.AddBlueprintAction(Archetype, NodeSpawner);
		}
	}
}

FText UK2Node_UIDatasourceSingleBinding::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if(Path.IsEmpty())
	{
		return INVTEXT("Assign Model Path in Node Properties");
	}
	else
	{
		return FText::FormatOrdered(INVTEXT("Model {0} Changed"), FText::FromString(Path));
	}
}

bool UK2Node_UIDatasourceSingleBinding::IsActionFilteredOut(const FBlueprintActionFilter& Filter)
{
	if (SourceArchetype == nullptr)
	{
		return false;
	}

	for(UBlueprint* Blueprint : Filter.Context.Blueprints)
	{
		const UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Blueprint);
		if(!WidgetBlueprint)
		{
			return true;
		}
		
		const UUIDatasourceWidgetBlueprintExtension* DatasourceExtension = UWidgetBlueprintExtension::GetExtension<UUIDatasourceWidgetBlueprintExtension>(WidgetBlueprint);
		if(!DatasourceExtension)
		{
			return true;
		}

		if(DatasourceExtension->Archetype != SourceArchetype)
		{
			return true;
		}
	}

	return false;
}

bool UK2Node_UIDatasourceSingleBinding::IsCompatibleWithGraph(const UEdGraph* Graph) const
{
	if(!Super::IsCompatibleWithGraph(Graph))
	{
		return false;
	}

	if(Graph->GetSchema()->GetGraphType(Graph) != GT_Ubergraph)
	{
		return false;
	}
	
	if(!FBlueprintEditorUtils::FindBlueprintForGraph(Graph)->IsA<UWidgetBlueprint>())
	{
		return false;
	}

	return true;
}

void UK2Node_UIDatasourceSingleBinding::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UK2Node_UIDatasourceSingleBinding, Path)
		|| PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UK2Node_UIDatasourceSingleBinding, Type)
		|| PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UK2Node_UIDatasourceSingleBinding, EnumPath))
	{
		CreateProperlyTypedModelOutputPin();
	}

	if(PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UK2Node_UIDatasourceSingleBinding, bShowBindEventPin))
	{
		ReconstructNode();
	}
}

FLinearColor UK2Node_UIDatasourceSingleBinding::GetNodeTitleColor() const
{
	return GetDefault<UGraphEditorSettings>()->EventNodeTitleColor;
}
