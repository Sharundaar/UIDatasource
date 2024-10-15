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
 * 
 */
UCLASS()
class UIDATASOURCE_API UUIDatasourceListView : public UListViewBase, public ITypedUMGListView<FUIDatasourceHandle>
{
	GENERATED_BODY()

	UUIDatasourceListView(const FObjectInitializer& ObjectInitializer);
	IMPLEMENT_TYPED_UMG_LIST(FUIDatasourceHandle, MyListView)

public:
	UFUNCTION()
	void OnDatasourceChanged(FUIDatasourceChangeEventArgs EventArgs);
	
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ListView, meta = (DisplayName = "Style"))
	FTableViewStyle WidgetStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ListView)
	FScrollBarStyle ScrollBarStyle;

	UPROPERTY(EditAnywhere)
	int32 MinElementCount = 0;
	
#if WITH_EDITORONLY_DATA
	// Archetype this list view items uses
	UPROPERTY(EditAnywhere)
	UUIDatasourceArchetype* Archetype;

	// Path used to generate preview datasources
	UPROPERTY(EditAnywhere)
	FString PreviewDatasourcePath = "ListPreview";
#endif
	
	FUIDatasourceLink Linker;
};
