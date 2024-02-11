#pragma once

#include "TickableEditorObject.h"
#include "UIDatasource.h"

#include "UIDatasourceSubsystem.generated.h"

struct FUIDatasourcePool
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
	FUIDatasource* FindOrCreateDatasource(FUIDatasource* Parent, const FString& Path);
	FUIDatasource* FindDatasource(FUIDatasource* Parent, const FString& Path);
	FUIDatasource* FindOrCreateChildDatasource(FUIDatasource* Parent, FName Name);
	FUIDatasource* FindChildDatasource(FUIDatasource* Parent, FName Name);

	void DestroyDatasource(FUIDatasource* Datasource);
	
protected:
	static constexpr int ChunkSize = 2048;
	TArray<FUIDatasource> Datasources = {};
	int FirstFree = 0;
	int AllocatedCount = 0;

public:
	static FUIDatasource SinkDatasource; // Special datasource that no-ops
};

UCLASS()
class UUIDatasourceSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:
	static UUIDatasourceSubsystem* Get();
	
public:
	virtual void Deinitialize() override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void Tick(float DeltaTime);

	FUIDatasourcePool Pool;

protected:
	static UUIDatasourceSubsystem* Instance;
};

