// Copyright Sharundaar. All Rights Reserved.

#include "UIDatasourceArchetype.h"

#include "UIDatasourceSubsystem.h"

void UUIDatasourceArchetype::GenerateDatasource(FUIDatasourceHandle Handle) const
{
	if(FUIDatasource* Datasource = Handle.Get())
	{
		FUIDatasourcePool* Pool = Datasource->GetPool();
		for(const FUIDatasourceDescriptor& Descriptor : Children)
		{
			FUIDatasource* ChildDatasource = Pool->FindOrCreateDatasource(Datasource, Descriptor.Path);
			switch(Descriptor.Type)
			{
			case EUIDatasourceValueType::Void: break;
			case EUIDatasourceValueType::Int:
				ChildDatasource->Set<int>({});
				break;
			case EUIDatasourceValueType::Enum:
				ChildDatasource->Set<int>({});
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
				ChildDatasource->Set<TSoftObjectPtr<UTexture2D>>({});
				break;
			case EUIDatasourceValueType::GameplayTag:
				ChildDatasource->Set<FGameplayTag>({});
				break;
			case EUIDatasourceValueType::Archetype:
				if(Descriptor.Archetype)
				{
					
				}
				break;
			default: ;
			}
		}
	}
}

const TArray<FUIDatasourceDescriptor>& UUIDatasourceArchetype::GetDescriptors() const
{
	return Children;
}
