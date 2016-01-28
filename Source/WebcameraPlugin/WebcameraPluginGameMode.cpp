// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "WebcameraPlugin.h"
#include "WebcameraPluginGameMode.h"
#include "WebcameraPluginHUD.h"
#include "WebcameraPluginCharacter.h"

AWebcameraPluginGameMode::AWebcameraPluginGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AWebcameraPluginHUD::StaticClass();
}
