// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "GameFramework/HUD.h"
#include "WebcameraPluginHUD.generated.h"

UCLASS()
class AWebcameraPluginHUD : public AHUD
{
	GENERATED_BODY()

public:
	AWebcameraPluginHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

