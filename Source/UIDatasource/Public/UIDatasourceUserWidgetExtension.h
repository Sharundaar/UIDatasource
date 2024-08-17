// Copyright Sharundaar. All Rights Reserved.

#pragma once

#include "UIDatasource.h"
#include "UIDatasourceArchetype.h"
#include "UIDatasourceHandle.h"
#include "Extensions/UserWidgetExtension.h"
#include "Extensions/WidgetBlueprintGeneratedClassExtension.h"

#include "UIDatasourceUserWidgetExtension.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FUIDatasourceChangedDelegate, FUIDatasourceHandle, Handle);

// Represents the type of binding we operate with regard to a datasource
UENUM(BlueprintType)
enum class EDatasourceBindType : uint8
{
	Self, // Binds to the datasource assigned to a widget
	Global, // Binds to a global datasource directly parented to the root
};

UINTERFACE(BlueprintType)
class UIDATASOURCE_API UUIDatasourceEventHandler : public UInterface
{
	GENERATED_BODY()
};

class UIDATASOURCE_API IUIDatasourceEventHandler : public IInterface
{
	GENERATED_BODY()

public:
	virtual void NativeOnDatasourceChanging(FUIDatasourceHandle Handle);
	virtual void NativeOnDatasourceChanged(FUIDatasourceHandle Handle);

	// Called before the datasource has been changed, can be used to prepare a datasource before assignation
	UFUNCTION(BlueprintImplementableEvent, DisplayName="Pre Datasource Change")
	void BP_OnDatasourceChanging(FUIDatasourceHandle Handle);

	// Called after the widget's datasource has been changed, can be used to batch setup states
	UFUNCTION(BlueprintImplementableEvent, DisplayName="Post Datasource Change")
	void BP_OnDatasourceChanged(FUIDatasourceHandle Handle);
};

struct FUIDataBind
{
	FOnDatasourceChangedDelegateBP Bind;
	FString Path;
	EDatasourceBindType BindType;
};

UCLASS()
class UIDATASOURCE_API UUIDatasourceUserWidgetExtension : public UUserWidgetExtension
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	FUIDatasourceHandle GetDatasource() const { return Handle; }
	
	UFUNCTION(BlueprintCallable)
	void SetDatasource(FUIDatasourceHandle InHandle);

	UFUNCTION(BlueprintCallable, DisplayName="Set Datasource", meta=(DefaultToSelf=UserWidget))
	static void SetUserWidgetDatasource(UUserWidget* UserWidget, FUIDatasourceHandle Handle);

	UFUNCTION(BlueprintPure, DisplayName="Get Datasource", meta=(DefaultToSelf=UserWidget))
	static FUIDatasourceHandle GetUserWidgetDatasource(UUserWidget* UserWidget);

	UFUNCTION(BlueprintCallable, meta=(DefaultToSelf=UserWidget))
	static UUIDatasourceUserWidgetExtension* RegisterDatasourceExtension(UUserWidget* UserWidget);
	
	void UpdateBindings(FUIDatasourceHandle OldHandle, FUIDatasourceHandle NewHandle);
	void AddBinding(const FUIDataBind& Binding);
	
	virtual void Construct() override;
	virtual void Destruct() override;

protected:
	FUIDatasourceHandle Handle;
	TArray<FUIDataBind> Bindings;
	TArray<FUIDataBind> GlobalBindings;
};

USTRUCT()
struct UIDATASOURCE_API FUIDataBindTemplate
{
	GENERATED_BODY()
	
	UPROPERTY()
	FName BindDelegateName = {};

	UPROPERTY()
	FString Path = {};

	UPROPERTY()
	EDatasourceBindType BindType = EDatasourceBindType::Self;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	FUIDatasourceDescriptor Descriptor;
#endif
};

UCLASS()
class UIDATASOURCE_API UUIDatasourceWidgetBlueprintGeneratedClassExtension : public UWidgetBlueprintGeneratedClassExtension
{
	GENERATED_BODY()

public:
	virtual void Initialize(UUserWidget* UserWidget) override;
#if WITH_EDITORONLY_DATA
	virtual void PreConstruct(UUserWidget* UserWidget, bool IsDesignTime) override;
#endif

	UPROPERTY()
	TArray<FUIDataBindTemplate> Bindings;
};
