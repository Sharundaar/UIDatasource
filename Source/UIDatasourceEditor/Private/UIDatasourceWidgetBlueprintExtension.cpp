// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasourceWidgetBlueprintExtension.h"

#include "K2Node_UIDatasourceSingleBinding.h"
#include "UIDatasourceUserWidgetExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UIDatasourceWidgetBlueprintExtension)

void UUIDatasourceWidgetBlueprintExtension::HandleBeginCompilation(FWidgetBlueprintCompilerContext& InCreationContext)
{
	CurrentContext = &InCreationContext;
}

void UUIDatasourceWidgetBlueprintExtension::HandleFinishCompilingClass(UWidgetBlueprintGeneratedClass* Class)
{
	check(CurrentContext);

	if(CurrentContext->bIsFullCompile)
	{
		UUIDatasourceWidgetBlueprintGeneratedClassExtension* UIDatasourceExtension = NewObject<UUIDatasourceWidgetBlueprintGeneratedClassExtension>(Class);
		
		CurrentContext->AddExtension(Class, UIDatasourceExtension);

		TArray<UK2Node_UIDatasourceSingleBinding*> SingleBindingNodes;
		UWidgetBlueprint* WidgetBP = GetWidgetBlueprint();
		for (const UEdGraph* Graph : WidgetBP->UbergraphPages)
		{
			SingleBindingNodes.Empty();
			Graph->GetNodesOfClass<UK2Node_UIDatasourceSingleBinding>(SingleBindingNodes);
			for (const UK2Node_UIDatasourceSingleBinding* Node : SingleBindingNodes)
			{
				FUIDataBindTemplate Template;
				Template.BindDelegateName = Node->GetGeneratedEventName();
				Template.Path = Node->Path;
				Template.BindType = Node->BindType;
#if WITH_EDITORONLY_DATA
				Template.Descriptor = {
					Node->Path,
					Node->Type,
					Node->EnumPath,
					Node->ImageType,
				};
#endif
				UIDatasourceExtension->Bindings.Add(Template);
			}
		}

	}
}

void UUIDatasourceWidgetBlueprintExtension::HandleEndCompilation()
{
	CurrentContext = nullptr;
}

void UUIDatasourceWidgetBlueprintExtension::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UUIDatasourceWidgetBlueprintExtension, Archetype))
	{
		OnBindingChanged.Broadcast();
	}
}
