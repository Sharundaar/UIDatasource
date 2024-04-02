// Copyright Sharundaar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UMGEditorModule.h"
#include "BlueprintModes/WidgetBlueprintApplicationMode.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyle.h"

class SUIDatasourceDebugger;

class FUIDatasourceStyle
    : public FSlateStyleSet
{
public:
    static FName StyleName;

    /** Access the singleton instance for this style set */
    static FUIDatasourceStyle& Get();

private:
    FUIDatasourceStyle();
    ~FUIDatasourceStyle();
};

class FUIDatasourceEditorModule : public IModuleInterface
{
public:
    void OnPatchComplete();
    virtual void StartupModule() override;
    void DeferOnShutdown(TFunction<void()> Callback);
    virtual void ShutdownModule() override;
    
    virtual bool SupportsDynamicReloading() override { return true; }
    
    static void HandleRegisterBlueprintEditorTab(const FWidgetBlueprintApplicationMode& WidgetBlueprintApplicationMode, FWorkflowAllowedTabSet& WorkflowAllowedTabs);
    TSharedRef<SUIDatasourceDebugger> GetDatasourceDebugger(TSharedRef<SDockTab> InParentTab);
    TSharedRef<SDockTab> SpawnDatasourceDebugger(const FSpawnTabArgs& SpawnTabArgs);    
protected:
    TWeakPtr<SUIDatasourceDebugger> DatasourceDebuggerPtr;
    TArray<TFunction<void()>> OnShutdownCallbacks;
};
