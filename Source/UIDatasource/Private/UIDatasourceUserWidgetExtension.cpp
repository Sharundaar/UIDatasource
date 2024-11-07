// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasourceUserWidgetExtension.h"

#include "UIDatasourceSubsystem.h"
#include "Blueprint/UserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UIDatasourceUserWidgetExtension)

void IUIDatasourceEventHandler::NativeOnDatasourceChanging(FUIDatasourceHandle Handle)
{
	Execute_BP_OnDatasourceChanging(Cast<UObject>(this), Handle);
}

void IUIDatasourceEventHandler::NativeOnDatasourceChanged(FUIDatasourceHandle Handle)
{
	Execute_BP_OnDatasourceChanged(Cast<UObject>(this), Handle);
}

void UUIDatasourceUserWidgetExtension::AddBinding(const FUIDataBind& Binding)
{
	Linker.AddBinding(Binding);
}

void UUIDatasourceUserWidgetExtension::Construct()
{
	Linker.LinkGlobalBindings(true);
}

void UUIDatasourceUserWidgetExtension::Destruct()
{
	SetDatasource({});
	Linker.LinkGlobalBindings(false);
}

void UUIDatasourceWidgetBlueprintGeneratedClassExtension::Initialize(UUserWidget* UserWidget)
{
	UUIDatasourceUserWidgetExtension* DatasourceExtension = UUIDatasourceUserWidgetExtension::RegisterDatasourceExtension(UserWidget);
	for (FUIDataBindTemplate& Binding : Bindings)
	{
		UFunction* Func = UserWidget->FindFunction(Binding.BindDelegateName);
		if (ensureMsgf(Func, TEXT("Failed to find BindDelegateName function in this user widget, need to validate at blueprint compilation that the binding exists.")))
		{
			FOnDatasourceChangedDelegateBP Delegate;
			Delegate.BindUFunction(UserWidget, Binding.BindDelegateName);
			DatasourceExtension->AddBinding({
				Delegate,
				Binding.Path,
				Binding.BindType,
			});
		}
	}
}

#if WITH_EDITORONLY_DATA
void UUIDatasourceWidgetBlueprintGeneratedClassExtension::PreConstruct(UUserWidget* UserWidget, bool IsDesignTime)
{
	if(UUIDatasourceSubsystem::Get()->IsDesignerMockingEnabled())
	{
		if(UserWidget->IsDesignTime()) // Initialize isn't called in the designer, so call it manually here
		{
			Initialize(UserWidget);
		}
		FUIDatasource* MockDatasource = UUIDatasourceSubsystem::Get()->Pool.FindOrCreateDatasource(nullptr, "Mock");
		UUIDatasourceArchetype* Archetype = NewObject<UUIDatasourceArchetype>(this, UUIDatasourceArchetype::StaticClass(), "MockArchetype", RF_Transient);

		TArray<FUIDatasourceDescriptor> Descriptors;
		for(FUIDataBindTemplate& Binding : Bindings)
		{
			Descriptors.Add(Binding.Descriptor);
		}
		Archetype->SetChildren(Descriptors);
		Archetype->MockDatasource(MockDatasource);

		UserWidget->GetExtension<UUIDatasourceUserWidgetExtension>()->SetDatasource(MockDatasource);
	}
}
#endif

void FUIDatasourceLink::UpdateBindings(FUIDatasourceHandle OldHandle, FUIDatasourceHandle NewHandle)
{
	const FUIDatasourcePool& DatasourcePool = UUIDatasourceSubsystem::Get()->Pool;
	if(const FUIDatasource* OldDatasource = OldHandle.Get())
	{
		for(auto& Bind: Bindings)
		{
			if(FUIDatasource* Datasource = DatasourcePool.FindDatasource(OldDatasource, Bind.Path))
			{
#if WITH_UIDATASOURCE_MONITOR
				UUIDatasourceSubsystem::Get()->Monitor.UnbindDatasourceEvent(Datasource, Bind.Bind);
#else
				Datasource->OnDatasourceChanged.Remove(Bind.Bind);
#endif
			}
		}
	}

	if(const FUIDatasource* NewDatasource = NewHandle.Get())
	{
		for(const FUIDataBind& Bind : Bindings)
		{
			if(FUIDatasource* Datasource = DatasourcePool.FindDatasource(NewDatasource, Bind.Path))
			{
#if WITH_UIDATASOURCE_MONITOR
				UUIDatasourceSubsystem::Get()->Monitor.BindDatasourceEvent(Datasource, Bind.Bind);
#else
				Datasource->OnDatasourceChanged.Add(Bind.Bind);
#endif
				
				// ReSharper disable once CppExpressionWithoutSideEffects
				Bind.Bind.ExecuteIfBound({ EUIDatasourceChangeEventKind::InitialBind, Datasource });
			}
		}
	}
}

void FUIDatasourceLink::AddBinding(const FUIDataBind& Binding)
{
	if(Binding.BindType == EDatasourceBindType::Self)
	{
		Bindings.Add(Binding);
		if(const FUIDatasource* OwnDatasource = Handle.Get())
		{
			if(FUIDatasource* Datasource = UUIDatasourceSubsystem::Get()->Pool.FindDatasource(OwnDatasource, Binding.Path))
			{
#if WITH_UIDATASOURCE_MONITOR
				UUIDatasourceSubsystem::Get()->Monitor.BindDatasourceEvent(Datasource, Binding.Bind);
#else
				Datasource->OnDatasourceChanged.AddUnique(Binding.Bind);
#endif
				
				// ReSharper disable once CppExpressionWithoutSideEffects
				Binding.Bind.ExecuteIfBound({ EUIDatasourceChangeEventKind::InitialBind, Datasource });
			}
		}
	}

	if(Binding.BindType == EDatasourceBindType::Global)
	{
		GlobalBindings.Add(Binding);
	}
}

void FUIDatasourceLink::LinkGlobalBindings(bool bLink)
{
	
	// Resolve any global bindings here
	for (FUIDataBind& Binding : GlobalBindings)
	{
		if(FUIDatasource* Datasource = UUIDatasourceSubsystem::Get()->Pool.FindOrCreateDatasource(nullptr, Binding.Path))
		{
			if(bLink)
			{
#if WITH_UIDATASOURCE_MONITOR
				UUIDatasourceSubsystem::Get()->Monitor.BindDatasourceEvent(Datasource, Binding.Bind);
#else
				Datasource->OnDatasourceChanged.Add(Binding.Bind);
#endif

				// ReSharper disable once CppExpressionWithoutSideEffects
				Binding.Bind.ExecuteIfBound({ EUIDatasourceChangeEventKind::InitialBind, Datasource });
			}
			else
			{
#if WITH_UIDATASOURCE_MONITOR
				UUIDatasourceSubsystem::Get()->Monitor.UnbindDatasourceEvent(Datasource, Binding.Bind);
#else
				Datasource->OnDatasourceChanged.Remove(Binding.Bind);
#endif
			}
		}
	}
}

void UUIDatasourceUserWidgetExtension::SetDatasource(FUIDatasourceHandle InHandle)
{
	if(InHandle != Linker.Handle)
	{
		UUserWidget* UserWidget = GetUserWidget();
		const bool bUserWidgetImplementsEventHandler = UserWidget->Implements<UUIDatasourceEventHandler>(); 
		if(UserWidget && bUserWidgetImplementsEventHandler)
		{
			IUIDatasourceEventHandler::Execute_BP_OnDatasourceChanging(UserWidget, InHandle);
		}
		const FUIDatasourceHandle OldHandle = Linker.Handle;
		Linker.Handle = InHandle; // @NOTE: It's important that Handle has the right value, as binds might do something with it
		Linker.UpdateBindings(OldHandle, Linker.Handle);
		if(UserWidget && bUserWidgetImplementsEventHandler)
		{
			IUIDatasourceEventHandler::Execute_BP_OnDatasourceChanged(UserWidget, InHandle);
		}
	}
}

void UUIDatasourceUserWidgetExtension::SetUserWidgetDatasource(UUserWidget* UserWidget, FUIDatasourceHandle Handle)
{
	if (UserWidget)
	{
		RegisterDatasourceExtension(UserWidget)->SetDatasource(Handle);
	}
}

FUIDatasourceHandle UUIDatasourceUserWidgetExtension::GetUserWidgetDatasource(UUserWidget* UserWidget)
{
	return UserWidget ? RegisterDatasourceExtension(UserWidget)->GetDatasource() : FUIDatasourceHandle{};
}

UUIDatasourceUserWidgetExtension* UUIDatasourceUserWidgetExtension::RegisterDatasourceExtension(UUserWidget* UserWidget)
{
	if (!UserWidget)
	{
		return nullptr;
	}
	
	UUIDatasourceUserWidgetExtension* Extension = UserWidget->GetExtension<UUIDatasourceUserWidgetExtension>();
	if(!Extension)
	{
		Extension = UserWidget->AddExtension<UUIDatasourceUserWidgetExtension>();
		Extension->SetFlags(RF_Transient);
	}
	return Extension;
}
