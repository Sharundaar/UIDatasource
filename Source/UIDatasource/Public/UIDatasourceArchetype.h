// Copyright Sharundaar. All Rights Reserved.

#pragma once

#include "UIDatasource.h"
#include "Engine/DataAsset.h"

#include "UIDatasourceArchetype.generated.h" 

class UUIDatasourceArchetype;

UENUM()
enum class EUIDatasourceArchetypeImportMethod
{
	AsChild,
	Inline,
	AsArray,
};

USTRUCT()
struct FUIDatasourceDescriptor
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FString Path;

	UPROPERTY(EditAnywhere)
	EUIDatasourceValueType Type = EUIDatasourceValueType::Void;

	UPROPERTY(EditAnywhere, meta=(EditConditionHides, EditCondition="Type==EUIDatasourceValueType::Enum"))
	FString EnumPath = "";

	UPROPERTY(EditAnywhere, meta=(EditConditionHides, EditCondition="Type==EUIDatasourceValueType::Archetype"))
	TObjectPtr<UUIDatasourceArchetype> Archetype = nullptr;

	UPROPERTY(EditAnywhere, meta=(EditConditionHides, EditCondition="Type==EUIDatasourceValueType::Archetype"))
	EUIDatasourceArchetypeImportMethod ImportMethod = EUIDatasourceArchetypeImportMethod::AsChild;

	bool IsInlineArchetype() const
	{
		return Archetype && ImportMethod == EUIDatasourceArchetypeImportMethod::Inline;
	}
};

UCLASS()
class UIDATASOURCE_API UUIDatasourceArchetype : public UDataAsset
{
	GENERATED_BODY()

public:
	void MockDatasource(FUIDatasource* Datasource) const;
	void GenerateDatasource(FUIDatasource* Datasource) const;
	
	UFUNCTION(BlueprintCallable)
	void GenerateDatasource(FUIDatasourceHandle Handle) const;

	UFUNCTION(BlueprintCallable)
	void MockDatasource(FUIDatasourceHandle Handle) const;
	
	const TArray<FUIDatasourceDescriptor>& GetDescriptors() const; 

	void SetChildren(const TArray<FUIDatasourceDescriptor>& Descriptors);
	
protected:
	UPROPERTY(EditAnywhere)
	TArray<FUIDatasourceDescriptor> Children;

	static TArray<FName> MockNameSource;
	static TArray<FString> MockStringSource;
	static TArray<FText> MockTextSource;
};
