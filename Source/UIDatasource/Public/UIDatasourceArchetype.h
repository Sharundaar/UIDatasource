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
	UFUNCTION(BlueprintCallable)
	void GenerateDatasource(FUIDatasourceHandle Handle) const;

	const TArray<FUIDatasourceDescriptor>& GetDescriptors() const; 
	
protected:
	UPROPERTY(EditAnywhere)
	TArray<FUIDatasourceDescriptor> Children;
};
