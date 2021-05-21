// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UProjectileMovementCompModified.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSGAMEPLAY_API UUProjectileMovementCompModified : public UProjectileMovementComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUProjectileMovementCompModified();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	

	//If true the homing acceleration is inverted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Homing)
	bool bIsHomingInverted;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Allow the projectile to track towards its homing target. Modified so that gravity ball can affect it*/
	virtual FVector ComputeHomingAcceleration(const FVector& InVelocity, float DeltaTime) const override;
};
