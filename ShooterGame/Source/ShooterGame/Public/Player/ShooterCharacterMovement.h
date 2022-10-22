// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * Movement component meant for use with Pawns.
 */

#pragma once
#include "ShooterCharacterMovement.generated.h"

#define FLAG_TELEPORT FLAG_Custom_0
#define FLAG_JETPACK FLAG_Custom_1

class FSavedMove_Custom : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;
	virtual uint8 GetCompressedFlags() const override;
	virtual void Clear() override;
	virtual void PrepMoveFor(ACharacter* Character) override;
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;

private:
	bool bWantsToTeleport;
	bool bWantsToFly;

};

class FNetworkPredictionData_Shooter : public FNetworkPredictionData_Client_Character
{
public:
	FNetworkPredictionData_Shooter(const UShooterCharacterMovement& ClientMovement);
	virtual FSavedMovePtr AllocateNewMove() override;
};

UCLASS()
class UShooterCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

	friend class FSavedMove_Custom;
public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void OnComponentDestroyed(bool bDestroyedHierachy) override;
	virtual void PerformMovement(float DeltaSeconds) override;
	virtual float GetMaxSpeed() const override;
	virtual void BeginPlay() override;


	void UpdateFromCompressedFlags(uint8 Flags) override;

	void OnTeleport();

	void OnFly();

	void OnStopFly();

	void Teleport();

	void OnTeleportStop();

	bool CanFly() const;

	bool bWantsToTeleport;
	bool bWantsToFly;


	UPROPERTY(EditAnywhere, Category = Ability)
	float TeleportCoolDown;

	UPROPERTY(EditAnywhere, Category = Ability)
	float TeleportDistance;

	UPROPERTY(EditAnywhere, Category = "Ability|Jetpack")
	float MaxJetpackFuel;

	UPROPERTY(EditAnywhere, Category = "Ability|Jetpack")
	float StartVelocityZ;

	UPROPERTY(EditAnywhere, Category = "Ability|Jetpack")
	float MaxVelocityZ;

	UPROPERTY(EditAnywhere, Category = "Ability|Jetpack")
	float JetpackConsumeRate;

	UPROPERTY(EditAnywhere, Category = "Ability|Jetpack")
	float JetpackZSpeedVariation;
private:
	bool CanTeleport() const;
	float LastTeleport;
	float JetpackFuel;
	float ActualZSpeed;

};

