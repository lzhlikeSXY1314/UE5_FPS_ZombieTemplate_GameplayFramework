// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/DismembermentComponent.h"
#include "GoreComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent, DisplayName = "Dismember Gore Component"))
class DISMEMBERMENTSYSTEM_API UGoreComponent : public UDismembermentComponent
{
	GENERATED_BODY()
	

public:
	virtual void BeginPlay() override;

	void InitializeVertexColors();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dismemberment|FX")
	EDismemberColorChannel BloodVertexChannel = EDismemberColorChannel::B_Channel;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dismemberment|Advanced", meta = (ToolTip = "The Custom Primitive Index for the Blood Animation Scalar Parameter in the MF_DIS_ApplyBlood function"))
	int32 BloodAnimationPrimitiveIndex = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dismemberment|FX|Decal", meta = (ToolTip = "Scales the Size of the Blood Pool Decal."))
	float BloodPoolScale = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dismemberment|FX|Decal")
	bool bOverrideBloodDecal = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dismemberment|FX|Decal", meta = (EditCondition = "bOverrideBloodDecal", EditConditionHides = "true"))
	class UMaterialInterface* BloodDecal = nullptr;

	class UMaterialInterface* GetBloodDecal();


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dismemberment|FX")
	bool bOverrideBloodParticles = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dismemberment|FX", meta = (EditCondition = "bOverrideBloodParticles", EditConditionHides = "true"))
	class UNiagaraSystem* FX_BloodBurst = nullptr;

	class UNiagaraSystem* GetBloodBurstFX();



/*
*	Config
*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dismemberment|FX", meta = (ToolTip = "Scales the Size of the Blood Pool Niagara Particles."))
	float BloodParticleScale = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dismemberment|FX|BloodPaint", meta = (ToolTip = "Scales the Amount of blood that gets painted on the Skeletal Mesh"))
	float BloodPaintScale = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dismemberment|FX|BloodPaint", meta = (ToolTip = "The Size in, Unreal Units, of the area that the blood paints of the Skeletal Mesh"))
	float BloodPaintRadius = 120.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dismemberment|FX|BloodPaint", meta = (ToolTip = "The Size in, Unreal Units, of the area that the blood paints on the Dismembered Limbs"))
	float BloodPaintLimbRadius = 75.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dismemberment|FX|BloodPaint", meta = (ToolTip = "The Falloff of the blood that gets painted on the Skeletal Mesh"))
	float BloodPaintFalloff = 0.2f;

protected:
	virtual void PreDismemberment(const FName BoneName, FVector Impulse) override;
	virtual void PostDismemberment(const FName BoneName, USkeletalMeshComponent* DismemberMesh) override;

private:
	void LineTraceForBloodPool(FVector HitLocation, FVector Direction);
	void SpawnBloodPool(FVector Location, FVector Normal, FVector SplatterDirection, USceneComponent* Attachment = nullptr);


/*
*	Blood Painting
*/
	static float SphereMask(const FVector& Center, const FVector& Location, const float& Radius, const float& Hardness);

	UFUNCTION(BlueprintCallable, Category = "Dismemberment", meta = (ToolTip = "Applies blood to the Owning Characters Mesh"))
	void ApplyBlood(const FName BoneName, const float Radius = 75.f, const float Hardness = 0.2f);

	UFUNCTION(BlueprintCallable, Category = "Dismemberment", meta = (ToolTip = "Applies blood to a specified Skeletal Mesh"))
	void ApplyBloodToMesh(USkeletalMeshComponent* Mesh, const FName BoneName, const float Radius = 75.f, const float Hardness = 0.2f);


/*
*	Material
*/
	UFUNCTION()
	void BeginBloodAnimation();

	UFUNCTION()
	void TickBloodAnimation();

	void SetBloodAnimation(USkeletalMeshComponent* InMesh, float In) const;
	UPROPERTY(EditAnywhere, Category = "Dismemberment|FX|BloodPaint", meta = (ToolTip = "The Amount of time it takes for blood to apply to a Mesh"))

	float BloodAnimationTime = 1.f;
	float BloodAnimation = 0.f;

/*
*	Helper Functions
*/
	FRotator GetRotationForBlood(FVector Normals);
	FRotator GetRotationForBloodActor(FVector Normals) const;


public:
	UFUNCTION(BlueprintCallable, Category = "Gore | Blood")
	void SpawnBloodPoolFromBleedingHits(float InBloodParticleScale , float InBloodPoolScale, FVector HitLocation, FRotator InSpawnRotation, float ReflectedRayLength = 500, FVector2D ReflectedDecalMappingSize = FVector2D(0.3f, 1.2f));

	UFUNCTION(BlueprintCallable, Category = "Gore | Blood")
	void SpawnPenetratingBloodPool(bool Disabled, FVector HitLocation, FVector ShotDirection, FVector2D PenetratingDecalMappingSize = FVector2D(0.3f, 1.2f), float PenetratingRayLength = 500.0f, float InBloodPoolScale = 1.f);


};
