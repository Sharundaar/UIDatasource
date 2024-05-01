// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasourceArchetype.h"

#include "UIDatasourceSubsystem.h"
#include "UObject/UObjectIterator.h"

TArray<FName> UUIDatasourceArchetype::MockNameSource = { "Name1", "Name2", "Name3" };
TArray<FString> UUIDatasourceArchetype::MockStringSource = { TEXT("String1"), TEXT("String2"), TEXT("String3") };
TArray<FText> UUIDatasourceArchetype::MockTextSource = { INVTEXT("Text1"), INVTEXT("Text2"), INVTEXT("Text3") };

void UUIDatasourceArchetype::MockDatasource(FUIDatasource* Datasource) const
{
	if(!Datasource)
	{
		return;
	}

	FUIDatasourcePool* Pool = Datasource->GetPool();
	for(const FUIDatasourceDescriptor& Descriptor : Children)
	{
		if(Descriptor.IsInlineArchetype())
		{
			Descriptor.Archetype->MockDatasource(Datasource);
		}
		else
		{
			
			FUIDatasource* ChildDatasource = Pool->FindOrCreateDatasource(Datasource, *Descriptor.Path);
			switch(Descriptor.Type)
			{
			case EUIDatasourceValueType::Void: break;
			case EUIDatasourceValueType::Int:
				ChildDatasource->Set<int32>(FMath::RandHelper(100));
				break;
			case EUIDatasourceValueType::Enum:
				{
					UEnum* Enum = FindObject<UEnum>(nullptr, *Descriptor.EnumPath);
					const int MaxIndex = Enum->NumEnums();
					ChildDatasource->Set<int32>(Enum->GetValueByIndex(FMath::RandHelper(MaxIndex)));
				}
				break;
			case EUIDatasourceValueType::Float:
				ChildDatasource->Set<float>(FMath::FRandRange(0.0f, 1.0f));
				break;
			case EUIDatasourceValueType::Bool:
				ChildDatasource->Set<bool>(FMath::RandBool());
				break;
			case EUIDatasourceValueType::Name:
				ChildDatasource->Set<FName>(MockNameSource[FMath::RandHelper(MockNameSource.Num())]);
				break;
			case EUIDatasourceValueType::Text:
				ChildDatasource->Set<FText>(MockTextSource[FMath::RandHelper(MockNameSource.Num())]);
				break;
			case EUIDatasourceValueType::String:
				ChildDatasource->Set<FString>(MockStringSource[FMath::RandHelper(MockNameSource.Num())]);
				break;
			case EUIDatasourceValueType::Image:
				ChildDatasource->Set<FUIDatasourceImage>({});
				break;
			case EUIDatasourceValueType::GameplayTag:
				ChildDatasource->Set<FGameplayTag>({});
				break;
			case EUIDatasourceValueType::Archetype:
				if(Descriptor.Archetype)
				{
					switch(Descriptor.ImportMethod)
					{
					case EUIDatasourceArchetypeImportMethod::AsChild:
						Descriptor.Archetype->MockDatasource(ChildDatasource);
						break;
					case EUIDatasourceArchetypeImportMethod::AsArray:
						// @TODO: Build array datasource structure
						break;
					default: ;
					}
				}
				break;
			default: ;
			}
		}
	}
}

void UUIDatasourceArchetype::GenerateDatasource(FUIDatasource* Datasource) const
{
	if(!Datasource)
	{
		return;
	}
	
	FUIDatasourcePool* Pool = Datasource->GetPool();
	for(const FUIDatasourceDescriptor& Descriptor : Children)
	{
		if(Descriptor.IsInlineArchetype())
		{
			Descriptor.Archetype->GenerateDatasource(Datasource);
		}
		else
		{
			
			FUIDatasource* ChildDatasource = Pool->FindOrCreateDatasource(Datasource, *Descriptor.Path);
			switch(Descriptor.Type)
			{
			case EUIDatasourceValueType::Void: break;
			case EUIDatasourceValueType::Int:
				ChildDatasource->Set<int32>({});
				break;
			case EUIDatasourceValueType::Enum:
				ChildDatasource->Set<int32>({});
				break;
			case EUIDatasourceValueType::Float:
				ChildDatasource->Set<float>({});
				break;
			case EUIDatasourceValueType::Bool:
				ChildDatasource->Set<bool>({});
				break;
			case EUIDatasourceValueType::Name:
				ChildDatasource->Set<FName>({});
				break;
			case EUIDatasourceValueType::Text:
				ChildDatasource->Set<FText>({});
				break;
			case EUIDatasourceValueType::String:
				ChildDatasource->Set<FString>({});
				break;
			case EUIDatasourceValueType::Image:
				ChildDatasource->Set<FUIDatasourceImage>({});
				break;
			case EUIDatasourceValueType::GameplayTag:
				ChildDatasource->Set<FGameplayTag>({});
				break;
			case EUIDatasourceValueType::Archetype:
				if(Descriptor.Archetype)
				{
					switch(Descriptor.ImportMethod)
					{
					case EUIDatasourceArchetypeImportMethod::AsChild:
						Descriptor.Archetype->GenerateDatasource(ChildDatasource);
						break;
					case EUIDatasourceArchetypeImportMethod::AsArray:
						// @TODO: Build array datasource structure
						break;
					default: ;
					}
				}
				break;
			default: ;
			}
		}
	}
}

void UUIDatasourceArchetype::GenerateDatasource(FUIDatasourceHandle Handle) const
{
	GenerateDatasource(Handle.Get());
}

void UUIDatasourceArchetype::MockDatasource(FUIDatasourceHandle Handle) const
{
	
}

const TArray<FUIDatasourceDescriptor>& UUIDatasourceArchetype::GetDescriptors() const
{
	return Children;
}

void UUIDatasourceArchetype::SetChildren(const TArray<FUIDatasourceDescriptor>& Descriptors)
{
	Children = Descriptors;
}

TArray<FString> UUIDatasourceArchetype::GetEnumChoices()
{
	TArray<FString> EnumPaths = {};

	for(TObjectIterator<UEnum> EnumIt; EnumIt; ++EnumIt)
	{
		EnumPaths.Add(EnumIt->GetPathName());
	}
	
	return EnumPaths;
}
