// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasourceEditor.h"

#include "GraphEditAction.h"
#include "K2Node_UIDatasourceSingleBinding.h"
#include "PropertyEditorModule.h"
#include "UIDatasourceArchetype.h"
#include "UIDatasourceSubsystem.h"
#include "UIDatasourceWidgetBlueprintExtension.h"
#include "UMGEditorModule.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "BlueprintModes/WidgetBlueprintApplicationModes.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Views/STreeView.h"

#define LOCTEXT_NAMESPACE "FUIDatasourceEditorModule"

//////////////////////////////////////////////////////////////////
/// SUIDatasourcePanel

class SUIDatasourcePanel : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SUIDatasourcePanel) {}
	SLATE_END_ARGS()

	void UpdateContent();
	FReply HandleCreateUIDatasourceClicked();
	void HandleExtensionAdded(UBlueprintExtension* BlueprintExtension);
	void OnBPUbergraphChanged(const FEdGraphEditAction& EdGraphEditAction);
	void OnBPUbergraphPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent, const FString& String);
	void Construct(const FArguments& InArgs, TSharedPtr<FWidgetBlueprintEditor> Editor);
	SUIDatasourcePanel() = default;
	virtual ~SUIDatasourcePanel() override;

	TWeakObjectPtr<UUIDatasourceWidgetBlueprintExtension> UIDatasourceExtension;
	TWeakPtr<FWidgetBlueprintEditor> WeakWidgetEditor;
	TSharedPtr<IDetailsView> DatasourceArchetypeDetailsView;
	TMap<TWeakObjectPtr<UEdGraph>, FDelegateHandle> GraphChangedDelegates;
	TMap<TWeakObjectPtr<UEdGraph>, FDelegateHandle> GraphPropertyChangedDelegates;
	FDelegateHandle BindingChangedEventHandler;

	struct FListViewData
	{
		FUIDatasourceDescriptor Descriptor;
		TWeakObjectPtr<UK2Node_UIDatasourceSingleBinding> Node; // Can be nullptr
	};
	TArray<TSharedPtr<FListViewData>> ArchetypeListData;
	TArray<TSharedPtr<FListViewData>> CustomListData;
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
		DetailsViewArgs.bHideSelectionTip = true;
		DetailsViewArgs.bAllowSearch = false;

		DatasourceArchetypeDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	}

	UUIDatasourceWidgetBlueprintExtension* Extension = UIDatasourceExtension.Get();
	DatasourceArchetypeDetailsView->SetObject(Extension);

	if (Extension && Extension->Archetype)
	{
		UWidgetBlueprint* WidgetBlueprint = Extension->GetWidgetBlueprint();
		check(WidgetBlueprint);

		ArchetypeListData.Empty();
		CustomListData.Empty();
		
		TArray<const UUIDatasourceArchetype*> Stack;
		Stack.Add(Extension->Archetype);
		while(!Stack.IsEmpty())
		{
			const UUIDatasourceArchetype* Archetype = Stack.Pop();
			for (const FUIDatasourceDescriptor& Descriptor : Archetype->GetDescriptors())
			{
				if(Descriptor.IsInlineArchetype())
				{
					Stack.Push(Descriptor.Archetype);
					continue;
				}
				
				const TSharedPtr<FListViewData>& Elem = ArchetypeListData.Add_GetRef(MakeShared<FListViewData>());
				Elem->Descriptor = Descriptor;
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
			TSharedPtr<FListViewData>* FoundElement = ArchetypeListData.FindByPredicate([Node](TSharedPtr<FListViewData> Elem)
			{
				return !Elem->Node.IsValid() && Elem->Descriptor.Path == Node->Path;
			});
			if (FoundElement)
			{
				(*FoundElement)->Node = Node;
			}
			else
			{
				const TSharedPtr<FListViewData>& Elem = CustomListData.Add_GetRef(MakeShared<FListViewData>());
				Elem->Descriptor = FUIDatasourceDescriptor {
					Node->Path,
					Node->Type,
					Node->EnumPath,
					nullptr,
					EUIDatasourceArchetypeImportMethod::AsChild
				};
				Elem->Node = Node;
			}
		}
	}
	
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
			SNew(SScrollBox).Orientation(Orient_Vertical).Visibility_Lambda([this]()
			{
				if (UIDatasourceExtension.IsValid())
				{
					return EVisibility::Visible;
				}
				return EVisibility::Collapsed;
			})
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					DatasourceArchetypeDetailsView ? DatasourceArchetypeDetailsView.ToSharedRef() : SNullWidget::NullWidget
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().AutoWidth() [ SNew(STextBlock).Text(INVTEXT("Mock Datasource")) ]
					+SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SCheckBox)
						.IsChecked_Lambda([]() { return UUIDatasourceSubsystem::Get()->IsDesignerMockingEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
						.OnCheckStateChanged_Lambda([](ECheckBoxState State) {
							UUIDatasourceSubsystem::Get()->EnableDesignerMocking(State == ECheckBoxState::Checked);
						})
					]
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SSeparator).Orientation(Orient_Vertical).Thickness(4.0f)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SListView<TSharedPtr<FListViewData>>)
					.ListItemsSource(&ArchetypeListData)
					.SelectionMode(ESelectionMode::None)
					.ItemHeight(20.0f)
					.OnGenerateRow(this, &SUIDatasourcePanel::GenerateListRow)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SSeparator).Orientation(Orient_Vertical).Thickness(4.0f)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SListView<TSharedPtr<FListViewData>>)
					.ListItemsSource(&CustomListData)
					.SelectionMode(ESelectionMode::None)
					.ItemHeight(20.0f)
					.OnGenerateRow(this, &SUIDatasourcePanel::GenerateListRow)
				]
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

void SUIDatasourcePanel::OnBPUbergraphPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent, const FString& PropertyName)
{
	bool bReconstructView = false;
	const int32 NumObject = PropertyChangedEvent.GetNumObjectsBeingEdited();
	for(int32 Idx = 0; Idx<NumObject; ++Idx)
	{
		const UObject* Obj = PropertyChangedEvent.GetObjectBeingEdited(Idx);
		if(const UK2Node_UIDatasourceSingleBinding* BindingNode = Cast<UK2Node_UIDatasourceSingleBinding>(Obj))
		{
			bReconstructView = true;
			break;
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
		GraphPropertyChangedDelegates.Add(Graph, Graph->AddPropertyChangedNotifier(FOnPropertyChanged::FDelegate::CreateRaw(this, &SUIDatasourcePanel::OnBPUbergraphPropertyChanged)));
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
	
	for (auto& [WeakGraph, Delegate] : GraphPropertyChangedDelegates)
	{
		if (UEdGraph* Graph = WeakGraph.Get())
		{
			Graph->RemovePropertyChangedNotifier(Delegate);
		}
		Delegate.Reset();
	}
	GraphPropertyChangedDelegates.Empty();

	if(auto Extension = UIDatasourceExtension.Get())
	{
		Extension->OnBindingChanged.Remove(BindingChangedEventHandler);
	}
	BindingChangedEventHandler.Reset();
}

FEdGraphPinType GetPinTypeForDescriptor(const FUIDatasourceDescriptor& Descriptor)
{
	FEdGraphPinType PinType = {};
	
	switch (Descriptor.Type)
	{
	case EUIDatasourceValueType::Int:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Int;
		break;
	case EUIDatasourceValueType::Bool:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
		break;
	case EUIDatasourceValueType::Float:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Real;
		PinType.PinSubCategory = UEdGraphSchema_K2::PC_Float;
		break;
	case EUIDatasourceValueType::Text:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Text;
		break;
	case EUIDatasourceValueType::Name:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Name;
		break;
	case EUIDatasourceValueType::String:
		PinType.PinCategory = UEdGraphSchema_K2::PC_String;
		break;
	case EUIDatasourceValueType::GameplayTag:
		PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		PinType.PinSubCategoryObject = FGameplayTag::StaticStruct();
		break;
	case EUIDatasourceValueType::Image:
		PinType.PinCategory = UEdGraphSchema_K2::PC_SoftObject;
		PinType.PinSubCategoryObject = UTexture2D::StaticClass();
		break;
	case EUIDatasourceValueType::Enum:
		if (!Descriptor.EnumPath.IsEmpty())
		{
			PinType.PinCategory = UEdGraphSchema_K2::PC_Byte;
			PinType.PinSubCategoryObject = FindObject<UEnum>(nullptr, *Descriptor.EnumPath);
		}
		else
		{
			PinType.PinCategory = UEdGraphSchema_K2::PC_Byte;
		}
		break;
	case EUIDatasourceValueType::Archetype:
	case EUIDatasourceValueType::Void: // default to sending the datasource handle
		PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		PinType.PinSubCategoryObject = FUIDatasourceHandle::StaticStruct();
		break;
	default: checkf(false, TEXT("Missing enum value implementation %d"), Descriptor.Type);
	}
	
	return PinType;
}

TSharedRef<ITableRow> SUIDatasourcePanel::GenerateListRow(TSharedPtr<FListViewData> InItem, const TSharedRef<STableViewBase>& InOwningTable)
{
	const FEdGraphPinType PinType = GetPinTypeForDescriptor(InItem->Descriptor);
	const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
	
	return SNew(STableRow<TSharedPtr<FListViewData>>, InOwningTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SButton)
				.ButtonStyle( FAppStyle::Get(), "SimpleButton" )
				.ContentPadding(0)
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
							SingleBindingNode->SourceArchetype = Extension->Archetype;
							SingleBindingNode->Path = InItem->Descriptor.Path;
							SingleBindingNode->Type = InItem->Descriptor.Type;
							SingleBindingNode->EnumPath = InItem->Descriptor.EnumPath;
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
				[ 
					SNew( SImage )
					.Image_Lambda([WeakNode = InItem->Node]()
					{
						if (UK2Node_UIDatasourceSingleBinding* Node = WeakNode.Get())
						{
							return FAppStyle::GetBrush("Icons.Search");
						}
						else
						{
							return FAppStyle::GetBrush("Icons.PlusCircle");
						}
					})
					.ColorAndOpacity( FSlateColor::UseForeground() )
				]
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SImage)
			.Image(FBlueprintEditorUtils::GetIconFromPin(PinType, true))
			.ColorAndOpacity(Schema->GetPinTypeColor(PinType))
		]
		+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
		[
			SNew(STextBlock).Text(FText::FromString(InItem->Descriptor.Path))
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
		return SNew(SUIDatasourcePanel, BlueprintEditorPtr).AddMetaData<FTagMetaData>(FTagMetaData(TabID));
	}

protected:
	TWeakPtr<FWidgetBlueprintEditor> WeakBlueprintEditor;
};

const FName FUIDatasourceSummoner::TabID = "UIDatasourcePanel";

//////////////////////////////////////////////////////////////////
/// SDatasourceDebugger

// Represents a Datasource in the tree view
struct FUIDatasourceNode;
using FUIDatasourceNodePtr = TSharedPtr<FUIDatasourceNode>;
using SDebuggerTree = STreeView<FUIDatasourceNodePtr>;
struct FUIDatasourceNode
{
	FUIDatasourceHandle Handle = {};
	TSharedPtr<ITableRow> InstancedTable = nullptr; 
};

class SUIDatasourceDebugger
	: public SCompoundWidget
{
public:
	SLATE_USER_ARGS(SUIDatasourceDebugger)
	{ }
		SLATE_ARGUMENT(TSharedPtr<SDockTab>, ParentTab)
	SLATE_END_ARGS()

	TSharedRef<ITableRow> DebuggerTreeView_GenerateRow(FUIDatasourceNodePtr Datasource, const TSharedRef<STableViewBase>& TableViewBase);
	void DebuggerTreeView_GetChildren(FUIDatasourceNodePtr InReflectorNode, TArray<FUIDatasourceNodePtr>& OutChildren);

	virtual void Construct( const FArguments& InArgs );
	virtual ~SUIDatasourceDebugger() override;
	
	TSharedPtr<SDebuggerTree> DebuggerTree;
	FDelegateHandle MonitorEventHandle;
	TMap<EUIDatasourceId, FUIDatasourceNodePtr> DebugTreeView_Nodes;
};
SLATE_IMPLEMENT_WIDGET(SUIDatasourceDebugger);

class SUIDatasourceDebuggerTreeViewItem : public SMultiColumnTableRow<FUIDatasourceNodePtr>
{
public:
	static FName NAME_Name;
	static FName NAME_Value;

	SLATE_BEGIN_ARGS(SUIDatasourceDebuggerTreeViewItem)
	{}
		SLATE_ARGUMENT(FUIDatasourceNodePtr, DatasourceNode)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override;

protected:
	FUIDatasourceNodePtr Node;
};
SLATE_IMPLEMENT_WIDGET(SUIDatasourceDebuggerTreeViewItem);

FName SUIDatasourceDebuggerTreeViewItem::NAME_Name("Name");
FName SUIDatasourceDebuggerTreeViewItem::NAME_Value("Value");

void SUIDatasourceDebuggerTreeViewItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	Node = InArgs._DatasourceNode;

	SMultiColumnTableRow::Construct(SMultiColumnTableRow::FArguments().Padding(0.0f), InOwnerTableView);
}

TSharedRef<SWidget> SUIDatasourceDebuggerTreeViewItem::GenerateWidgetForColumn(const FName& InColumnName)
{
	if(InColumnName == NAME_Name)
	{
		TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);
		HorizontalBox->AddSlot()
		.AutoWidth()
		[
			SNew(SExpanderArrow, SharedThis(this))
			.IndentAmount(16)
			.ShouldDrawWires(true)
		];

		HorizontalBox->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock).Text(FText::FromName(Node->Handle.Get()->Name))
		];
		return HorizontalBox;
	}

	if(InColumnName == NAME_Value)
	{
		return SNew(STextBlock).Text(UEnum::GetDisplayValueAsText(Node->Handle.Get()->Value.GetType()));
	}

	return SNullWidget::NullWidget;
}

TSharedRef<ITableRow> SUIDatasourceDebugger::DebuggerTreeView_GenerateRow(FUIDatasourceNodePtr InDatasource, const TSharedRef<STableViewBase>& InOwningTable)
{
	return SAssignNew(InDatasource->InstancedTable, SUIDatasourceDebuggerTreeViewItem, InOwningTable).DatasourceNode(InDatasource);
}

void SUIDatasourceDebugger::DebuggerTreeView_GetChildren(FUIDatasourceNodePtr InNode, TArray<FUIDatasourceNodePtr>& OutChildren)
{
	FUIDatasource* NodeDatasource = InNode->Handle.Get();
	FUIDatasourcePool* Pool = NodeDatasource->GetPool();
	for(FUIDatasource* Datasource = Pool->GetDatasourceById(NodeDatasource->FirstChild); Datasource; Datasource = Pool->GetDatasourceById(Datasource->NextSibling))
	{
		FUIDatasourceNodePtr NodePtr;
		const FUIDatasourceNodePtr* Ptr = DebugTreeView_Nodes.Find(Datasource->Id);
		if(!Ptr)
		{
			Ptr = &DebugTreeView_Nodes.Add(Datasource->Id, MakeShared<FUIDatasourceNode>(FUIDatasourceNode{}));
		}
		(*Ptr)->Handle = Datasource;
		OutChildren.Add(*Ptr);
	}
}

void SUIDatasourceDebugger::Construct(const FArguments& InArgs)
{
	static TArray<FUIDatasourceNodePtr> Items;
	Items.Reset();
	Items.Add(MakeShared<FUIDatasourceNode>(FUIDatasourceNode{ UUIDatasourceSubsystem::Get()->Pool.GetRootDatasource() }));

	DebugTreeView_Nodes.Reset();
	DebugTreeView_Nodes.Add(EUIDatasourceId::Root, Items[0]);
	
	ChildSlot[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight() [
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().VAlign(VAlign_Center) [ SNew(STextBlock).Text(INVTEXT("Datasource Debugger")) ]
			+ SHorizontalBox::Slot().AutoWidth() [ SNew(SButton).Text(INVTEXT("Generate Debug Data")).OnClicked_Lambda([this]()
			{
				FUIDatasourcePool& Pool = UUIDatasourceSubsystem::Get()->Pool;
				FUIDatasource& Root = *Pool.GetRootDatasource();
				Root["Player.Stats.Health"].Set<float>(100.0f);
				Root["Player.Stats.MaxHealth"].Set<float>(100.0f);
				Root["Player.Stats.Mana"].Set<float>(100.0f);
				Root["Player.Stats.MaxMana"].Set<float>(100.0f);
				Root["Player.IsAlive"].Set<bool>(true);
				FUIDatasource& Inventory = Root["Inventory"];
				Inventory["Items.0.Name"].Set<FText>(INVTEXT("Boots"));
				Inventory["Items.0.Cost"].Set<int32>(16);
				Inventory["Items.1.Name"].Set<FText>(INVTEXT("Goggles"));
				Inventory["Items.1.Cost"].Set<int32>(500);
				Inventory["Items.2.Name"].Set<FText>(INVTEXT("Sword"));
				Inventory["Items.2.Cost"].Set<int32>(30);
				Inventory["Items.Count"].Set<int32>(3);
				return FReply::Handled();
			}) ]
		]
		+ SVerticalBox::Slot() [ 
			SAssignNew(DebuggerTree, SDebuggerTree)
			.ItemHeight(24.0f)
			.TreeItemsSource(&Items)
			.OnGenerateRow(this, &SUIDatasourceDebugger::DebuggerTreeView_GenerateRow)
			.OnGetChildren(this, &SUIDatasourceDebugger::DebuggerTreeView_GetChildren)
			.HeaderRow(
				SNew(SHeaderRow)
				.CanSelectGeneratedColumn(true)

				+SHeaderRow::Column(SUIDatasourceDebuggerTreeViewItem::NAME_Name)
				.DefaultLabel(INVTEXT("Datasource Name"))
				.FillWidth(1.f)
				.ShouldGenerateWidget(true)

				+SHeaderRow::Column(SUIDatasourceDebuggerTreeViewItem::NAME_Value)
				.DefaultLabel(INVTEXT("Value"))
				.ShouldGenerateWidget(true)
			)
		]
	];

	MonitorEventHandle = UUIDatasourceSubsystem::Get()->Monitor.OnMonitorEvent.AddLambda([this]() { DebuggerTree->RequestTreeRefresh(); });
}

SUIDatasourceDebugger::~SUIDatasourceDebugger()
{
	UUIDatasourceSubsystem::Get()->Monitor.OnMonitorEvent.Remove(MonitorEventHandle);
	MonitorEventHandle.Reset();
}


void FUIDatasourceEditorModule::StartupModule()
{
	IUMGEditorModule& UMGEditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>("UMGEditor");
	UMGEditorModule.OnRegisterTabsForEditor().AddStatic(&FUIDatasourceEditorModule::HandleRegisterBlueprintEditorTab);

	const IWorkspaceMenuStructure& MenuStructure = WorkspaceMenu::GetMenuStructure();
	FTabSpawnerEntry& SpawnerEntry = FGlobalTabmanager::Get()->RegisterNomadTabSpawner("DatasourceDebugger", FOnSpawnTab::CreateRaw(this, &FUIDatasourceEditorModule::SpawnDatasourceDebugger) )
				.SetDisplayName(LOCTEXT("DatasourceDebuggerTitle", "Datasource Debugger"))
				.SetTooltipText(LOCTEXT("DatasourceDebuggerTooltipText", "Open the Datasource Debugger tab."))
				.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));
	SpawnerEntry.SetGroup(MenuStructure.GetDeveloperToolsDebugCategory());
}

void FUIDatasourceEditorModule::ShutdownModule()
{
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("DatasourceDebugger");
}

void FUIDatasourceEditorModule::HandleRegisterBlueprintEditorTab(const FWidgetBlueprintApplicationMode& ApplicationMode, FWorkflowAllowedTabSet& TabFactories)
{
	if(ApplicationMode.GetModeName() == FWidgetBlueprintApplicationModes::GraphMode
		|| ApplicationMode.GetModeName() == FWidgetBlueprintApplicationModes::DesignerMode)
	{
		TabFactories.RegisterFactory(MakeShared<FUIDatasourceSummoner>(ApplicationMode.GetBlueprintEditor()));
	}
}

TSharedRef<SUIDatasourceDebugger> FUIDatasourceEditorModule::GetDatasourceDebugger(TSharedRef<SDockTab> InParentTab)
{
	TSharedPtr<SUIDatasourceDebugger> DatasourceDebugger = DatasourceDebuggerPtr.Pin();

	if(!DatasourceDebugger.IsValid())
	{
		DatasourceDebugger = SNew(SUIDatasourceDebugger).ParentTab(InParentTab);
		DatasourceDebuggerPtr = DatasourceDebugger;
	}

	return DatasourceDebugger.ToSharedRef();
}

TSharedRef<SDockTab> FUIDatasourceEditorModule::SpawnDatasourceDebugger(const FSpawnTabArgs& SpawnTabArgs)
{
	TSharedRef<SDockTab> DatasourceDebuggerTab = SNew(SDockTab)
			.TabRole(ETabRole::NomadTab);
	DatasourceDebuggerTab->SetContent(GetDatasourceDebugger(DatasourceDebuggerTab));
	return DatasourceDebuggerTab;
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FUIDatasourceEditorModule, UIDatasourceEditor)