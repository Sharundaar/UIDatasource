#pragma once

#include "UIDatasourceListView.h"
#include "UIDatasourceTileView.generated.h"

UCLASS()
class UUIDatasourceTileView : public UUIDatasourceListView
{
	GENERATED_BODY()

public:
	virtual TSharedRef<STableViewBase> RebuildListWidget() override;

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	
protected:
	/** The height of each tile */
	UPROPERTY(EditAnywhere, Category = ListEntries)
	float EntryHeight = 128.f;

	/** The width of each tile */
	UPROPERTY(EditAnywhere, Category = ListEntries)
	float EntryWidth = 128.f;
	
	/** The method by which to align the tile entries in the available space for the tile view */
	UPROPERTY(EditAnywhere, Category = ListEntries)
	EListItemAlignment TileAlignment;

	TSharedPtr<STileView<FUIDatasourceHandle>> MyTileView;
};