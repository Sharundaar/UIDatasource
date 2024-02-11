﻿#include "UIDatasourceEditor.h"

#include "GraphEditAction.h"
#include "K2Node_UIDatasourceSingleBinding.h"
#include "PropertyEditorModule.h"
#include "UIDatasourceWidgetBlueprintExtension.h"
#include "UMGEditorModule.h"
#include "BlueprintModes/WidgetBlueprintApplicationModes.h"

#define LOCTEXT_NAMESPACE "FUIDatasourceEditorModule"

namespace FUIDatasourceEditorModulePrivate
{
	const static FName PropertyEditor("PropertyEditor");
	const static FName AnimationTabSummonerTabID(TEXT("Animations"));
}

class SUIDatasourcePanel : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SUIDatasourcePanel) {}
	SLATE_END_ARGS()

	void UpdateContent();
	FReply HandleCreateUIDatasourceClicked();
	void HandleExtensionAdded(UBlueprintExtension* BlueprintExtension);
	void OnBPUbergraphChanged(const FEdGraphEditAction& EdGraphEditAction);
	void Construct(const FArguments& InArgs, TSharedPtr<FWidgetBlueprintEditor> Editor);
	SUIDatasourcePanel() = default;
	virtual ~SUIDatasourcePanel() override;

	TWeakObjectPtr<UUIDatasourceWidgetBlueprintExtension> UIDatasourceExtension;
	TWeakPtr<FWidgetBlueprintEditor> WeakWidgetEditor;
	TSharedPtr<IDetailsView> DatasourceArchetypeDetailsView;
	TMap<TWeakObjectPtr<UEdGraph>, FDelegateHandle> GraphChangedDelegates;
	FDelegateHandle BindingChangedEventHandler;

	struct FListViewData
	{
		// FUIDatasourceArchetypeElement Element;
		TWeakObjectPtr<UK2Node_UIDatasourceSingleBinding> Node;
	};
	TArray<TSharedPtr<FListViewData>> ListData;
	TSharedRef<ITableRow> GenerateListRow(TSharedPtr<FListViewData> InItem, const TSharedRef<STableViewBase>& InOwningTable);
};

void SUIDatasourcePanel::UpdateContent()
{
	if (!DatasourceArchetypeDetailsView)
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		FDetailsViewArgs DetailsViewArgs = {};
		DetailsViewArgs.bShowPropertyMatrixButton = false;
		DetailsViewArgs.bShowOptions = false;
		DetailsViewArgs.NotifyHook = this;

		DatasourceArchetypeDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	}

	UUIDatasourceWidgetBlueprintExtension* Extension = UIDatasourceExtension.Get();
	DatasourceArchetypeDetailsView->SetObject(Extension);

	/* @TODO: Bring back DatasourceArchetype
	if (Extension && Extension->DatasourceArchetype)
	{
		UWidgetBlueprint* WidgetBlueprint = Extension->GetWidgetBlueprint();
		check(WidgetBlueprint);

		ListData.Empty();
		TArray<const UUIDatasourceArchetype*> Stack;
		Stack.Add(Extension->DatasourceArchetype);
		while(!Stack.IsEmpty())
		{
			const UUIDatasourceArchetype* Archetype = Stack.Pop();
			for (const FUIDatasourceArchetypeElement& Child : Archetype->Children)
			{
				if(Child.IsInlineArchetype() && Child.Archetype)
				{
					Stack.Push(Child.Archetype);
					continue;
				}
				
				const TSharedPtr<FListViewData>& Elem = ListData.Add_GetRef(MakeShared<FListViewData>());
				Elem->Element = Child;
				Elem->Node = nullptr;
			}
		}

		TArray<UK2Node_UIDatasourceSingleBinding*> Nodes;
		for (UEdGraph* Graph : WidgetBlueprint->UbergraphPages)
		{
			Graph->GetNodesOfClass(Nodes);
		}

		for (UK2Node_UIDatasourceSingleBinding* Node : Nodes)
		{
			TSharedPtr<FListViewData>* FoundElement = ListData.FindByPredicate([Node](TSharedPtr<FListViewData> Elem)
			{
				return !Elem->Node.IsValid() && Elem->Element.Name == FName(Node->GetBindPath(), FNAME_Find);
			});
			if (FoundElement)
			{
				(*FoundElement)->Node = Node;
			}
			else
			{
				const TSharedPtr<FListViewData>& Elem = ListData.Add_GetRef(MakeShared<FListViewData>());
				Elem->Element = FUIDatasourceArchetypeElement {
					FName(Node->GetBindPath()),
					Node->GetBindType(),
					Node->GetEnumPath(),
					nullptr,
					EUIDatasourceArchetypeInstanciationMethod::AsChild,
					nullptr,
				};
				Elem->Node = Node;
			}
		}
	}
	*/
	
	ChildSlot[
		SNew(SOverlay)
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			SNew(SButton)
			.Visibility_Lambda([this]()
			{
				if (UIDatasourceExtension.IsValid())
				{
					return EVisibility::Collapsed;
				}
				return EVisibility::Visible;
			})
			.OnClicked(this, &SUIDatasourcePanel::HandleCreateUIDatasourceClicked)
			[
				SNew(STextBlock).Text(INVTEXT("Create UIDatasource Extension"))
			]
		]
		+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			SNew(SVerticalBox)
			.Visibility_Lambda([this]()
			{
				if (UIDatasourceExtension.IsValid())
				{
					return EVisibility::Visible;
				}
				return EVisibility::Collapsed;
			})
			+ SVerticalBox::Slot().AutoHeight()
			[
				DatasourceArchetypeDetailsView ? DatasourceArchetypeDetailsView.ToSharedRef() : SNullWidget::NullWidget
			]
			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SNew(SListView<TSharedPtr<FListViewData>>)
				.ListItemsSource(&ListData)
				.SelectionMode(ESelectionMode::None)
				.ItemHeight(20.0f)
				.OnGenerateRow(this, &SUIDatasourcePanel::GenerateListRow)
			]
		]
	];
}

FReply SUIDatasourcePanel::HandleCreateUIDatasourceClicked()
{
	if (TSharedPtr<FWidgetBlueprintEditor> WidgetEditor = WeakWidgetEditor.Pin())
	{
		UWidgetBlueprint* WidgetBlueprint = WidgetEditor->GetWidgetBlueprintObj();
		check(WidgetBlueprint);

		UUIDatasourceWidgetBlueprintExtension::RequestExtension<UUIDatasourceWidgetBlueprintExtension>(WidgetBlueprint);
	}
	return FReply::Handled();
}

void SUIDatasourcePanel::HandleExtensionAdded(UBlueprintExtension* BlueprintExtension)
{
	if (UUIDatasourceWidgetBlueprintExtension* UIDatasourceExtensionPtr = Cast<UUIDatasourceWidgetBlueprintExtension>(BlueprintExtension))
	{
		UWidgetBlueprint* WidgetBlueprint = UIDatasourceExtensionPtr->GetWidgetBlueprint();

		if (WidgetBlueprint)
		{
			WidgetBlueprint->OnExtensionAdded.RemoveAll(this);

			UIDatasourceExtension = UIDatasourceExtensionPtr;
			BindingChangedEventHandler = UIDatasourceExtensionPtr->OnBindingChanged.AddRaw(this, &SUIDatasourcePanel::UpdateContent);
			UpdateContent();
		}
	}
}

void SUIDatasourcePanel::OnBPUbergraphChanged(const FEdGraphEditAction& EdGraphEditAction)
{
	bool bReconstructView = false;

	if(EdGraphEditAction.Action & (GRAPHACTION_AddNode | GRAPHACTION_RemoveNode))
	{
		for (auto& Node : EdGraphEditAction.Nodes)
		{
			if (Cast<UK2Node_UIDatasourceSingleBinding>(Node))
			{
				bReconstructView = true;
				break;
			}
		}
	}
	
	if (bReconstructView)
	{
		UpdateContent();
	}
}

void SUIDatasourcePanel::Construct(const FArguments& InArgs, TSharedPtr<FWidgetBlueprintEditor> Editor)
{
	WeakWidgetEditor = Editor;

	UWidgetBlueprint* WidgetBlueprint = Editor->GetWidgetBlueprintObj();
	check(WidgetBlueprint);

	for (UEdGraph* Graph : WidgetBlueprint->UbergraphPages)
	{
		GraphChangedDelegates.Add(Graph, Graph->AddOnGraphChangedHandler(FOnGraphChanged::FDelegate::CreateRaw(this, &SUIDatasourcePanel::OnBPUbergraphChanged)));
	}

	UUIDatasourceWidgetBlueprintExtension* UIDatasourceExtensionPtr = UUIDatasourceWidgetBlueprintExtension::GetExtension<UUIDatasourceWidgetBlueprintExtension>(WidgetBlueprint);
	UIDatasourceExtension = UIDatasourceExtensionPtr;
	if (!UIDatasourceExtensionPtr)
	{
		WidgetBlueprint->OnExtensionAdded.AddSP(this, &SUIDatasourcePanel::HandleExtensionAdded);
	}
	else
	{
		BindingChangedEventHandler = UIDatasourceExtension->OnBindingChanged.AddRaw(this, &SUIDatasourcePanel::UpdateContent);
	}
	
	UpdateContent();
}

SUIDatasourcePanel::~SUIDatasourcePanel()
{
	if (TSharedPtr<FWidgetBlueprintEditor> WidgetEditor = WeakWidgetEditor.Pin())
	{
		if (UWidgetBlueprint* WidgetBlueprint = WidgetEditor->GetWidgetBlueprintObj())
		{
			WidgetBlueprint->OnExtensionAdded.RemoveAll(this);
		}
	}

	for (auto& [WeakGraph, Delegate] : GraphChangedDelegates)
	{
		if (UEdGraph* Graph = WeakGraph.Get())
		{
			Graph->RemoveOnGraphChangedHandler(Delegate);
		}
		Delegate.Reset();
	}
	GraphChangedDelegates.Empty();

	if(auto Extension = UIDatasourceExtension.Get())
	{
		Extension->OnBindingChanged.Remove(BindingChangedEventHandler);
	}
	BindingChangedEventHandler.Reset();
}

TSharedRef<ITableRow> SUIDatasourcePanel::GenerateListRow(TSharedPtr<FListViewData> InItem, const TSharedRef<STableViewBase>& InOwningTable)
{
	return SNew(STableRow<TSharedPtr<FListViewData>>, InOwningTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("TODO")/*@TODO: InItem->Element.Name.ToString()*/))
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SButton)
			.Text_Lambda([WeakNode = InItem->Node]()
			{
				if (UK2Node_UIDatasourceSingleBinding* Node = WeakNode.Get())
				{
					return INVTEXT("Go to Binding");
				}
				else
				{
					return INVTEXT("Create Binding");
				}
			})
			.OnClicked_Lambda([WeakEditor = WeakWidgetEditor, WeakExtension = UIDatasourceExtension, InItem]()
			{
				TSharedPtr<FWidgetBlueprintEditor> Editor = WeakEditor.Pin();
				UUIDatasourceWidgetBlueprintExtension* Extension = WeakExtension.Get();
				if(!Editor)
				{
					return FReply::Unhandled();
				}

				if(!Extension)
				{
					return FReply::Unhandled();
				}
				
				if (UK2Node_UIDatasourceSingleBinding* Node = InItem->Node.Get())
				{
					Editor->JumpToNode(Node);
				}
				else
				{
					UWidgetBlueprint* WidgetBlueprint = Editor->GetWidgetBlueprintObj();
					UEdGraph* TargetGraph = WidgetBlueprint && WidgetBlueprint->UbergraphPages.Num() > 0 ? WidgetBlueprint->UbergraphPages[0] : nullptr;

					if(TargetGraph)
					{
						UK2Node_UIDatasourceSingleBinding* SingleBindingNode = NewObject<UK2Node_UIDatasourceSingleBinding>(TargetGraph);
						/* @TODO:
						SingleBindingNode->SetData(
							Extension->DatasourceArchetype,
							InItem->Element.Name.ToString(),
							InItem->Element.Type,
							InItem->Element.EnumPath);
						*/
						SingleBindingNode->CreateNewGuid();
						SingleBindingNode->PostPlacedNewNode();
						SingleBindingNode->AllocateDefaultPins();
						const FVector2D NewPosition = TargetGraph->GetGoodPlaceForNewNode();
						SingleBindingNode->NodePosX = NewPosition.X;
						SingleBindingNode->NodePosY = NewPosition.Y;

						TargetGraph->AddNode(SingleBindingNode, true, true);
					}
				}

				return FReply::Handled();
			})
		]
	];
}

struct FUIDatasourceSummoner : public FWorkflowTabFactory
{
public:
	static const FName TabID;
	FUIDatasourceSummoner(const TSharedPtr<FWidgetBlueprintEditor>& BlueprintEditor)
		: FWorkflowTabFactory(TabID, BlueprintEditor)
		, WeakBlueprintEditor(BlueprintEditor)
	{
		TabLabel = INVTEXT("UIDatasources");
		TabIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.DataLayer");

		bIsSingleton = true;
		ViewMenuDescription = INVTEXT("Datasources");
		ViewMenuTooltip = INVTEXT("Show the datasources panel");
	}

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override
	{
		TSharedPtr<FWidgetBlueprintEditor> BlueprintEditorPtr = StaticCastSharedPtr<FWidgetBlueprintEditor>(WeakBlueprintEditor.Pin());
		return SNew(SUIDatasourcePanel, BlueprintEditorPtr).AddMetaData<FTagMetaData>(FTagMetaData(TEXT("UIDatasourcePanel")));
	}

protected:
	TWeakPtr<FWidgetBlueprintEditor> WeakBlueprintEditor;
};

const FName FUIDatasourceSummoner::TabID = "UIDatasources";

void FUIDatasourceEditorModule::StartupModule()
{
	IUMGEditorModule& UMGEditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>("UMGEditor");
	UMGEditorModule.OnRegisterTabsForEditor().AddStatic(&FUIDatasourceEditorModule::HandleRegisterBlueprintEditorTab);
}

void FUIDatasourceEditorModule::ShutdownModule()
{
    
}

void FUIDatasourceEditorModule::HandleRegisterBlueprintEditorTab(const FWidgetBlueprintApplicationMode& ApplicationMode, FWorkflowAllowedTabSet& TabFactories)
{
	if(ApplicationMode.GetModeName() == FWidgetBlueprintApplicationModes::GraphMode)
	{
		TabFactories.RegisterFactory(MakeShared<FUIDatasourceSummoner>(ApplicationMode.GetBlueprintEditor()));
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FUIDatasourceEditorModule, UIDatasourceEditor)