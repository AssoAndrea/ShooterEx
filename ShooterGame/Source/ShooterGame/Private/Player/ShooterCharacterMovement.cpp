// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Player/ShooterCharacterMovement.h"

//----------------------------------------------------------------------//
// UPawnMovementComponent
//----------------------------------------------------------------------//
UShooterCharacterMovement::UShooterCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


FNetworkPredictionData_Client* UShooterCharacterMovement::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UShooterCharacterMovement* MutableThis = const_cast<UShooterCharacterMovement*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Shooter(*this);
	}
	return ClientPredictionData;
}

void UShooterCharacterMovement::OnComponentDestroyed(bool bDestroyedHierachy)
{
}

void UShooterCharacterMovement::PerformMovement(float DeltaSeconds)
{
	Super::PerformMovement(DeltaSeconds);

	if (bWantsToTeleport)
	{
		Teleport();
	}

	if (bWantsToFly && JetpackFuel >= 0)
	{
		Fly(DeltaSeconds);
		SetMovementMode(EMovementMode::MOVE_Flying); //other way can be override physCustom and use custom flag movement
	}
	else if (bWantsToFly)
	{
		OnStopFly();
	}
	if (JetpackFuel < MaxJetpackFuel && IsMovingOnGround())
	{
		JetpackFuel += JetpackConsumeRate * DeltaSeconds;
	}
}

void UShooterCharacterMovement::ServerStopFly_Implementation()
{
	SetMovementMode(EMovementMode::MOVE_Falling);
}

void UShooterCharacterMovement::Fly(float DeltaSeconds)
{
	FVector newVelocity = FVector(Velocity.X, Velocity.Y, JetpackVelocity);

	Velocity = newVelocity;

	JetpackFuel -= JetpackConsumeRate * DeltaSeconds;
}
float UShooterCharacterMovement::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	if (ShooterCharacterOwner)
	{
		if (ShooterCharacterOwner->IsTargeting())
		{
			MaxSpeed *= ShooterCharacterOwner->GetTargetingSpeedModifier();
		}
		if (ShooterCharacterOwner->IsRunning())
		{
			MaxSpeed *= ShooterCharacterOwner->GetRunningSpeedModifier();
		}
	}

	return MaxSpeed;
}

void UShooterCharacterMovement::BeginPlay()
{
	Super::BeginPlay();
	JetpackFuel = MaxJetpackFuel;
}



FNetworkPredictionData_Shooter::FNetworkPredictionData_Shooter(const UShooterCharacterMovement& ClientMovement) : FNetworkPredictionData_Client_Character(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Shooter::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Custom());
}

uint8 FSavedMove_Custom::GetCompressedFlags() const
{
	uint8 result = Super::GetCompressedFlags();
	if (bWantsToTeleport)
	{
		result |= FLAG_TELEPORT;
	}
	if (bWantsToFly)
	{
		result |= FLAG_JETPACK;
	}
	return result;
}

void FSavedMove_Custom::Clear()
{
	Super::Clear();
	bWantsToTeleport = false;
	bWantsToFly = false;
}

void FSavedMove_Custom::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);
	UShooterCharacterMovement* CharacterMovement = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		CharacterMovement->bWantsToTeleport = bWantsToTeleport;
		CharacterMovement->bWantsToFly = bWantsToFly;
	}
}

void FSavedMove_Custom::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);
	const UShooterCharacterMovement* CharacterMovement = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		bWantsToTeleport = CharacterMovement->bWantsToTeleport;
		bWantsToFly = CharacterMovement->bWantsToFly;
	}
}

void UShooterCharacterMovement::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bWantsToTeleport = ((Flags & FSavedMove_Character::FLAG_TELEPORT) != 0);
	bWantsToFly = ((Flags & FSavedMove_Character::FLAG_JETPACK) != 0);
}


void UShooterCharacterMovement::OnTeleport()
{
	bWantsToTeleport = true;
}

void UShooterCharacterMovement::OnFly()
{
	bWantsToFly = true;
}

void UShooterCharacterMovement::OnStopFly()
{
	bWantsToFly = false;
	if (!CharacterOwner->HasAuthority())
	{
		ServerStopFly();
	}
}

void UShooterCharacterMovement::Teleport()
{
	if (!CanTeleport())
	{
		return;
	}
	FVector newPosition = GetActorLocation() + (CharacterOwner->GetActorForwardVector() * TeleportDistance);
	FHitResult hit;

	FVector _, PlayerDimension; //need only player dimension
	CharacterOwner->GetActorBounds(true, _, PlayerDimension);


	//check for obstacles
	bool hitted = GetWorld()->LineTraceSingleByObjectType(hit, GetActorLocation(), newPosition,ECollisionChannel::ECC_WorldStatic);

	FVector TeleportPosition = hitted ? hit.ImpactPoint - CharacterOwner->GetActorForwardVector() * PlayerDimension.X/2  : newPosition;

	if (CharacterOwner->SetActorLocation(TeleportPosition))
	{
		LastTeleport = GetWorld()->TimeSeconds;
		bWantsToTeleport = false;
	}

}

void UShooterCharacterMovement::OnTeleportStop()
{
	bWantsToTeleport = false;
}




bool UShooterCharacterMovement::CanTeleport() const
{
	float ActualTime = GetWorld()->TimeSeconds;
	if (LastTeleport == 0 || (ActualTime - LastTeleport) > TeleportCoolDown)
	{
		return true;
	}
	return false;
}
