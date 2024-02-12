// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasourceWidgetBlueprintExtension.h"

#include "K2Node_UIDatasourceSingleBinding.h"
#include "UIDatasourceUserWidgetExtension.h"

void UUIDatasourceWidgetBlueprintExtension::HandleBeginCompilation(FWidgetBlueprintCompilerContext& InCreationContext)
{
	CurrentContext = &InCreationContext;
}

void UUIDatasourceWidgetBlueprintExtension::HandleFinishCompilingClass(UWidgetBlueprintGeneratedClass* Class)
{
	check(CurrentContext);

	if(CurrentContext->bIsFullCompile)
	{
		UUIDatasourceWidgetBlueprintGeneratedClassExtension* UIDatasourceExtension = NewObject<UUIDatasourceWidgetBlueprintGeneratedClassExtension>();
		
		CurrentContext->AddExtension(Class, UIDatasourceExtension);

		TArray<UK2Node_UIDatasourceSingleBinding*> SingleBindingNodes;
		UWidgetBlueprint* WidgetBP = GetWidgetBlueprint();
		for (const UEdGraph* Graph : WidgetBP->UbergraphPages)
		{
			SingleBindingNodes.Empty();
			Graph->GetNodesOfClass<UK2Node_UIDatasourceSingleBinding>(SingleBindingNodes);
			for (const UK2Node_UIDatasourceSingleBinding* Node : SingleBindingNodes)
			{
				UIDatasourceExtension->Bindings.Add({ Node->GetGeneratedEventName(), Node->Path });
			}
		}

	}
}

void UUIDatasourceWidgetBlueprintExtension::HandleEndCompilation()
{
	CurrentContext = nullptr;
}
