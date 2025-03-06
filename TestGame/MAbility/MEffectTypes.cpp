#include "MEffectTypes.h"

UScriptStruct* FMGameplayEffectContext::GetScriptStruct() const
{
	return FMGameplayEffectContext::StaticStruct();
}

bool FMGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	uint8 RepBits = 0;
	if (Ar.IsSaving())
	{
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}
		if (bReplicateEffectCauser && EffectCauser.IsValid())
		{
			RepBits |= 1 << 1;
		}
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
		if (EffectIndex != INDEX_NONE)
		{
			RepBits |= 1 << 7;
		}
	}

	Ar.SerializeBits(&RepBits, 8);

	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	if (RepBits & (1 << 5))
	{
		if (Ar.IsLoading())
		{
			if (!HitResult.IsValid())
			{
				HitResult = TSharedPtr<FHitResult>(new FHitResult());
			}
		}
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}
	if (RepBits & (1 << 7))
	{
		Ar << EffectIndex;
	}

	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get()); // Just to initialize InstigatorAbilitySystemComponent
	}

	bOutSuccess = true;
	return true;
}

FGameplayEffectContext* FMGameplayEffectContext::Duplicate() const
{
	FMGameplayEffectContext* NewContext = new FMGameplayEffectContext();

	// 얕은복사 후 HitResult 깊은복사
	*NewContext = *this;
	if (GetHitResult())
	{
		// Does a deep copy of the hit result
		NewContext->AddHitResult(*GetHitResult(), true);
	}

	return NewContext;
}

FMGameplayEffectContextHandle::FMGameplayEffectContextHandle()
{

}

FMGameplayEffectContextHandle::FMGameplayEffectContextHandle(FGameplayEffectContext* DataPtr)
	: FGameplayEffectContextHandle(DataPtr)
{
}

void FMGameplayEffectContextHandle::SetEffectIndex(int32 InEffectindex)
{
	if (FMGameplayEffectContext* Context = GetMEffectContext(*this))
	{
		Context->EffectIndex = InEffectindex;
	}
}

FMGameplayEffectContext* FMGameplayEffectContextHandle::GetMEffectContext(FGameplayEffectContextHandle& InContextHandle)
{
	if (InContextHandle.IsValid() == false)
	{
		return nullptr;
	}

	if (InContextHandle.Get()->GetScriptStruct() != FMGameplayEffectContext::StaticStruct())
	{
		return nullptr;
	}

	return static_cast<FMGameplayEffectContext*>(InContextHandle.Get());
}

int32 UMGameplayEffectContextHelper::GetEffectIndex(FGameplayEffectContextHandle InContextHandle)
{
	if (FMGameplayEffectContext* Context = FMGameplayEffectContextHandle::GetMEffectContext(InContextHandle))
	{
		return Context->EffectIndex;
	}

	return INDEX_NONE;
}
