// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "FPSGameplayGameMode.h"
#include "FPSGameplayHUD.h"
#include "FPSGameplayCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFPSGameplayGameMode::AFPSGameplayGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFPSGameplayHUD::StaticClass();
}
