#pragma once

#include "EdGraph/EdGraphPin.h" 
#include "Engine/MemberReference.h"

struct FUIDatasourceDescriptor;

namespace UIDatasourceEditorHelpers
{
	// Return a pin type compatible with the descriptor
	FEdGraphPinType GetPinTypeForDescriptor(const FUIDatasourceDescriptor& Descriptor);

	// Return the member reference that allows a function node to extract data from a datasource in a type safe way
	FMemberReference GetGetterFunctionForDescriptor(const FUIDatasourceDescriptor& Descriptor);

	// Return the member reference that allows a function node to set data to a datasource in a type safe way
	FMemberReference GetSetterFunctionForDescriptor(const FUIDatasourceDescriptor& Descriptor);
}