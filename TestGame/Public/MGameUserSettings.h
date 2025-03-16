// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "MGameUserSettings.generated.h"

UCLASS()
class TESTGAME_API UMGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	static UMGameUserSettings* GetMGameUserSettings();

	UFUNCTION(BlueprintCallable)
	void SetMasterVolume(float InValue) { MasterVolume = InValue; }
	UFUNCTION(BlueprintCallable)
	void SettFXVolume(float InValue) { FXVolume = InValue; }
	UFUNCTION(BlueprintCallable)
	void SetBGMVolume(float InValue) { BGMVolume = InValue; }
	UFUNCTION(BlueprintPure)
	float GetMasterVolume() const { return MasterVolume; }
	UFUNCTION(BlueprintPure)
	float GetFXVolume() const { return FXVolume; }
	UFUNCTION(BlueprintPure)
	float GetBGMVolume() const { return BGMVolume; }

	UPROPERTY(config)
	float MasterVolume = 1.f;
	UPROPERTY(config)
	float FXVolume = 1.f;
	UPROPERTY(config)
	float BGMVolume = 1.f;

	UFUNCTION(BlueprintCallable)
	void SetGraphic(int32 InGraphic);
	UFUNCTION(BlueprintPure)
	int32 GetGraphic() const { return Graphic; }
	UPROPERTY(config)
	int32 Graphic = 3;
};
