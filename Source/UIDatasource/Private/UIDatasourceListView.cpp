// Fill out your copyright notice in the Description page of Project Settings.


#include "UIDatasourceListView.h"

#include "UIDatasourceSubsystem.h"
#include "UIDatasourceUserWidgetExtension.h"

static FTableViewStyle* DefaultListViewStyle = nullptr;
static FScrollBarStyle* DefaultListViewScrollBarStyle = nullptr;

UUIDatasourceListView::UUIDatasourceListView(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FOnDatasourceChangedDelegateBP Delegate;
	Delegate.BindDynamic(this, &UUIDatasourceListView::OnDatasourceChanged);
	Linker.AddBinding(FUIDataBind{
		Delegate,
		"",
		EDatasourceBindType::Self
	});

	if (DefaultListViewStyle == nullptr)
	{
		DefaultListViewStyle = new FTableViewStyle(FUMGCoreStyle::Get().GetWidgetStyle<FTableViewStyle>("ListView"));

		// Unlink UMG default colors.
		DefaultListViewStyle->UnlinkColors();
	}

	if (DefaultListViewScrollBarStyle == nullptr)
	{
		DefaultListViewScrollBarStyle = new FScrollBarStyle(FUMGCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("Scrollbar"));

		// Unlink UMG default colors.
		DefaultListViewScrollBarStyle->UnlinkColors();
	}

	WidgetStyle = *DefaultListViewStyle;
	ScrollBarStyle = *DefaultListViewScrollBarStyle;
}

void UUIDatasourceListView::OnDatasourceChanged(FUIDatasourceChangeEventArgs EventArgs)
{
	ListItems.Reset();
	FUIDatasource* Datasource = EventArgs.Handle.Get();
	if (FUIArrayDatasource* ArrayDatasource = FUIArrayDatasource::Cast(Datasource))
	{
		FUIDatasourcePool* Pool = ArrayDatasource->GetPool();
		const int32 ArrayNum = ArrayDatasource->GetNum();
		ListItems.SetNumZeroed(FMath::Max(MinElementCount, ArrayNum), false);
		for (const FUIDatasource* Child = Pool->GetDatasourceById(ArrayDatasource->FirstChild); Child; Child = Pool->GetDatasourceById(Child->NextSibling))
		{
			const int32 ChildIndex = Child->Name.GetNumber() - 1; // FName numbers starts at 1, so bring that back to [0;Num] space
			if (ChildIndex < ArrayNum)
			{
				ListItems[ChildIndex] = Child;
			}
		}

		for (int32 Idx = ArrayNum; Idx < ListItems.Num(); ++Idx)
		{
			FUIDatasourceHandle Handle;
			Handle.Id = UIDatasource_PackId(Idx+1, EUIDatasourceId::Invalid);
			ListItems[Idx] = Handle;
		}
	}
	
	RequestRefresh();
}

void UUIDatasourceListView::SetDatasource(FUIDatasourceHandle Datasource)
{
	FUIDatasourceHandle OldHandle = Linker.Handle;
	Linker.Handle = Datasource;
	Linker.UpdateBindings(OldHandle, Linker.Handle);
}

void UUIDatasourceListView::HandleOnEntryInitialized(FUIDatasourceHandle DatasourceHandle,
	const TSharedRef<ITableRow>& TableRow) const
{
	UUserWidget* UserWidget = GetEntryWidgetFromItem<UUserWidget>(DatasourceHandle);
	UUIDatasourceUserWidgetExtension::SetUserWidgetDatasource(UserWidget, DatasourceHandle);
}

TSharedRef<STableViewBase> UUIDatasourceListView::RebuildListWidget()
{
	FListViewConstructArgs Args;
	Args.bAllowFocus = true;
	Args.SelectionMode = ESelectionMode::None;
	Args.bClearSelectionOnClick = false;
	Args.ConsumeMouseWheel = EConsumeMouseWheel::WhenScrollingPossible;
	Args.Orientation = Orient_Vertical;
	Args.ListViewStyle = &WidgetStyle;
	Args.ScrollBarStyle = &ScrollBarStyle;

	if (ListItems.Num() < MinElementCount)
	{
		ListItems.SetNumZeroed(MinElementCount);
		for (int32 Idx = 0; Idx < MinElementCount; ++Idx)
		{
			FUIDatasourceHandle Handle;
			Handle.Id = UIDatasource_PackId(Idx+1, EUIDatasourceId::Invalid);
			ListItems[Idx] = Handle;
		}
	}
	
	MyListView = ITypedUMGListView::ConstructListView(this, ListItems, Args);
	MyListView->SetOnEntryInitialized(SListView<FUIDatasourceHandle>::FOnEntryInitialized::CreateUObject(this, &UUIDatasourceListView::HandleOnEntryInitialized));
	
	return MyListView.ToSharedRef();
}

void UUIDatasourceListView::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyListView.Reset();
}

void UUIDatasourceListView::OnRefreshDesignerItems()
{
	if(!Archetype)
	{
		ListItems.Reset();
		RefreshDesignerItems<FUIDatasourceHandle>(ListItems, [this]() { return FUIDatasourceHandle(); });
	}
	else
	{
		ListItems.Reset();
		FUIDatasourcePool& DatasourcePool = UUIDatasourceSubsystem::Get()->Pool;
		FUIDatasource* PreviewDatasource = DatasourcePool.FindOrCreateDatasource(nullptr, *PreviewDatasourcePath);
		FUIArrayDatasource* PreviewDatasourceArray = FUIArrayDatasource::Make(PreviewDatasource);
		RefreshDesignerItems<FUIDatasourceHandle>(ListItems, [this, PreviewDatasourceArray]()
		{
			FUIDatasource* ItemDatasource = PreviewDatasourceArray->Append();
			Archetype->MockDatasource(ItemDatasource);
			return ItemDatasource;
		});
	}
}

UUserWidget& UUIDatasourceListView::OnGenerateEntryWidgetInternal(FUIDatasourceHandle Item,
                                                                  TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable)
{
	return GenerateTypedEntry<UUserWidget, SObjectTableRow<FUIDatasourceHandle>>(DesiredEntryClass, OwnerTable);	
}
