// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasourceUserWidgetExtension.h"

#include "UIDatasourceSubsystem.h"
#include "Blueprint/UserWidget.h"

void IUIDatasourceEventHandler::NativeOnDatasourceChanging(FUIDatasourceHandle Handle)
{
	Execute_BP_OnDatasourceChanging(Cast<UObject>(this), Handle);
}

void IUIDatasourceEventHandler::NativeOnDatasourceChanged(FUIDatasourceHandle Handle)
{
	Execute_BP_OnDatasourceChanged(Cast<UObject>(this), Handle);
}

void UUIDatasourceUserWidgetExtension::UpdateBindings(FUIDatasourceHandle OldHandle, FUIDatasourceHandle NewHandle)
{
	FUIDatasourcePool& DatasourcePool = UUIDatasourceSubsystem::Get()->Pool;
	if(FUIDatasource* OldDatasource = OldHandle.Get())
	{
		for(auto& Bind: Bindings)
		{
			if(FUIDatasource* Datasource = DatasourcePool.FindDatasource(OldDatasource, Bind.Path))
			{
				Datasource->OnDatasourceChanged.Remove(Bind.Bind);
			}
		}
	}

	if(FUIDatasource* NewDatasource = NewHandle.Get())
	{
		for(auto& Bind: Bindings)
		{
			if(FUIDatasource* Datasource = DatasourcePool.FindDatasource(NewDatasource, Bind.Path))
			{
				Datasource->OnDatasourceChanged.Add(Bind.Bind);
				// ReSharper disable once CppExpressionWithoutSideEffects
				Bind.Bind.ExecuteIfBound({ EUIDatasourceChangeEventKind::InitialBind, Datasource });
			}
		}
	}
}

void UUIDatasourceUserWidgetExtension::AddBinding(const FUIDataBind& Binding)
{
	Bindings.Add(Binding);
	if(FUIDatasource* OwnDatasource = Handle.Get())
	{
		if(FUIDatasource* Datasource = UUIDatasourceSubsystem::Get()->Pool.FindDatasource(OwnDatasource, Binding.Path))
		{
			Datasource->OnDatasourceChanged.Add(Binding.Bind);
			// ReSharper disable once CppExpressionWithoutSideEffects
			Binding.Bind.ExecuteIfBound({ EUIDatasourceChangeEventKind::InitialBind, Datasource });
		}
	}
}

void UUIDatasourceUserWidgetExtension::Destruct()
{
	SetDatasource({});
}

void UUIDatasourceWidgetBlueprintGeneratedClassExtension::Initialize(UUserWidget* UserWidget)
{
	Super::Initialize(UserWidget);

	UUIDatasourceUserWidgetExtension* DatasourceExtension = UUIDatasourceUserWidgetExtension::RegisterDatasourceExtension(UserWidget);
	for(FUIDataBindTemplate& Binding : Bindings)
	{
		UFunction* Func = UserWidget->FindFunction(Binding.BindDelegateName);
		if(ensure(Func))
		{
			FOnDatasourceChangedDelegateBP Delegate;
			Delegate.BindUFunction(UserWidget, Binding.BindDelegateName);
			DatasourceExtension->AddBinding({
				Delegate,
				Binding.Path
			});
		}
	}
}

void UUIDatasourceUserWidgetExtension::SetDatasource(FUIDatasourceHandle InHandle)
{
	if(InHandle != Handle)
	{
		IUIDatasourceEventHandler* EvtHandler = Cast<IUIDatasourceEventHandler>(GetUserWidget());
		if(EvtHandler)
		{
			EvtHandler->NativeOnDatasourceChanging(InHandle);
		}
		const FUIDatasourceHandle OldHandle = Handle;
		Handle = InHandle;
		UpdateBindings(OldHandle, Handle); // @NOTE: It's important that Handle has the right value, as binds might do something with it
		if(EvtHandler)
		{
			EvtHandler->NativeOnDatasourceChanged(Handle);
		}
	}
}

void UUIDatasourceUserWidgetExtension::SetUserWidgetDatasource(UUserWidget* UserWidget, FUIDatasourceHandle Handle)
{
	RegisterDatasourceExtension(UserWidget)->SetDatasource(Handle);
}

UUIDatasourceUserWidgetExtension* UUIDatasourceUserWidgetExtension::RegisterDatasourceExtension(UUserWidget* UserWidget)
{
	UUIDatasourceUserWidgetExtension* Extension = UserWidget->GetExtension<UUIDatasourceUserWidgetExtension>();
	if(!Extension)
	{
		Extension = UserWidget->AddExtension<UUIDatasourceUserWidgetExtension>();
	}
	return Extension;
}
