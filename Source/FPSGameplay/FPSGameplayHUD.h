// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FPSGameplayHUD.generated.h"

UCLASS()
class AFPSGameplayHUD : public AHUD
{
	GENERATED_BODY()

public:
	AFPSGameplayHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	/*Changes the text for the gun gravity mode in the UI*/
	UFUNCTION(BlueprintImplementableEvent, Category = GravityBall)
		void ChangeGravityModeUI(int mode);
private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

