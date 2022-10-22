// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/ShooterDamageType.h"
#include "NiagaraSystem.h"
#include "SpecialShooterDamageType.generated.h"

UENUM(BlueprintType)
enum class SpecialType : uint8 {
	DEFAULT UMETA(DisplayName = "Default"),
	ICE		UMETA(DisplayName = "Ice"),
	//We can add more type like this
	//FIRE	UMETA(DisplayName = "Fire"),
};

/**
 * 
 */
UCLASS(const, Blueprintable, BlueprintType)
class SHOOTERGAME_API USpecialShooterDamageType : public UShooterDamageType
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category = DamageSettings)
	SpecialType Type;

	UPROPERTY(EditDefaultsOnly, Category = DamageSettings)
	float EffectDuration;

	UPROPERTY(EditDefaultsOnly, Category = DamageSettings)
	bool bBlockMovement;

	UPROPERTY(EditDefaultsOnly, Category = DamageSettings)
	bool bBlockShooting;

	UPROPERTY(EditDefaultsOnly, Category = NiagaraEffect)
	UNiagaraSystem* NiagaraSystem;

};
