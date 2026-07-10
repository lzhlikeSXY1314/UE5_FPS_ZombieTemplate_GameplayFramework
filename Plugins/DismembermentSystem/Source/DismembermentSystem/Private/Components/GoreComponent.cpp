// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GoreComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Gore/BloodPool.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Rendering/SkeletalMeshRenderData.h"




void UGoreComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetSkeletalMeshComponent()) return;

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UGoreComponent::InitializeVertexColors);
}

void UGoreComponent::InitializeVertexColors() 
{
	for (USkeletalMeshComponent* Mesh : GetAllMeshes())
	{
		if (!IsValid(Mesh)) continue;

		for (int32 i = 0; i < Mesh->GetNumLODs(); i++)
		{
			TArray<FLinearColor> Colors = GetCurrentVertexColors(Mesh, i);
			SetLinearColorChannel(Colors, 0.f, BloodVertexChannel);
			Mesh->SetVertexColorOverride_LinearColor(i, Colors);
		}
	}
}

UMaterialInterface* UGoreComponent::GetBloodDecal()
{

	//if (bOverrideBloodDecal)
	//{
	//	if (bOverrideBloodDecalTexture) BloodDecal = GetDynamicBloodDecal();

	//	return BloodDecal;
	//}

	//if (!BloodDecal)
	//{
	//	if (bOverrideBloodDecalTexture)
	//	{
	//		BloodDecal = LoadObject<UMaterialInterface>(nullptr, TEXT("/DismembermentSystem/EnGoreDismembermentSystem/Gore/Mats/M_Decal_BloodPool_Custom.M_Decal_BloodPool_Custom"));

	//		BloodDecal = GetDynamicBloodDecal();
	//	}
	//	else
	//	{
	//		BloodDecal = LoadObject<UMaterialInterface>(nullptr, TEXT("/DismembermentSystem/EnGoreDismembermentSystem/Gore/Mats/M_Decal_BloodPool.M_Decal_BloodPool"));
	//	}
	//} ///

	//return BloodDecal;


	if (bOverrideBloodDecal)
	{
		return BloodDecal;
	}
	else
	{
		return BloodDecal = LoadObject<UMaterialInterface>(nullptr, TEXT("/DismembermentSystem/BloodSystem/Materials/M_BaseDecal_Inst.M_BaseDecal_Inst"));
	}






}



UNiagaraSystem* UGoreComponent::GetBloodBurstFX()
{
	if (bOverrideBloodParticles) return FX_BloodBurst;

	if (!FX_BloodBurst) FX_BloodBurst = LoadObject<UNiagaraSystem>(nullptr, TEXT("/DismembermentSystem/EnGoreDismembermentSystem/Gore/NS_DIS_BloodBurst.NS_DIS_BloodBurst"));

	return FX_BloodBurst;
}

void UGoreComponent::PreDismemberment(const FName BoneName, FVector Impulse)
{
	Super::PreDismemberment(BoneName, Impulse);

	const FVector HitLocation = GetSkeletalMeshComponent()->GetSocketLocation(BoneName);

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, GetBloodBurstFX(), HitLocation, Impulse.Rotation(), FVector(BloodParticleScale));

	LineTraceForBloodPool(HitLocation, Impulse.GetSafeNormal());
}

void UGoreComponent::PostDismemberment(const FName BoneName, USkeletalMeshComponent* DismemberMesh)
{
	if (bSupportAttachedChildMeshes)
	{
		ApplyBlood(BoneName, BloodPaintRadius, BloodPaintFalloff);

		for (USkeletalMeshComponent* DisMesh : GetAllAttachedMeshes(DismemberMesh)) ApplyBloodToMesh(DisMesh, BoneName, BloodPaintLimbRadius, BloodPaintFalloff);
	}
	else
	{
		CopyVertexColorsToMesh(GetSkeletalMeshComponent(), DismemberMesh);

		ApplyBloodToMesh(GetSkeletalMeshComponent(), BoneName, BloodPaintRadius, BloodPaintFalloff);
		ApplyBloodToMesh(DismemberMesh, BoneName, BloodPaintLimbRadius, BloodPaintFalloff);
	}

	Super::PostDismemberment(BoneName, DismemberMesh);
}

void UGoreComponent::LineTraceForBloodPool(FVector HitLocation, FVector Direction)
{
	FVector End = Direction;
	End.Z = -0.7;
	End *= 2000.f;
	End += HitLocation;

	FHitResult HitResult;

	UKismetSystemLibrary::LineTraceSingleForObjects(GetOwner(), HitLocation, End, { EObjectTypeQuery::ObjectTypeQuery1 }, true, {}, EDrawDebugTrace::ForDuration, HitResult, true);

	if (!HitResult.bBlockingHit) return;

	SpawnBloodPool(HitResult.ImpactPoint, HitResult.ImpactNormal, Direction, HitResult.GetComponent());
}

void UGoreComponent::SpawnBloodPool(FVector Location, FVector Normal, FVector SplatterDirection, USceneComponent* Attachment)
{
	FTransform Transform;
	Transform.SetLocation(Location);
	Transform.SetRotation(GetRotationForBloodActor(Normal).Quaternion());
	Transform.SetScale3D(FVector(1));

	ABloodPool* Blood = GetWorld()->SpawnActorDeferred<ABloodPool>(ABloodPool::StaticClass(), Transform, GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	Blood->StartDelay = 0.25f;
	Blood->DecalSize =  FVector::OneVector; //Y : FMath::Lerp(2.f, 0.5f, FMath::Abs(SplatterDirection.Z)) 
	Blood->DecalSize *= BloodPoolScale;
	Blood->DecalSize.Z = 0.1f;

	Blood->DecalRotation = GetRotationForBlood(SplatterDirection);
	Blood->InterpTime = 0.3f;
	Blood->DecalMaterial = GetBloodDecal();

	Blood->FinishSpawning(Transform);
	if (Attachment) Blood->AttachToComponent(Attachment, FAttachmentTransformRules::KeepWorldTransform);
}

float UGoreComponent::SphereMask(const FVector& Center, const FVector& Location, const float& Radius, const float& Hardness)
{
	const float InvRadius = 1 / Radius;
	const float NormalizeDistance = FVector::Distance(Location, Center) * InvRadius;

	float InvHardness = 1 - Hardness;
	InvHardness = 1 / FMath::Max(InvHardness, 0.00001f);

	return FMath::Clamp((1 - NormalizeDistance) * InvHardness, 0, 1);
}

void UGoreComponent::ApplyBlood(const FName BoneName, const float Radius, const float Hardness)
{
	for (USkeletalMeshComponent* Mesh : GetAllMeshes())
	{
		if (Mesh->ComponentTags.Contains("Ignore Blood")) continue;

		ApplyBloodToMesh(Mesh, BoneName, Radius, Hardness);
	}
}

void UGoreComponent::ApplyBloodToMesh(USkeletalMeshComponent* Mesh, const FName BoneName, const float Radius, const float Hardness)
{
	const FVector LocalHit = FTransform(GetMesh()->GetComposedRefPoseMatrix(BoneName)).GetLocation();

	for (int32 LOD = 0; LOD < Mesh->GetNumLODs(); LOD++)
	{
		FPositionVertexBuffer& Buffer = Mesh->GetSkeletalMeshAsset()->GetResourceForRendering()->LODRenderData[LOD].StaticVertexBuffers.PositionVertexBuffer;
		const int32 Num = Buffer.GetNumVertices();

		if (!Buffer.IsInitialized())
		{
				return;		
		}

		if (!Buffer.GetAllowCPUAccess())
		{
				return;
		}

		TArray<float> Mask;
		Mask.Init(1.f, Num);
		//		Mask.SetNum(Num);

		for (int32 i = 0; i < Num; i++)
		{
			FVector VertexLocation(Buffer.VertexPosition(i));

			Mask[i] = SphereMask(LocalHit, VertexLocation, Radius, Hardness);
		}

		TArray<FLinearColor> Colors = GetCurrentVertexColors(Mesh, LOD);

		MaxLinearColorChannel(Colors, Mask, BloodVertexChannel);

		Mesh->SetVertexColorOverride_LinearColor(LOD, Colors);

		if (Mesh == GetSkeletalMeshComponent()) BeginBloodAnimation();
		else SetBloodAnimation(Mesh, 1.f);
	}
}

void UGoreComponent::BeginBloodAnimation()
{
	if (BloodAnimation > 0) return;

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UGoreComponent::TickBloodAnimation);
}

void UGoreComponent::TickBloodAnimation()
{
	BloodAnimation += GetWorld()->GetDeltaSeconds() / BloodAnimationTime;
	FMath::Clamp(BloodAnimation, BloodAnimation, 1);

	SetBloodAnimation(GetSkeletalMeshComponent(), BloodAnimation);

	if (BloodAnimation < 1) GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UGoreComponent::TickBloodAnimation);
}

void UGoreComponent::SetBloodAnimation(USkeletalMeshComponent* InMesh, float In) const
{
	InMesh->SetCustomPrimitiveDataFloat(BloodAnimationPrimitiveIndex, In);
}

FRotator UGoreComponent::GetRotationForBlood(FVector Normals)
{
	// If Normals are completely flat then it will mess up the math as you can't divide by 0
	//Normals = Normals + 0.0001;

	//Normals = Normals * -1;

	//float Dot = FVector::ForwardVector.Dot(Normals);

	//Dot = FMath::Clamp(Dot, -1.0f, 1.0f);

	//Dot = (180.0) / DOUBLE_PI * FMath::Acos(Dot);

	//return { 0, FRotationMatrix::MakeFromZ(Normals * Dot).Rotator().Yaw, 0 };

	Normals = Normals + 0.0001;

	Normals = Normals * -1;

	float Dot = FVector::ForwardVector.Dot(Normals);
	Dot = FMath::Clamp(Dot, -1.0f, 1.0f);
	Dot = (180.0) / PI * FMath::Acos(Dot);

	float Yaw = FRotationMatrix::MakeFromZ(Normals * Dot).Rotator().Yaw;

	float RandomRoll = FMath::FRandRange(0.0f, 359.9f);

	return FRotator(0, Yaw, RandomRoll);

}

FRotator UGoreComponent::GetRotationForBloodActor(FVector Normals) const
{
	float Dot = FVector::UpVector.Dot(Normals);;

	Dot = (180.0) / DOUBLE_PI * FMath::Acos(Dot);

	return FRotationMatrix::MakeFromZ(Normals * Dot).Rotator();
}

void UGoreComponent::SpawnBloodPoolFromBleedingHits(float InBloodParticleScale, float InBloodPoolScale, FVector HitLocation, FRotator InSpawnRotation, float ReflectedRayLength,FVector2D ReflectedDecalMappingSize)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,GetBloodBurstFX(),HitLocation,InSpawnRotation, FVector(InBloodParticleScale));


	FVector TraceDirection = InSpawnRotation.Vector();
	TraceDirection.Z = -0.7f;

	FVector TraceEnd = HitLocation + TraceDirection * ReflectedRayLength;

	FHitResult HitResult;
	bool bHit = UKismetSystemLibrary::LineTraceSingleForObjects(GetOwner(),HitLocation,TraceEnd,{ EObjectTypeQuery::ObjectTypeQuery1 },true,{},EDrawDebugTrace::ForDuration,HitResult,true);

	if (bHit && HitResult.bBlockingHit)
	{
		// ========== 距离计算：越远血越大 ==========
		float HitDistance = FVector::Distance(HitLocation, HitResult.ImpactPoint);
		float DistanceRatio = FMath::Clamp(HitDistance / ReflectedRayLength, 0.f, 1.0f);

		float MinSizeScale = ReflectedDecalMappingSize.X;
		float MaxSizeScale = ReflectedDecalMappingSize.Y;
		float DistanceSizeScale = FMath::Lerp(MinSizeScale, MaxSizeScale, DistanceRatio);
		// ========================================

		FTransform Transform;
		Transform.SetLocation(HitResult.ImpactPoint);
		Transform.SetRotation(GetRotationForBloodActor(HitResult.ImpactNormal).Quaternion());
		Transform.SetScale3D(FVector::OneVector);

		ABloodPool* BloodPool = GetWorld()->SpawnActorDeferred<ABloodPool>(
			ABloodPool::StaticClass(),
			Transform,
			GetOwner(),
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		if (BloodPool)
		{
			BloodPool->StartDelay = 0.25f;
			BloodPool->DecalSize = FVector::OneVector;  //FMath::Lerp(2.0f, 0.5f, FMath::Abs(TraceDirection.Z)), 拉伸问题 血液有点怪

			BloodPool->DecalSize *= InBloodPoolScale * DistanceSizeScale;

			BloodPool->DecalSize.Z = 0.01f;

			BloodPool->DecalRotation = GetRotationForBlood(TraceDirection);
			BloodPool->InterpTime = 0.3f;
			BloodPool->DecalMaterial = GetBloodDecal();

			BloodPool->FinishSpawning(Transform);

			if (HitResult.GetComponent())
			{
				BloodPool->AttachToComponent(HitResult.GetComponent(), FAttachmentTransformRules::KeepWorldTransform);
			}
		}
	}


}

void UGoreComponent::SpawnPenetratingBloodPool(bool Disabled,FVector HitLocation, FVector ShotDirection, FVector2D PenetratingDecalMappingSize, float PenetratingRayLength,float InBloodPoolScale)
{
	if (Disabled) return;
 
	FVector PenetrateEnd = HitLocation + ShotDirection * PenetratingRayLength;

	FHitResult PenetrateHit;
	bool bPenetrateHit = UKismetSystemLibrary::LineTraceSingleForObjects(GetOwner(),HitLocation,PenetrateEnd,{ EObjectTypeQuery::ObjectTypeQuery1 },true,{},EDrawDebugTrace::ForDuration,PenetrateHit,true);

	if (!bPenetrateHit || !PenetrateHit.bBlockingHit) return;

	float HitDistance = FVector::Distance(HitLocation, PenetrateHit.ImpactPoint);
	float DistanceRatio = FMath::Clamp(HitDistance / PenetratingRayLength, 0.0f, 1.0f);

	float MinSizeScale = PenetratingDecalMappingSize.X;
	float MaxSizeScale = PenetratingDecalMappingSize.Y;
	float DistanceSizeScale = FMath::Lerp(MinSizeScale, MaxSizeScale, DistanceRatio);

	FTransform Transform;
	Transform.SetLocation(PenetrateHit.ImpactPoint);
	Transform.SetRotation(GetRotationForBloodActor(PenetrateHit.ImpactNormal).Quaternion());
	Transform.SetScale3D(FVector::OneVector);

	ABloodPool* BloodPool = GetWorld()->SpawnActorDeferred<ABloodPool>(ABloodPool::StaticClass(),Transform,GetOwner(),nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!BloodPool) return;

	const FVector Normal = PenetrateHit.ImpactNormal;

	BloodPool->StartDelay = 0.1f;
	BloodPool->DecalSize = FVector::OneVector;

	BloodPool->DecalSize *= InBloodPoolScale * DistanceSizeScale;
	BloodPool->DecalSize.Z = float(0.01f);

	BloodPool->DecalRotation = GetRotationForBlood(ShotDirection);
	BloodPool->InterpTime = 0.2f;
	BloodPool->DecalMaterial = GetBloodDecal();

	BloodPool->FinishSpawning(Transform);


	if (PenetrateHit.GetComponent())
	{
		BloodPool->AttachToComponent(PenetrateHit.GetComponent(), FAttachmentTransformRules::KeepWorldTransform);
	}

}


