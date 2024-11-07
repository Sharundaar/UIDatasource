// Copyright Sharundaar. All Rights Reserved.

#pragma once

#include "UIDatasource.h"
#include "UIDatasourceMonitor.h"
#include "Subsystems/EngineSubsystem.h"

#include "UIDatasourceSubsystem.generated.h"

struct UIDATASOURCE_API FUIDatasourcePool
{	
public:
	FUIDatasourcePool() = default;
	FUIDatasource* Allocate();

	void Clear();
	void Initialize();
	
	FUIDatasourceHeader* GetHeaderDatasource();
	const FUIDatasourceHeader* GetHeaderDatasource() const;
	FUIDatasource* GetRootDatasource();
	const FUIDatasource* GetRootDatasource() const;

	const FUIDatasource* GetDatasourceById(EUIDatasourceId Id) const;
	FUIDatasource* GetDatasourceById(EUIDatasourceId Id);

	// Find a child datasource from Parent according to Path
	// If Parent is empty, search from the root
	// If Path is empty, returns parent (this makes it easier to fetch and bind to 'self' datasource)
	FUIDatasource* FindOrCreateDatasource(FUIDatasource* Parent, FWideStringView Path);
	FUIDatasource* FindOrCreateDatasource(FUIDatasource* Parent, FAnsiStringView Path);
	FUIDatasource* FindDatasource(const FUIDatasource* Parent, FWideStringView Path) const;
	FUIDatasource* FindDatasource(const FUIDatasource* Parent, FAnsiStringView Path) const;
	
	FUIDatasource* FindOrCreateChildDatasource(FUIDatasource* Parent, FName Name);
	FUIDatasource* FindChildDatasource(const FUIDatasource* Parent, FName Name);

	void DestroyDatasource(FUIDatasource* Datasource);

	int32 Num() const { return AllocatedCount; };
	static constexpr int Capacity() { return ChunkSize; }
	
protected:
	static constexpr int ChunkSize = 4096;
	TArray<FUIDatasource> Datasources = {};
	int FirstFree = 0;
	int AllocatedCount = 0;

public:
	static FUIDatasource SinkDatasource; // Special datasource that no-ops
};

UCLASS()
class UIDATASOURCE_API UUIDatasourceSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:
	static UUIDatasourceSubsystem* Get();
	static void LogDatasourceChange(FUIDatasourceLogEntry Change);
		
public:
	virtual void Deinitialize() override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	static void Reset()
	{
		checkf(Instance, TEXT("DatasourceSubsystem not initialized, called too early."));
		Instance->Pool.Clear();
	}
	
	// Find a child datasource from Parent according to Path
	// If Parent is empty, search from the root
	// If Path is empty, returns parent (this makes it easier to fetch and bind to 'self' datasource)
	template<class STRVIEW>
	static FUIDatasource* FindOrCreateDatasource(FUIDatasource* Parent, const STRVIEW& Path)
	{
		checkf(Instance, TEXT("DatasourceSubsystem not initialized, called too early."));
		return Instance->Pool.FindOrCreateDatasource(Parent, Path);
	}
	template<class STRVIEW>
	static FUIDatasource* FindOrCreateDatasource(const STRVIEW& Path)
	{
		checkf(Instance, TEXT("DatasourceSubsystem not initialized, called too early."));
		return Instance->Pool.FindOrCreateDatasource(nullptr, Path);
	}
	template<class STRVIEW>
	static FUIDatasource* FindDatasource(const FUIDatasource* Parent, const STRVIEW& Path)
	{
		checkf(Instance, TEXT("DatasourceSubsystem not initialized, called too early."));
		return Instance->Pool.FindDatasource(Parent, Path);
	}
	template<class STRVIEW>
	static FUIDatasource* FindDatasource(const STRVIEW& Path)
	{
		checkf(Instance, TEXT("DatasourceSubsystem not initialized, called too early."));
		return Instance->Pool.FindDatasource(nullptr, Path);
	}
	
	static void DestroyDatasource(FUIDatasource* Datasource)
	{
		checkf(Instance, TEXT("DatasourceSubsystem not initialized, called too early."));
		Instance->Pool.DestroyDatasource(Datasource);
	}
	
	
#if WITH_DATASOURCE_DEBUG_IMGUI
	void DrawDebugUI();
#endif
	
	bool IsDesignerMockingEnabled() const;
	void EnableDesignerMocking(bool bEnabled);

	FUIDatasourcePool Pool;
#if WITH_UIDATASOURCE_MONITOR
	FUIDatasourceMonitor Monitor;
	FDelegateHandle SlatePreTickHandle;
#else
	FSimpleMulticastDelegate OnLog;
#endif

protected:
	bool bIsDesignerMockingEnabled = false;
	static UUIDatasourceSubsystem* Instance;
};

