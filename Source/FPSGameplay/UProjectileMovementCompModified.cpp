// Fill out your copyright notice in the Description page of Project Settings.


#include "UProjectileMovementCompModified.h"

// Sets default values for this component's properties
UUProjectileMovementCompModified::UUProjectileMovementCompModified()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UUProjectileMovementCompModified::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UUProjectileMovementCompModified::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

// Allow the projectile to track towards its homing target.
FVector UUProjectileMovementCompModified::ComputeHomingAcceleration(const FVector& InVelocity, float DeltaTime) const
{
	FVector HomingAcceleration = ((HomingTargetComponent->GetComponentLocation() - UpdatedComponent->GetComponentLocation()).GetSafeNormal() * HomingAccelerationMagnitude);

	//modification so that the homing mode can be used to repulse the projectile as well
	HomingAcceleration = bIsHomingInverted ? -HomingAcceleration : HomingAcceleration;

	return HomingAcceleration;
}
