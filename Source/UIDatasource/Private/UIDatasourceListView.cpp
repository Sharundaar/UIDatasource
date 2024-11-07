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
	// Goals here is to pull out all array items out of the array datasource and push them to the ListItems array, which our underlying SListItem consumes.
	// To do so we just parse the array datasource children manually for efficiency, this avoids a potential N^2 lookup if we went by name, this uses the assumption
	// that datasource elements array named numbers represents their index in the array.
	// We also want to fill up the ListItems array with invalid datasource that hashes out to different pair buckets, this is why you'll see different generations setup
	// just know that the generation isn't meant to mean anything, we only really care that the datasource registers as "Invalid", and the they are different from each other
	// for the purpose of the SListView hashing mechanism.
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

#if WITH_EDITOR
void UUIDatasourceListView::OnRefreshDesignerItems()
{
	// This is purely for the designer preview, the generational index shift is based on the same principle
	// outlined in OnDatasourceChanged, we just want different "Invalid" datasource to hash to different buckets
	// If we define an Archetype, we can use that to generate editor time datasources and link them to the preview.
	// We also use MinElementCount to supplement the preview element count to be more accurate with runtime use.
	if(!Archetype)
	{
		ListItems.Reset();
		int32 Num = 0;
		RefreshDesignerItems<FUIDatasourceHandle>(ListItems, [this, &Num]()
		{
			FUIDatasourceHandle Handle;
			Handle.Id = UIDatasource_PackId(++Num, EUIDatasourceId::Invalid);
			return Handle;
		});
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
		for(int32 Idx = ListItems.Num(); Idx < MinElementCount; ++Idx)
		{
			FUIDatasourceHandle Handle;
			Handle.Id = UIDatasource_PackId(Idx, EUIDatasourceId::Invalid);
			ListItems.Add(Handle);
		}
	}
}
#endif

UUserWidget& UUIDatasourceListView::OnGenerateEntryWidgetInternal(FUIDatasourceHandle Item,
                                                                  TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable)
{
	return GenerateTypedEntry<UUserWidget, SObjectTableRow<FUIDatasourceHandle>>(DesiredEntryClass, OwnerTable);	
}
