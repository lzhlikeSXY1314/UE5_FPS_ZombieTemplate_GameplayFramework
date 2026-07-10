// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BloodPool.generated.h"

UCLASS()
class DISMEMBERMENTSYSTEM_API ABloodPool : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABloodPool();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UBoxComponent* Collision;
	UPROPERTY(EditAnywhere, Category = "Components")
	class UDecalComponent* Decal;

protected:
	FVector RemapSizeForDecal(const FVector In) const;
	


public:
	UPROPERTY(BlueprintReadWrite, Category = "Dismemberment", meta = (ExposeOnSpawn = "true", ToolTip = "Adds a Delay before the Blood Decal Splatters"))
	float StartDelay = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Dismemberment", meta = (ExposeOnSpawn = "true", ToolTip = "The Lifetime of the blood before it dries up and fades away"))
	float MaxLifetime = 60.f;
	UPROPERTY(BlueprintReadWrite, Category = "Dismemberment", meta = (ExposeOnSpawn = "true", ToolTip = "This is a wrapper for Decal Size"))
	FVector DecalSize = FVector(100);
	UPROPERTY(BlueprintReadWrite, Category = "Dismemberment", meta = (ExposeOnSpawn = "true", ToolTip = "This can be used to set a relative rotation of the Decal"))
	FRotator DecalRotation = FRotator(0);

	UPROPERTY(BlueprintReadWrite, Category = "Dismemberment", meta = (ExposeOnSpawn = "true", ToolTip = "This is the Time it will take in seconds to Lerp to the new Size and Location"))
	float InterpTime = 0.3f;

	UPROPERTY(BlueprintReadWrite, Category = "Dismemberment", meta = (ExposeOnSpawn = "true"))
	UMaterialInterface* DecalMaterial = nullptr;

};
