#include "UIDatasourceEditorHelpers.h"

#include "EdGraphSchema_K2.h"
#include "UIDatasourceArchetype.h"
#include "UIDatasourceBlueprintLibrary.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstance.h"

FEdGraphPinType UIDatasourceEditorHelpers::GetPinTypeForDescriptor(const FUIDatasourceDescriptor& Descriptor)
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
			PinType.PinSubCategoryObject = Descriptor.ImageType == EUIDatasourceImageType::Texture ? UTexture2D::StaticClass() : UMaterialInterface::StaticClass();
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

FMemberReference UIDatasourceEditorHelpers::GetGetterFunctionForDescriptor(const FUIDatasourceDescriptor& Descriptor)
{
	FName GetDatasourceValueFunctionName;
	switch (Descriptor.Type)
	{
	case EUIDatasourceValueType::Int:
		GetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, GetInt);
		break;
	case EUIDatasourceValueType::Float:
		GetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, GetFloat);
		break;
	case EUIDatasourceValueType::Bool:
		GetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, GetBool);
		break;
	case EUIDatasourceValueType::Text:
		GetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, GetText);
		break;
	case EUIDatasourceValueType::Name:
		GetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, GetFName);
		break;
	case EUIDatasourceValueType::GameplayTag:
		GetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, GetGameplayTag);
		break;
	case EUIDatasourceValueType::Enum:
		GetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, GetIntAsByte);
		break;
	case EUIDatasourceValueType::Image:
		GetDatasourceValueFunctionName = Descriptor.ImageType == EUIDatasourceImageType::Texture ? GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, GetTexture) : GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, GetMaterial);
		break;
	case EUIDatasourceValueType::String:
		GetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, GetString);
		break;
		
	case EUIDatasourceValueType::Archetype:
	case EUIDatasourceValueType::Void:
		checkf(false, TEXT("No possible getter function for descriptor type %d"), Descriptor.Type);
		break; // @NOTE: Shouldn't get here, no conversion needed
		
	default: checkf(false, TEXT("Missing enum value implementation %d"), Descriptor.Type);
	}
	FMemberReference MemberReference;
	MemberReference.SetExternalMember(GetDatasourceValueFunctionName, UUIDatasourceBlueprintLibrary::StaticClass());
	return MemberReference;
}

FMemberReference UIDatasourceEditorHelpers::GetSetterFunctionForDescriptor(const FUIDatasourceDescriptor& Descriptor)
{
	FName SetDatasourceValueFunctionName;
	switch (Descriptor.Type)
	{
	case EUIDatasourceValueType::Int:
		SetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, SetInt);
		break;
	case EUIDatasourceValueType::Float:
		SetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, SetFloat);
		break;
	case EUIDatasourceValueType::Bool:
		SetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, SetBool);
		break;
	case EUIDatasourceValueType::Text:
		SetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, SetText);
		break;
	case EUIDatasourceValueType::Name:
		SetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, SetFName);
		break;
	case EUIDatasourceValueType::GameplayTag:
		SetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, SetGameplayTag);
		break;
	case EUIDatasourceValueType::Enum:
		SetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, SetIntAsByte);
		break;
	case EUIDatasourceValueType::Image:
		SetDatasourceValueFunctionName = Descriptor.ImageType == EUIDatasourceImageType::Texture ? GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, SetTexture) : GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, SetMaterial);
		break;
	case EUIDatasourceValueType::String:
		SetDatasourceValueFunctionName = GET_FUNCTION_NAME_CHECKED(UUIDatasourceBlueprintLibrary, SetString);
		break;
		
	case EUIDatasourceValueType::Archetype:
	case EUIDatasourceValueType::Void:
		checkf(false, TEXT("No possible setter function for descriptor type %d"), Descriptor.Type);
		break; // @NOTE: Shouldn't get here, no conversion needed
		
	default: checkf(false, TEXT("Missing enum value implementation %d"), Descriptor.Type);
	}
	FMemberReference MemberReference;
	MemberReference.SetExternalMember(SetDatasourceValueFunctionName, UUIDatasourceBlueprintLibrary::StaticClass());
	return MemberReference;
}