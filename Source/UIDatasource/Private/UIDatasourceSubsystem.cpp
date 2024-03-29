﻿// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasourceSubsystem.h"

#if WITH_DATASOURCE_DEBUG_IMGUI
#include "imgui.h"
#endif

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

template<typename CHARTYPE>
static FUIDatasource* FindOrCreateDatasource_Internal(FUIDatasourcePool& Pool, FUIDatasource* Parent, TStringView<CHARTYPE> Path)
{
	int32 DotPos;

	FUIDatasource* Current = Parent; 
	if(!Current)
	{
		Current = Pool.GetRootDatasource();
	}

	while(!Path.IsEmpty())
	{
		FName SearchName;
		if(Path.FindChar('.', DotPos))
		{
			SearchName = FName(TStringView<CHARTYPE>(Path.GetData(), DotPos), FNAME_Add);
			Path.RightChopInline(DotPos + 1);
		}
		else
		{
			SearchName = FName(Path, FNAME_Add);
			Path.Reset(); // Finished iterating
		}

		FUIDatasource* ChildIt = Pool.GetDatasourceById(Current->FirstChild);
		while(ChildIt && ChildIt->Name != SearchName)
		{
			ChildIt = Pool.GetDatasourceById(ChildIt->NextSibling);
		}
		
		if(!ChildIt)
		{
			// @NOTE: Creates a new datasource and attach it to the chain
			FUIDatasource* NewDatasource = Pool.Allocate();
			NewDatasource->Name = SearchName;
			NewDatasource->Parent = Current->Id;
			if(FUIDatasource* FirstChild = Pool.GetDatasourceById(Current->FirstChild))
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

template<typename CHARTYPE>
static FUIDatasource* FindDatasource_Internal(const FUIDatasourcePool& Pool, const FUIDatasource* Parent, TStringView<CHARTYPE> Path)
{
	int32 DotPos;

	const FUIDatasource* Current = Parent;
	if(!Current)
	{
		Current = Pool.GetRootDatasource();
	}

	while(Current && !Path.IsEmpty())
	{
		FName SearchName = FName();
		if(Path.FindChar(TEXT('.'), DotPos))
		{
			SearchName = FName(TStringView<CHARTYPE>(Path.GetData(), DotPos), FNAME_Find);
			Path.RightChopInline(DotPos + 1);
		}
		else
		{
			SearchName = FName(Path, FNAME_Find);
			Path.Reset(); // Finished iterating
		}

		if(SearchName.IsNone())
		{
			return nullptr; // FName has never been registered, as such it can't ID a datasource, so early bail out
		}

		const FUIDatasource* ChildIt = Pool.GetDatasourceById(Current->FirstChild);
		while(ChildIt && ChildIt->Name != SearchName)
		{
			ChildIt = Pool.GetDatasourceById(ChildIt->NextSibling);
		}

		Current = ChildIt;
	}

	// @NOTE: const_cast here as the pool owns the datasource memory, but from a user
	// perspective we'll want to modify the returned pointer anyways
	return const_cast<FUIDatasource*>(Current);
}


FUIDatasource* FUIDatasourcePool::FindOrCreateDatasource(FUIDatasource* Parent, FWideStringView Path) { return FindOrCreateDatasource_Internal(*this, Parent, Path); }
FUIDatasource* FUIDatasourcePool::FindOrCreateDatasource(FUIDatasource* Parent, FAnsiStringView Path) { return FindOrCreateDatasource_Internal(*this, Parent, Path); }
FUIDatasource* FUIDatasourcePool::FindDatasource(const FUIDatasource* Parent, FWideStringView Path) const { return FindDatasource_Internal(*this, Parent, Path); }
FUIDatasource* FUIDatasourcePool::FindDatasource(const FUIDatasource* Parent, FAnsiStringView Path) const { return FindDatasource_Internal(*this, Parent, Path); }

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
		Instance = this;
		Pool.Initialize();
	}
}

bool UUIDatasourceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

#if WITH_DATASOURCE_DEBUG_IMGUI
void UUIDatasourceSubsystem::DrawDebugUI()
{
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
}
#endif

bool UUIDatasourceSubsystem::IsDesignerMockingEnabled() const
{
	return bIsDesignerMockingEnabled;
}

void UUIDatasourceSubsystem::EnableDesignerMocking(bool bEnabled)
{
	bIsDesignerMockingEnabled = bEnabled;
}
