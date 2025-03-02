#include "MMiscFunctionLibrary.h"
#include "Online/CoreOnline.h"


bool UMMiscFunctionLibrary::CompareUniqueNetID(APlayerState* Lhs, APlayerState* Rhs)
{
	if (IsValid(Lhs) && IsValid(Rhs))
	{
		return Lhs->GetUniqueId().GetUniqueNetId() == Rhs->GetUniqueId().GetUniqueNetId();
	}

	return false;
}
