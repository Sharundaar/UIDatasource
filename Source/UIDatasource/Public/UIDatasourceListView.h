// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UIDatasource.h"
#include "UIDatasourceHandle.h"
#include "UIDatasourceUserWidgetExtension.h"
#include "Components/ListViewBase.h"
#include "UIDatasourceListView.generated.h"

class UUIDatasourceUserWidgetExtension;

/**
 * Functionality (e.g. setting the value, testing invalid value) specialized for FUIDatasourceHandle.
 */
template <>
struct TListTypeTraits<FUIDatasourceHandle>
{
public:
	typedef FUIDatasourceHandle NullableType;

	using MapKeyFuncs = TDefaultMapHashableKeyFuncs<FUIDatasourceHandle, TSharedRef<ITableRow>, false>;
	using MapKeyFuncsSparse = TDefaultMapHashableKeyFuncs<FUIDatasourceHandle, FSparseItemInfo, false>;
	using SetKeyFuncs = DefaultKeyFuncs<FUIDatasourceHandle>;

	template<typename U>
	static void AddReferencedObjects(FReferenceCollector& Collector,
		TArray<FUIDatasourceHandle>& ItemsWithGeneratedWidgets,
		TSet<FUIDatasourceHandle>& SelectedItems,
		TMap<const U*, FUIDatasourceHandle>& WidgetToItemMap)
	{
	}

	static bool IsPtrValid(const FUIDatasourceHandle& InValue)
	{
		return true;
	}

	static void ResetPtr(FUIDatasourceHandle& InValue)
	{
		InValue = FUIDatasourceHandle();
	}

	static FUIDatasourceHandle MakeNullPtr()
	{
		return FUIDatasourceHandle();
	}

	static FUIDatasourceHandle NullableItemTypeConvertToItemType(const FUIDatasourceHandle& InValue)
	{
		return InValue;
	}

	static FString DebugDump(FUIDatasourceHandle InValue)
	{
		if (FUIDatasource* Datasource = InValue.Get())
		{
			FString Dmp;
			Datasource->GetPath(Dmp);
			return Dmp;
		}
		
		return TEXT("Invalid");
	}

	class SerializerType {};
};

template <>
struct TIsValidListItem<FUIDatasourceHandle>
{
	enum
	{
		Value = true
	};
};

/**
 * ListView implementation that interacts directly with Datasource Arrays
 * Use SetDatasource to set the array datasource this list will be fed data from
 * The list automatically sends the item datasources to the generated entry
 */
UCLASS()
class UIDATASOURCE_API UUIDatasourceListView : public UListViewBase, public ITypedUMGListView<FUIDatasourceHandle>
{
	GENERATED_BODY()
	
public:
	UUIDatasourceListView(const FObjectInitializer& ObjectInitializer);
	IMPLEMENT_TYPED_UMG_LIST(FUIDatasourceHandle, MyListView)

	// Callback to update the listview content
	UFUNCTION()
	void OnDatasourceChanged(FUIDatasourceChangeEventArgs EventArgs);

	// Set the datasource linked to this list, expected to be an array datasource
	UFUNCTION(BlueprintCallable)
	void SetDatasource(FUIDatasourceHandle Datasource);

	void HandleOnEntryInitialized(FUIDatasourceHandle DatasourceHandle, const TSharedRef<ITableRow>& TableRow) const;
	
	virtual TSharedRef<STableViewBase> RebuildListWidget() override;

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
#if WITH_EDITOR
	virtual void OnRefreshDesignerItems() override;
#endif
	
protected:
	virtual UUserWidget& OnGenerateEntryWidgetInternal(FUIDatasourceHandle Item,
		TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable) override;

	TSharedPtr<SListView<FUIDatasourceHandle>> MyListView;
	TArray<FUIDatasourceHandle> ListItems;

	// Style for the listview
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ListView, meta = (DisplayName = "Style"))
	FTableViewStyle WidgetStyle;

	// Scrollbar Style
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ListView)
	FScrollBarStyle ScrollBarStyle;

	// Minimum number of elements the list should show
	UPROPERTY(EditAnywhere, Category = ListEntries)
	int32 MinElementCount = 0;
	
#if WITH_EDITORONLY_DATA
	// Archetype this list view items uses
	UPROPERTY(EditAnywhere, Category = ListEntries)
	UUIDatasourceArchetype* Archetype;

	// Path used to generate preview datasources
	UPROPERTY(EditAnywhere, Category = ListEntries)
	FString PreviewDatasourcePath = "ListPreview";
#endif
	
	FUIDatasourceLink Linker;
};
