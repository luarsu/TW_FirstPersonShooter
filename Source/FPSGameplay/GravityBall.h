// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "FPSGameplayProjectile.h"
#include "Materials/MaterialInstance.h"
#include "GravityBall.generated.h"

/** Enum for the different modes of the gravity ball */
UENUM(BlueprintType)
enum class E_GravityMode : uint8
{
	MODE_ATTRACTION = 0	UMETA(DisplayName = "Attraction"),
	MODE_REPULSION 	UMETA(DisplayName = "Repulsion"),
	MODE_HOOK	UMETA(DisplayName = "Hook")
};

UCLASS()
class FPSGAMEPLAY_API AGravityBall : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGravityBall();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** called when something enters in the gravity area */
	UFUNCTION()
		void OnOverlapGravityBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** called when something leaves the gravity area */
	UFUNCTION()
		void OnOverlapGravityEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
public:

	/** True if the gravity attraction/repulsion is activated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gravity)
		bool IsGravityActive;

	/** Force applied for the attraction effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gravity)
		float AttractForce;

	/** Force applied for the attraction effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gravity)
		float RepulsionForce;

	/** Acceleration applied when affection projectiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gravity)
		float ProjectileHomingAcceleration;

	/** Mode the gravity ball is in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gravity)
		E_GravityMode GravityMode;

	/** Array of objects that are in the orbit of the gravity ball */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gravity)
		TArray <AActor*> AffectedActors;

	/** Array of projectiles that are in the orbit of the gravity ball. In a separate array because we don't want to call the update function in them all the time like for the other actors (we are using the homing component) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gravity)
		TArray <AFPSGameplayProjectile*> AffectedProjectiles;

	/** Movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		float movementSpeed;

	/** Max distance to the player for the ball to stop moving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		float maxDistanceToplayer;

	/** True if it's moving forward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		bool IsMovingForward;
	
	/** True if it's not attached to the gun */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		bool IsDettached;

	/** The scene component the ball is attached when its in the gun */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		class USceneComponent* AttachToGunComponent;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		FVector GunOffset;

	/** The static mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Components)
		class UStaticMeshComponent* GravityBallMesh_Component;

	/** The trigger for the gravity area of effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Components)
		class USphereComponent* GravityAreaTrigger;

	/** Changes the color of the gravity effect */
	UFUNCTION(BlueprintImplementableEvent, Category = GravityBall)
		void ChangeGravityMaterial(UMaterialInstance* material);

	/** Function makes a little resize animation for the ball gravity area */
	UFUNCTION(BlueprintImplementableEvent, Category = GravityBall)
		void ResizeAreaOfGravity(bool dissapear);

	/** Function makes a little resize animation for the ball when it disapears/appears */
	UFUNCTION(BlueprintImplementableEvent, Category = GravityBall)
		void AppearDissapearGravityBall(bool dissapear);

	/** Function makes a little resize animation for the ball when it disapears/appears */
	UFUNCTION(BlueprintCallable, Category = GravityBall)
		void ReturnBall();

	/** Function that handles applying the gravity forces */
	UFUNCTION()
		void ApplyGravityEffect(float DeltaTime);

	/** Called when the ball is moving forward */
	UFUNCTION()
		void MoveForward(float DeltaTime);

	/** Called when the character wants to stop the ball moving. The ball gravity is only active when it's not moving */
	UFUNCTION()
		void StopMoving();

	/** Called when the character shoots the ball */
	UFUNCTION()
		void ShootBall();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
