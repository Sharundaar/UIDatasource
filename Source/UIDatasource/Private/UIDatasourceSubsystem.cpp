// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasourceSubsystem.h"

#include <imgui.h>

// checks that datasource is correctly allocated
static bool IsValid(const FUIDatasource* Datasource)
{
	return Datasource && Datasource->Id != EUIDatasourceId::Invalid;
}

static const FUIDatasource MakeSinkDatasource()
{
	FUIDatasource Sink = {};
	EnumAddFlags(Sink.Flags, EUIDatasourceFlag::IsInvalid);
	return Sink;
}  
FUIDatasource FUIDatasourcePool::SinkDatasource = MakeSinkDatasource();

FUIDatasource* FUIDatasourcePool::Allocate()
{
	UIDATASOURCE_FUNC_TRACE();

	checkf(FirstFree < Datasources.Num(), TEXT("No more room to allocate new datasource, consider cleaning unused Datasource or increase pool size."));
	if (FirstFree >= Datasources.Num())
	{
		UE_LOG(LogDatasource, Fatal, TEXT("No more room to allocate new datasource, consider cleaning unused Datasource or increase pool size."));
		return nullptr;
	}

	const EUIDatasourceId NewId = static_cast<EUIDatasourceId>(FirstFree);
	FUIDatasource* Alloc = &Datasources[FirstFree++];
	const FUIDatasourceGeneration Generation = Alloc->Generation;
	while (FirstFree < Datasources.Num() && IsValid(&Datasources[FirstFree]))
	{
		FirstFree++;
	}
	AllocatedCount++;

	*Alloc = {};
	Alloc->Id = NewId;
	Alloc->Generation = Generation;
	return Alloc;
}

void FUIDatasourcePool::Clear()
{
	Datasources.Empty();
	Initialize();
}

void FUIDatasourcePool::Initialize()
{
	Datasources.SetNumZeroed(ChunkSize);
	
	FUIDatasourceHeader* Header = reinterpret_cast<FUIDatasourceHeader*>(&Datasources[static_cast<int>(EUIDatasourceId::Header)]);
	*Header = { this };
	AllocatedCount++;

	FUIDatasource* Root = &Datasources[static_cast<int>(EUIDatasourceId::Root)];
	Root->Id = EUIDatasourceId::Root;
	Root->Name = "Root";
	AllocatedCount++;

	FirstFree = ToIndex(EUIDatasourceId::Root) + 1;
}

FUIDatasourceHeader* FUIDatasourcePool::GetHeaderDatasource()
{
	return reinterpret_cast<FUIDatasourceHeader*>(&Datasources[ToIndex(EUIDatasourceId::Header)]);
}

const FUIDatasourceHeader* FUIDatasourcePool::GetHeaderDatasource() const
{
	return reinterpret_cast<const FUIDatasourceHeader*>(&Datasources[ToIndex(EUIDatasourceId::Header)]);
}

FUIDatasource* FUIDatasourcePool::GetRootDatasource()
{
	return &Datasources[ToIndex(EUIDatasourceId::Root)];
}

const FUIDatasource* FUIDatasourcePool::GetRootDatasource() const
{
	return &Datasources[ToIndex(EUIDatasourceId::Header)];
}

const FUIDatasource* FUIDatasourcePool::GetDatasourceById(EUIDatasourceId Id) const
{
	return Id != EUIDatasourceId::Invalid ? &Datasources[ToIndex(Id)] : nullptr;
}

FUIDatasource* FUIDatasourcePool::GetDatasourceById(EUIDatasourceId Id)
{
	return Id != EUIDatasourceId::Invalid ? &Datasources[ToIndex(Id)] : nullptr;
}

FUIDatasource* FUIDatasourcePool::FindOrCreateDatasource(FUIDatasource* Parent, const FString& Path)
{
	FStringView StrView = Path;
	int32 DotPos;

	FUIDatasource* Current = Parent; 
	if(!Current)
	{
		Current = GetRootDatasource();
	}

	while(!StrView.IsEmpty())
	{
		FName SearchName;
		if(StrView.FindChar(TEXT('.'), DotPos))
		{
			SearchName = FName(FStringView(StrView.GetData(), DotPos), FNAME_Add);
			StrView.RightChopInline(DotPos + 1);
		}
		else
		{
			SearchName = FName(StrView, FNAME_Add);
			StrView.Reset(); // Finished iterating
		}

		FUIDatasource* ChildIt = GetDatasourceById(Current->FirstChild);
		while(ChildIt && ChildIt->Name != SearchName)
		{
			ChildIt = GetDatasourceById(ChildIt->NextSibling);
		}
		
		if(!ChildIt)
		{
			// @NOTE: Creates a new datasource and attach it to the chain
			FUIDatasource* NewDatasource = Allocate();
			NewDatasource->Name = SearchName;
			NewDatasource->Parent = Current->Id;
			if(FUIDatasource* FirstChild = GetDatasourceById(Current->FirstChild))
			{
				FirstChild->PrevSibling = NewDatasource->Id;
			}
			NewDatasource->NextSibling = Current->FirstChild;
			Current->FirstChild = NewDatasource->Id;
			ChildIt = NewDatasource;
		}

		Current = ChildIt;
	}

	return Current;
}

FUIDatasource* FUIDatasourcePool::FindDatasource(FUIDatasource* Parent, const FString& Path)
{
	FStringView StrView = Path;
	int32 DotPos;

	FUIDatasource* Current = Parent; 
	if(!Current)
	{
		Current = GetRootDatasource();
	}

	while(Current && !StrView.IsEmpty())
	{
		FName SearchName = FName();
		if(StrView.FindChar(TEXT('.'), DotPos))
		{
			SearchName = FName(FStringView(StrView.GetData(), DotPos), FNAME_Find);
			StrView.RightChopInline(DotPos + 1);
		}
		else
		{
			SearchName = FName(StrView, FNAME_Find);
			StrView.Reset(); // Finished iterating
		}

		if(SearchName.IsNone())
		{
			return nullptr; // FName has never been registered, as such it can't ID a datasource, so early bail out
		}

		FUIDatasource* ChildIt = GetDatasourceById(Current->FirstChild);
		while(ChildIt && ChildIt->Name != SearchName)
		{
			ChildIt = GetDatasourceById(ChildIt->NextSibling);
		}

		Current = ChildIt;
	}

	return Current;
}

FUIDatasource* FUIDatasourcePool::FindOrCreateChildDatasource(FUIDatasource* Parent, FName Name)
{
	if(Name.IsNone())
	{
		return nullptr;
	}

	if(!Parent)
	{
		Parent = GetRootDatasource();
	}

	FUIDatasource* ChildIt = GetDatasourceById(Parent->FirstChild);
	while(ChildIt && ChildIt->Name != Name)
	{
		ChildIt = GetDatasourceById(ChildIt->NextSibling);
	}
		
	if(!ChildIt)
	{
		// @NOTE: Creates a new datasource and attach it to the chain
		FUIDatasource* NewDatasource = Allocate();
		NewDatasource->Name = Name;
		NewDatasource->Parent = Parent->Id;
		if(FUIDatasource* FirstChild = GetDatasourceById(Parent->FirstChild))
		{
			FirstChild->PrevSibling = NewDatasource->Id;
		}
		NewDatasource->NextSibling = Parent->FirstChild;
		Parent->FirstChild = NewDatasource->Id;
		ChildIt = NewDatasource;
	}

	return ChildIt;
}

FUIDatasource* FUIDatasourcePool::FindChildDatasource(FUIDatasource* Parent, FName Name)
{
	if(Name.IsNone())
    {
    	return nullptr;
    }

    if(!Parent)
    {
    	Parent = GetRootDatasource();
    }

    FUIDatasource* ChildIt = GetDatasourceById(Parent->FirstChild);
    while(ChildIt && ChildIt->Name != Name)
    {
    	ChildIt = GetDatasourceById(ChildIt->NextSibling);
    }
	
    return ChildIt;
}

void FUIDatasourcePool::DestroyDatasource(FUIDatasource* Datasource)
{
	FirstFree = FMath::Min(FirstFree, static_cast<int>(Datasource->Id));

	for(FUIDatasource* Child = GetDatasourceById(Datasource->FirstChild); Child; Child = GetDatasourceById(Datasource->NextSibling))
	{
		DestroyDatasource(Child);
	}
	
	Datasource->Id = EUIDatasourceId::Invalid;
	Datasource->Generation++;
}

UUIDatasourceSubsystem* UUIDatasourceSubsystem::Instance = nullptr;
UUIDatasourceSubsystem* UUIDatasourceSubsystem::Get()
{
	return Instance;
}

void UUIDatasourceSubsystem::Deinitialize()
{
	Pool.Clear();
}

void UUIDatasourceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	if(!IsTemplate())
	{
		FSlateApplication::Get().OnPostTick().AddUObject(this, &UUIDatasourceSubsystem::Tick);
		Instance = this;

		Pool.Initialize();
	}
}

bool UUIDatasourceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UUIDatasourceSubsystem::Tick(float DeltaTime)
{
#if 0
	FString Name;
	const auto RecItTree = [this, &Name](FUIDatasource* Current, auto Rec) -> void
	{
		Current->Name.ToString(Name);
		if(Current->FirstChild == EUIDatasourceId::Invalid)
		{
			ImGui::Text(TCHAR_TO_ANSI(*Name));
		}
		else
		{
			if(ImGui::TreeNode(TCHAR_TO_ANSI(*Name)))
			{
				Current = Pool.GetDatasourceById(Current->FirstChild);
				while(Current)
				{
					Rec(Current, Rec);
					Current = Pool.GetDatasourceById(Current->NextSibling);
				}
				ImGui::TreePop();
			}
		}
	};
	if(ImGui::Begin("Datasource Debugger"))
	{
		// ImGui::SetNextWindowClass()
		ImGui::Text("Datasource tree:");
		RecItTree(Pool.GetRootDatasource(), RecItTree);

		static char Buffer[1024];
		ImGui::InputText("Path", Buffer, UE_ARRAY_COUNT(Buffer));
		ImGui::SameLine();
		if(ImGui::Button("Create"))
		{
			Pool.FindOrCreateDatasource(nullptr, Buffer);
		}
	}
	ImGui::End();
#endif
}
