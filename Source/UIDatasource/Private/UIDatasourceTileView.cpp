#include "UIDatasourceTileView.h"

TSharedRef<STableViewBase> UUIDatasourceTileView::RebuildListWidget()
{
	FTileViewConstructArgs Args;
	Args.bAllowFocus = true;
	Args.SelectionMode = ESelectionMode::None;
	Args.bClearSelectionOnClick = true;
	Args.ConsumeMouseWheel = EConsumeMouseWheel::WhenScrollingPossible;
	Args.bReturnFocusToSelection = true;
	Args.TileAlignment = TileAlignment;
	Args.EntryHeight = EntryHeight;
	Args.EntryWidth = EntryWidth;
	Args.bWrapDirectionalNavigation = true;
	Args.Orientation = Orient_Vertical;
	Args.ScrollBarStyle = &ScrollBarStyle;

	MyListView = MyTileView = ITypedUMGListView::ConstructTileView(this, ListItems, Args);
	MyTileView->SetOnEntryInitialized(SListView<FUIDatasourceHandle>::FOnEntryInitialized::CreateUObject(this, &UUIDatasourceTileView::HandleOnEntryInitialized));
	return MyTileView.ToSharedRef();
}

void UUIDatasourceTileView::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyTileView.Reset();
}
