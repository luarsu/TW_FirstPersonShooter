// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GravityBall.h"
#include "CableComponent.h"
#include "Materials/MaterialInstance.h"
#include "FPSGameplayHUD.h"
#include "FPSGameplayCharacter.generated.h"


class UInputComponent;

UCLASS(config = Game)
class AFPSGameplayCharacter : public ACharacter
{
	GENERATED_BODY()

		/** Pawn mesh: 1st person view (arms; seen only by self) */
		UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USceneComponent* FP_MuzzleLocation;

	/** Location on gun mesh where the gravity ball should be attached. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USceneComponent* FP_MuzzleLocationGravityBallSpawn;

	/** Location on gun mesh where the gravity ball should be attached. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USceneComponent* FP_MuzzleLocationHook;

	/** Gun mesh: VR view (attached to the VR controller directly, no arm, just the actual gun) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* VR_Gun;

	/** Location on VR gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USceneComponent* VR_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FirstPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UMotionControllerComponent* L_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UCableComponent* HookRope;

public:
	AFPSGameplayCharacter();

protected:
	virtual void BeginPlay();

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class AFPSGameplayProjectile> ProjectileClass;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Gravity)
		TSubclassOf<class AGravityBall> GravityBallClass;

	/** True if the player has the gravity ball connected */
	UPROPERTY(EditDefaultsOnly, Category = Gravity)
		bool HasGravityBall;

	/** Reference to the Gravity ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Gravity)
		class AGravityBall* GravityBall;

	/** True if the character is swinging */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
		bool IsSwinging;

	/** Extra force added to the swing force (has to be negative)*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
		float ForceSwingMagnitude = -6.f;

	/** How long can the gravity ball stay outside the player*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float GravityBallDuration = 10.f;

	/** Timer for the gravity ball*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
		float GravityBallTimer;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		class UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		uint32 bUsingMotionControllers : 1;

	/** Reference to the gravity material instance to change its color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gravity)
		UMaterialInstance* AttractMaterialInstance;

	/** Reference to the gravity material instance to change its color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gravity)
		UMaterialInstance* RepulsionMaterialInstance;

	/** Reference to the gravity material instance to change its color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gravity)
		UMaterialInstance* HookMaterialInstance;

	/** Reference to the game hud */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
		AFPSGameplayHUD* GameHud;

protected:

	/** Fires a projectile. */
	void OnFire();

	/** Fires the gravityGun. */
	void OnShootGravityBall();

	/** Calls the gravity ball back */
	void OnReturnGravityBall();

	/** Called when the player activates the hook action of the gravity gun */
	void OnHook();

	/** Called when the player is hanging from the hook */
	void HangFromGravityHook();

	/** Called when the player stops hanging from the hook*/
	void OnUnhook();

	/** Called when the player changes the gravity mode of the ball to attraction*/
	void OnSetGravityModeAttraction();

	/** Called when the player changes the gravity mode of the ball to repulsion*/
	void OnSetGravityModeRepulsion();

	/** Called when the player changes the gravity mode of the ball to hook mode*/
	void OnSetGravityModeHook();

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false; Location = FVector::ZeroVector; }
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/*
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

