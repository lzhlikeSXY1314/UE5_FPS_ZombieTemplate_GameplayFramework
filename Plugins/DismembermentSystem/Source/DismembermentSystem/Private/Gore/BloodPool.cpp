// © 2021, Brock Marsh. All rights reserved.


#include "Gore/BloodPool.h"

#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"

// Sets default values
ABloodPool::ABloodPool()  //执行
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);

	Decal = CreateDefaultSubobject<UDecalComponent>("Decal");
	Decal->SetupAttachment(Root);
	Decal->DecalSize = FVector(100);
	Decal->SetRelativeRotation(FRotator(-90, 0, 0));
	Decal->SetFadeIn(-0.2f, 0.3f);

	if (!DecalMaterial) DecalMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/DismembermentSystem/BloodSystem/Materials/M_BaseDecal_Inst.M_BaseDecal_Inst"));

	Collision = CreateDefaultSubobject<UBoxComponent>("Collision");
	Collision->SetupAttachment(Root);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Collision->InitBoxExtent(FVector(100));

	Collision->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Overlap);
}

void ABloodPool::BeginPlay()
{
	Super::BeginPlay();

	if (!DecalMaterial) DecalMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/DismembermentSystem/BloodSystem/Materials/M_BaseDecal_Inst.M_BaseDecal_Inst"));

	Decal->SetFadeIn(StartDelay, 0.3f);
	Decal->SetRelativeScale3D(RemapSizeForDecal(DecalSize));
	Decal->SetDecalMaterial(DecalMaterial);
	Decal->SetRelativeRotation(DecalRotation);
	Decal->AddRelativeRotation(FRotator(-90, FMath::FRandRange(-10.f, 10.f), 0));

	Decal->SetFadeOut(MaxLifetime / 5, MaxLifetime, false);

	Collision->SetRelativeRotation(DecalRotation);
	Collision->SetRelativeScale3D(DecalSize);
}

FVector ABloodPool::RemapSizeForDecal(const FVector In) const
{
	FVector Out;
	Out.X = In.Z;
	Out.Y = In.Y;
	Out.Z = In.X;

	return Out;
}