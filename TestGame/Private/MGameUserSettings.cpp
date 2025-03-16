#include "MGameUserSettings.h"

UMGameUserSettings* UMGameUserSettings::GetMGameUserSettings()
{
	return Cast<UMGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void UMGameUserSettings::SetGraphic(int32 InGraphic)
{
	SetOverallScalabilityLevel(InGraphic);
	SetViewDistanceQuality(InGraphic);
	SetShadowQuality(InGraphic);
	SetGlobalIlluminationQuality(InGraphic);
	SetReflectionQuality(InGraphic);
	SetTextureQuality(InGraphic);
	SetVisualEffectQuality(InGraphic);
	SetPostProcessingQuality(InGraphic);
	SetFoliageQuality(InGraphic);
	SetShadingQuality(InGraphic);

	Graphic = InGraphic;
}
