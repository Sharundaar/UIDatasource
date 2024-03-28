// Copyright Sharundaar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintModes/WidgetBlueprintApplicationMode.h"
#include "Modules/ModuleManager.h"

class SUIDatasourceDebugger;

class FUIDatasourceEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static void HandleRegisterBlueprintEditorTab(const FWidgetBlueprintApplicationMode& WidgetBlueprintApplicationMode, FWorkflowAllowedTabSet& WorkflowAllowedTabs);
    TSharedRef<SUIDatasourceDebugger> GetDatasourceDebugger(TSharedRef<SDockTab> InParentTab);
    TSharedRef<SDockTab> SpawnDatasourceDebugger(const FSpawnTabArgs& SpawnTabArgs);

protected:
    TWeakPtr<SUIDatasourceDebugger> DatasourceDebuggerPtr;
};
