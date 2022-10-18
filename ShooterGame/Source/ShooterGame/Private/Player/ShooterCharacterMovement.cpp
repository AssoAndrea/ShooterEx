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
		result |= FLAG_TELEPORT; //Teleport
	}
	return result;
}

void FSavedMove_Custom::Clear()
{
	Super::Clear();
	bWantsToTeleport = false;
}

void FSavedMove_Custom::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);
	UShooterCharacterMovement* DefaultCharacterMovement = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (DefaultCharacterMovement)
	{
		DefaultCharacterMovement->bWantsToTeleport = bWantsToTeleport;
	}
}

void FSavedMove_Custom::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);
	const UShooterCharacterMovement* DefaultCharacterMovement = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (DefaultCharacterMovement)
	{
		bWantsToTeleport = DefaultCharacterMovement->bWantsToTeleport;
	}
}

void UShooterCharacterMovement::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	//UE_LOG(LogTemp, Warning, TEXT("flag = %u"), Flags);
	bWantsToTeleport = ((Flags & FSavedMove_Character::FLAG_Custom_0) != 0);
	if (CharacterOwner->GetLocalRole() == ROLE_Authority)
	{
		if (bWantsToTeleport)
		{
			Teleport();
			bWantsToTeleport = false;
		}
	}

}


void UShooterCharacterMovement::OnTeleport()
{
	if (CharacterOwner->IsLocallyControlled())
	{
		Teleport();
	}
}

void UShooterCharacterMovement::Teleport()
{
	if (!CanTeleport())
	{
		return;
	}
	FVector newPosition = CharacterOwner->GetActorForwardVector() * TeleportDistance;
	CharacterOwner->AddActorWorldOffset(newPosition, true);
	LastTeleport = GetWorld()->TimeSeconds;
	bWantsToTeleport = true;
}

void UShooterCharacterMovement::TeleportStop()
{
	//return;
	bWantsToTeleport = false;
}

bool UShooterCharacterMovement::CanTeleport()
{
	float ActualTime = GetWorld()->TimeSeconds;
	if (LastTeleport == 0 || (ActualTime - LastTeleport) > TeleportCoolDown)
	{
		return true;
	}
	return false;
}
