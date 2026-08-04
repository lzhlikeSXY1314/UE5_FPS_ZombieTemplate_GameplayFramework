// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/Actors/InspectItem.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/LightComponent.h"
#include "Components/RectLightComponent.h"
#include "InventorySystem/Functions/InventoryStaticFunctions.h"
#include <Kismet/KismetMathLibrary.h>

// Sets default values
AInspectItem::AInspectItem()
{
	PrimaryActorTick.bCanEverTick = true;

    SceneCaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent2D"));
    RootComponent = SceneCaptureComponent2D;
    SceneCaptureComponent2D->FOVAngle = 90.0f;

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = BaseLength;
    SpringArm->bUsePawnControlRotation = false;

    RectLight = CreateDefaultSubobject<URectLightComponent>(TEXT("RectLight"));
    RectLight->SetupAttachment(SpringArm);
    RectLight->Intensity = 7000.0f;

    Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
    Scene->SetupAttachment(SpringArm);

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMesh->SetupAttachment(Scene);
}

void AInspectItem::BeginPlay()
{
    Super::BeginPlay();

    PrimaryActorTick.bTickEvenWhenPaused = true;

    if (UInventoryStaticFunctions::GetInventoryOptions(this))
    {
        ItemMovementSpeed = UInventoryStaticFunctions::GetInventoryOptions(this)->InspectItemConfig.ItemMovementSpeed;
        ItemRotateSpeed = UInventoryStaticFunctions::GetInventoryOptions(this)->InspectItemConfig.ItemRotateSpeed;
        ItemZoomSpeed = UInventoryStaticFunctions::GetInventoryOptions(this)->InspectItemConfig.ItemZoomSpeed;
        CanRotate = UInventoryStaticFunctions::GetInventoryOptions(this)->InspectItemConfig.CanRotate;
        CanMove = UInventoryStaticFunctions::GetInventoryOptions(this)->InspectItemConfig.CanMove;
        CanZoom = UInventoryStaticFunctions::GetInventoryOptions(this)->InspectItemConfig.CanZoom;
    }

}

void AInspectItem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    DeltaSeconds = DeltaTime;

    if (!InspectInitialized) return;
    if (Seconds < 1.0)
    {
        Seconds += 5.0f * DeltaTime;
        double LerpValue = FMath::Lerp(-300.0, 0.0, Seconds);
        double ClampedZ = FMath::Clamp(LerpValue, -300.0, 0.0);
        FVector NewLoc = Scene->GetRelativeLocation();
        FRotator CurRot = Scene->GetRelativeRotation();
        NewLoc.Z = ClampedZ;
        Scene->SetRelativeLocationAndRotation(NewLoc, CurRot);
    }
    else
    {
        if (!bDoOnceFlag)
        {
            bDoOnceFlag = true;
            Scene->SetRelativeLocationAndRotation(FVector(0.0f), Scene->GetRelativeRotation());
        }
        else
        {
            RotateCurrent = FMath::Vector2DInterpTo(RotateCurrent, RotateAxis, DeltaTime, 50.0f);
            RotateInspectItem(RotateCurrent);

            DirectionCurrent = FMath::Vector2DInterpTo(DirectionCurrent, DirectionAxis, DeltaTime, 50.0f);
            MoveInspectItem(DirectionCurrent);

            RotateInspectItemLeftRight(RotateLeftRight * DeltaTime * 100.0f);
        }
    }

}

void AInspectItem::ResetTransform()
{
    SpringArm->TargetArmLength = BaseLength;
    SpringArm->SocketOffset = FVector::ZeroVector;
    Scene->SetRelativeRotation(ItemExamineSettings.ExamineStartRotation);
    Scene->SetRelativeLocation(FVector::ZeroVector);
}

void AInspectItem::ZoomItem(float InValue)
{
    if (!CanZoom) return;
    float TargetLength = FMath::Clamp(SpringArm->TargetArmLength + (InValue * ItemZoomSpeed * 4.0f), ZoomRange.X, ZoomRange.Y);
    SpringArm->TargetArmLength = TargetLength;
}

void AInspectItem::RotateInspectItem(FVector2D InVector)
{
    if (!CanRotate) return;

    const float Speed = ItemRotateSpeed;
    FRotator RotYaw = UKismetMathLibrary::RotatorFromAxisAndAngle(
        UKismetMathLibrary::GetRightVector(SceneCaptureComponent2D->GetComponentRotation()),
        InVector.Y * Speed * -0.25f
    );
    FRotator RotPitch = UKismetMathLibrary::RotatorFromAxisAndAngle(
        UKismetMathLibrary::GetUpVector(SceneCaptureComponent2D->GetComponentRotation()),
        InVector.X * Speed * -0.4f
    );

    FRotator NewRot = UKismetMathLibrary::ComposeRotators(Scene->GetComponentRotation(), RotYaw);
    NewRot = UKismetMathLibrary::ComposeRotators(NewRot, RotPitch);
    Scene->SetWorldRotation(NewRot);

}

void AInspectItem::MoveInspectItem(FVector2D InVector)
{
    if (!CanMove) return;
    const float MoveStep = ItemMovementSpeed * DeltaSeconds * 60.f;
    float TempY = FMath::Clamp(SpringArm->SocketOffset.Y + InVector.X * MoveStep, MoveLeftRightRange.X, MoveLeftRightRange.Y);
    float TempZ = FMath::Clamp(SpringArm->SocketOffset.Z + InVector.Y * MoveStep, MoveUpDownRange.X, MoveUpDownRange.Y);
    SpringArm->SocketOffset = FVector(0, TempY, TempZ);
}

void AInspectItem::RotateInspectItemLeftRight(float InLeftRightValue)
{
    if (!CanRotate) return;
    float Temp = InLeftRightValue * ItemRotateSpeed * 0.1f * -1.0f;
    FRotator RotA = Scene->GetComponentRotation();
    FRotator CaptureRot = SceneCaptureComponent2D->GetComponentRotation();
    FVector Axis = CaptureRot.Vector();
    FRotator RotB = UKismetMathLibrary::RotatorFromAxisAndAngle(Axis, Temp);

    FQuat Q_A = RotA.Quaternion();
    FQuat Q_B = RotB.Quaternion();
    FRotator NewRotation = (Q_B * Q_A).Rotator();

    Scene->SetWorldRotation(NewRotation);

}

void AInspectItem::InitItemExamine(FItemExamineSettings InItemExamineSettings)
{
    ItemExamineSettings = InItemExamineSettings;
    if (!ItemExamineSettings.ExamineMesh.IsValid()) return;
    ResetTransform();
    StaticMesh->SetStaticMesh(ItemExamineSettings.ExamineMesh.Get());
    StaticMesh->SetRelativeScale3D(FVector(ItemExamineSettings.ExamineMeshSize));
    StaticMesh->SetRelativeLocation(FVector(ItemExamineSettings.PivotPointOffset));
    InspectInitialized = true;
}




