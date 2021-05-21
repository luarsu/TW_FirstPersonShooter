// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "FPSGameplayCharacter.h"
#include "FPSGameplayProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "Math/Vector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AFPSGameplayCharacter

AFPSGameplayCharacter::AFPSGameplayCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	FP_MuzzleLocationGravityBallSpawn = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocationGravityBall"));
	FP_MuzzleLocationGravityBallSpawn->SetupAttachment(FP_Gun);
	FP_MuzzleLocationGravityBallSpawn->SetRelativeLocation(FVector(0.2f, 98.4f, -20.6));

	FP_MuzzleLocationHook = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocationHook"));
	FP_MuzzleLocationHook->SetupAttachment(FP_Gun);
	FP_MuzzleLocationHook->SetRelativeLocation(FVector(0.2f, 58.4f, 9.4));

	HookRope = CreateDefaultSubobject<UCableComponent>(TEXT("GravityHookConnection"));
	HookRope->SetVisibility(false);
	

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void AFPSGameplayCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	GameHud = Cast<AFPSGameplayHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}

	//Spawn the gravity ball
	if (GravityBallClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocationGravityBallSpawn != nullptr) ? FP_MuzzleLocationGravityBallSpawn->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			// spawn the gravity ball at the muzzle
			GravityBall = World->SpawnActor<AGravityBall>(GravityBallClass, SpawnLocation, SpawnRotation, ActorSpawnParams);

			//attach the gravity ball to the muzzle offset
			GravityBall->AttachToComponent(FP_MuzzleLocationGravityBallSpawn, FAttachmentTransformRules::KeepWorldTransform);
			GravityBall->AttachToGunComponent = FP_MuzzleLocationGravityBallSpawn;
			GravityBall->GunOffset = GunOffset;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("The gravity ball subclass is not selected!"));
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFPSGameplayCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSGameplayCharacter::OnFire);

	//Bind the actions for the gravity
	PlayerInputComponent->BindAction("ShootGravityBall", IE_Pressed, this, &AFPSGameplayCharacter::OnShootGravityBall);
	PlayerInputComponent->BindAction("ReturnGravityBall", IE_Pressed, this, &AFPSGameplayCharacter::OnReturnGravityBall);
	PlayerInputComponent->BindAction("Hook", IE_Pressed, this, &AFPSGameplayCharacter::OnHook);
	PlayerInputComponent->BindAction("Hook", IE_Released, this, &AFPSGameplayCharacter::OnUnhook);
	PlayerInputComponent->BindAction("GravityMode1", IE_Pressed, this, &AFPSGameplayCharacter::OnSetGravityModeAttraction);
	PlayerInputComponent->BindAction("GravityMode2", IE_Pressed, this, &AFPSGameplayCharacter::OnSetGravityModeRepulsion);
	PlayerInputComponent->BindAction("GravityMode3", IE_Pressed, this, &AFPSGameplayCharacter::OnSetGravityModeHook);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AFPSGameplayCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSGameplayCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSGameplayCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turn rate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFPSGameplayCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFPSGameplayCharacter::LookUpAtRate);
}

void AFPSGameplayCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//handle the swing
	if (GravityBall->GravityMode == E_GravityMode::MODE_HOOK && IsSwinging && GetCharacterMovement()->IsFalling() && GravityBall->IsGravityActive)
	{
		HangFromGravityHook();
	}

	//handle the timer
	if (GravityBallTimer > 0)
	{
		GravityBallTimer -= DeltaTime;
	}
	else if (GravityBall->IsGravityActive)
	{
		OnReturnGravityBall();
	}
}


void AFPSGameplayCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<AFPSGameplayProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<AFPSGameplayProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AFPSGameplayCharacter::OnShootGravityBall()
{
	if (GravityBall && !GravityBall->IsDettached)
	{
		GravityBall->ShootBall();
		GravityBallTimer = GravityBallDuration;
	}
	else if (GravityBall && GravityBall->IsMovingForward && GravityBall->IsDettached)
	{
		GravityBall->StopMoving();
	}
}

void AFPSGameplayCharacter::OnReturnGravityBall()
{
	if (GravityBall && GravityBall->IsDettached)
	{
		GravityBall->IsDettached = false;
		HookRope->SetVisibility(false);
		GravityBall->AppearDissapearGravityBall(true);
		GravityBall->ResizeAreaOfGravity(true);
	}
}

void AFPSGameplayCharacter::OnHook()
{
	if (GravityBall->GravityMode == E_GravityMode::MODE_HOOK && GravityBall->IsDettached && !GravityBall->IsMovingForward /*&& GetCharacterMovement()->IsFalling()*/)
	{
		HookRope->SetVisibility(true);
		HookRope->SetWorldLocation(GravityBall->GetActorLocation());
		HookRope->EndLocation = FVector::ZeroVector;
		FVector distanceBallActor = FP_MuzzleLocationHook->GetComponentLocation() - GravityBall->GetActorLocation();
		HookRope->CableLength = distanceBallActor.Size() - 500;
		IsSwinging = true;
	}
}

void AFPSGameplayCharacter::OnSetGravityModeAttraction() 
{
	if (GravityBall && GravityBall->GravityMode != E_GravityMode::MODE_ATTRACTION)
	{
		GravityBall->GravityMode = E_GravityMode::MODE_ATTRACTION;
		GravityBall->ChangeGravityMaterial(AttractMaterialInstance);
		if (GameHud)
		{
			GameHud->ChangeGravityModeUI(0);
		}
	}

}

void AFPSGameplayCharacter::OnSetGravityModeRepulsion()
{
	if (GravityBall && GravityBall->GravityMode != E_GravityMode::MODE_REPULSION)
	{
		GravityBall->GravityMode = E_GravityMode::MODE_REPULSION;
		GravityBall->ChangeGravityMaterial(RepulsionMaterialInstance);
		if (GameHud)
		{
			GameHud->ChangeGravityModeUI(1);
		}
	}

}

void AFPSGameplayCharacter::OnSetGravityModeHook()
{
	if (GravityBall && GravityBall->GravityMode != E_GravityMode::MODE_HOOK)
	{
		GravityBall->GravityMode = E_GravityMode::MODE_HOOK;
		GravityBall->ChangeGravityMaterial(HookMaterialInstance);
		if (GameHud)
		{
			GameHud->ChangeGravityModeUI(2);
		}
	}

}

void AFPSGameplayCharacter::HangFromGravityHook()
{
	HookRope->SetWorldLocation(GravityBall->GetActorLocation());
	HookRope->EndLocation = FVector::ZeroVector;
	FVector forceDirection = GetActorLocation() - GravityBall->GetActorLocation();
	float forceMagnitude = FVector::DotProduct(GetVelocity(), forceDirection);
	forceDirection.Normalize();
	FVector forceVector = forceDirection * forceMagnitude * ForceSwingMagnitude;
	GetCharacterMovement()->AddForce(forceVector);
}

void AFPSGameplayCharacter::OnUnhook()
{
	HookRope->SetVisibility(false);
	HookRope->EndLocation = FVector::ZeroVector;
	HookRope->CableLength = 0;
	IsSwinging = false;
}

void AFPSGameplayCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AFPSGameplayCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AFPSGameplayCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

void AFPSGameplayCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFPSGameplayCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFPSGameplayCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFPSGameplayCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AFPSGameplayCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AFPSGameplayCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AFPSGameplayCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AFPSGameplayCharacter::TouchUpdate);
		return true;
	}

	return false;
}
