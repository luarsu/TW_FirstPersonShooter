// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityBall.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StaticMeshComponent.h"
#include "UProjectileMovementCompModified.h"

// Sets default values
AGravityBall::AGravityBall()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGravityBall::BeginPlay()
{
	Super::BeginPlay();

	GravityBallMesh_Component = FindComponentByClass<UStaticMeshComponent>();
	if (!GravityBallMesh_Component)
	{
		UE_LOG(LogTemp, Warning, TEXT("The mesh for the gravity ball doesn't exist!"));
	}
	/*else
	{
		RootComponent = GravityBallMesh_Component;
	}*/

	GravityAreaTrigger = FindComponentByClass<USphereComponent>();
	if (!GravityAreaTrigger)
	{
		UE_LOG(LogTemp, Warning, TEXT("The trigger for the gravity effect doesn't exist!"));
	}
	else
	{
		GravityAreaTrigger->OnComponentBeginOverlap.AddDynamic(this, &AGravityBall::OnOverlapGravityBegin);
		GravityAreaTrigger->OnComponentEndOverlap.AddDynamic(this, &AGravityBall::OnOverlapGravityEnd);
	}
}

// Called every frame
void AGravityBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ApplyGravityEffect(DeltaTime);

	if (IsMovingForward)
	{
		MoveForward(DeltaTime);
	}
}

/** Function that handles applying the gravity forces */
void AGravityBall::ApplyGravityEffect(float DeltaTime)
{
	if (IsGravityActive && GravityMode != E_GravityMode::MODE_HOOK)
	{
		for (int i = 0; i < AffectedActors.Num(); i++)
		{
			FVector direction = AffectedActors[i]->GetActorLocation() - this->GetActorLocation();

			if (ACharacter* character = Cast<ACharacter>(AffectedActors[i]))
			{
				UCharacterMovementComponent* move = character->GetCharacterMovement();
				if (GravityMode == E_GravityMode::MODE_ATTRACTION)
				{
					move->AddForce(-direction * AttractForce * move->Mass);
				}
				else
				{
					move->AddForce(direction * RepulsionForce * move->Mass);
				}

			}
			else
			{
				UStaticMeshComponent* actorMesh = Cast<UStaticMeshComponent>(AffectedActors[i]->FindComponentByClass<UStaticMeshComponent>());
				if (actorMesh && actorMesh->IsSimulatingPhysics())
				{
					if (GravityMode == E_GravityMode::MODE_ATTRACTION)
					{
						actorMesh->AddForce(-direction * AttractForce * actorMesh->GetMass() );
					}
					else
					{
						actorMesh->AddForce(direction * RepulsionForce * actorMesh->GetMass() );
					}
				}
			}
		}
	}
}

/** Called when the ball is moving forward */
void AGravityBall::MoveForward(float DeltaTime)
{
	FVector newPosition = GetActorLocation() + (GetActorForwardVector() * movementSpeed * DeltaTime);
	SetActorLocation(newPosition, true);

	if (AttachToGunComponent)
	{
		if (GetDistanceTo(AttachToGunComponent->GetAttachmentRootActor()) >= maxDistanceToplayer && IsDettached)
		{
			StopMoving();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("This gun doesn't have a player parent!"));
	}
}

/** Called the character shoots the ball */
void AGravityBall::ShootBall()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	IsMovingForward = true;
	IsDettached = true;
}

/** Called when the character asks for the ball to come back */
void AGravityBall::StopMoving()
{
	IsMovingForward = false;
	IsGravityActive = true;
	ResizeAreaOfGravity(false);
}

void AGravityBall::ReturnBall()
{
	IsGravityActive = false;
	IsMovingForward = false;
	IsDettached = false;

	 APawn * owner = Cast<APawn>(AttachToGunComponent->GetOwner());
	if (owner)
	{
		const FRotator SpawnRotation = owner->GetControlRotation();
		const FVector SpawnLocation = AttachToGunComponent->GetComponentLocation() + SpawnRotation.RotateVector(GunOffset);

		SetActorLocation(SpawnLocation);
		SetActorRotation(SpawnRotation);
		AttachToComponent(AttachToGunComponent, FAttachmentTransformRules::KeepWorldTransform);
		AppearDissapearGravityBall(false);
	}
	
}

/** called when something enters in the gravity area */
void AGravityBall::OnOverlapGravityBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Other Actor is the actor that triggered the event. Check that is not ourself.  
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		//this should be called only once per projectile as once we set the flat for homing mode we don't need to update it every tick
		if (AFPSGameplayProjectile* projectile = Cast<AFPSGameplayProjectile>(OtherActor))
		{
			UUProjectileMovementCompModified* projectileMove = projectile->GetProjectileMovement();
			if (IsGravityActive && projectileMove)
			{
				projectileMove->bIsHomingProjectile = true;
				projectileMove->HomingAccelerationMagnitude = ProjectileHomingAcceleration;
				projectileMove->HomingTargetComponent = GravityBallMesh_Component;
				if (GravityMode == E_GravityMode::MODE_ATTRACTION)
				{
					projectileMove->bIsHomingInverted = false;
				}
				else
				{
					projectileMove->bIsHomingInverted = true;
				}

				AffectedProjectiles.Add(projectile);
			}
		}
		else
		{
			AffectedActors.Add(OtherActor);
		}
	}
}

/** called when something leaves the gravity area */
void AGravityBall::OnOverlapGravityEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Other Actor is the actor that triggered the event. Check that is not ourself.  
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		if (AFPSGameplayProjectile* projectile = Cast<AFPSGameplayProjectile>(OtherActor))
		{
			//if it's a projectile disable their homing mode before removing them from the list
			if (UUProjectileMovementCompModified* projectileMove = projectile->GetProjectileMovement())
			{
				projectileMove->bIsHomingProjectile = false;
				AffectedProjectiles.Remove(projectile);
			}
		}
		AffectedActors.Remove(OtherActor);
	}
}