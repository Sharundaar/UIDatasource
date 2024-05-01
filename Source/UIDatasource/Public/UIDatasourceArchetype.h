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

// Types of supported image by the K2Node, allows to statically determine what kind of image we're dealing with for the TSoftObjectPtr pin
UENUM()
enum class EUIDatasourceImageType : uint8
{
	Texture,
	Material,
};

USTRUCT()
struct FUIDatasourceDescriptor
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FString Path;

	UPROPERTY(EditAnywhere)
	EUIDatasourceValueType Type = EUIDatasourceValueType::Void;

	UPROPERTY(EditAnywhere, meta=(EditConditionHides, EditCondition="Type==EUIDatasourceValueType::Enum", GetOptions="UIDatasource.UIDatasourceArchetype.GetEnumChoices"))
	FString EnumPath = "";

	// Image type of this descriptor, only useful if the type is Image
	UPROPERTY(EditAnywhere, meta=(EditConditionHides, EditCondition="Type==EUIDatasourceValueType::Image"))
	EUIDatasourceImageType ImageType = EUIDatasourceImageType::Texture;
	
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

	// Return a string list of all loaded enums in the engine, used to gather UEnum* in editor for K2Node purposes
	UFUNCTION()
	static TArray<FString> GetEnumChoices();
	
protected:
	UPROPERTY(EditAnywhere)
	TArray<FUIDatasourceDescriptor> Children;

	static TArray<FName> MockNameSource;
	static TArray<FString> MockStringSource;
	static TArray<FText> MockTextSource;
};
