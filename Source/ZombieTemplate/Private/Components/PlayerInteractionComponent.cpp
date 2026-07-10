// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PlayerInteractionComponent.h"
#include "Engine/OverlapResult.h"
#include "Interface/Interactable.h"


// Sets default values for this component's properties
UPlayerInteractionComponent::UPlayerInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayerInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UPlayerInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    static float TimeSinceLastUpdate = 0.0f;
    TimeSinceLastUpdate += DeltaTime;
    if (TimeSinceLastUpdate > 0.2f)   // 每0.2秒刷新一次
    {
        TimeSinceLastUpdate = 0.0f;
        ProcessInteraction();
    }
}

bool UPlayerInteractionComponent::IsVisibleToCamera(AActor* Target)
{
    if (!Target) return false;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return false;

    // 获取摄像机位置和朝向
    FVector CameraLoc;
    FRotator CameraRot;
    PC->GetPlayerViewPoint(CameraLoc, CameraRot);
    FVector CameraForward = CameraRot.Vector();

    // 1. 计算物体中心到摄像机的方向
    FVector ToTarget = Target->GetActorLocation() - CameraLoc;
    float Distance = ToTarget.Size();
    FVector DirToTarget = ToTarget / Distance;   // 归一化

    // 2. 视野角度检查：计算方向点积，并与半角余弦比较
    float Dot = FVector::DotProduct(CameraForward, DirToTarget);
    float HalfFOVRad = FMath::DegreesToRadians(InteractionFieldOfView * 0.5f);
    float CosHalfFOV = FMath::Cos(HalfFOVRad);
    if (Dot < CosHalfFOV)
    {
        // 不在视野角度内，直接判定不可见
        return false;
    }

    // 获取目标物体包围盒
    FBoxSphereBounds Bounds = Target->GetComponentsBoundingBox();
    FVector Center = Bounds.Origin;
    FVector Extent = Bounds.BoxExtent;

    // 生成包围盒的 8 个角点
    TArray<FVector> Corners;
    for (int32 x = -1; x <= 1; x += 2)
        for (int32 y = -1; y <= 1; y += 2)
            for (int32 z = -1; z <= 1; z += 2)
                Corners.Add(Center + FVector(x * Extent.X, y * Extent.Y, z * Extent.Z));

    // 按到摄像机的距离排序，取最近的 5 个点（即物体正面的大致区域）
    Corners.Sort([CameraLoc](const FVector& A, const FVector& B) {
        return FVector::DistSquared(A, CameraLoc) < FVector::DistSquared(B, CameraLoc);
        });

    TArray<FVector> SamplePoints;
    for (int32 i = 0; i < FMath::Min(5, Corners.Num()); ++i)
    {
        SamplePoints.Add(Corners[i]);
    }

    // 设置射线参数，忽略玩家和目标自身
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Target);

    // 对每个采样点进行射线检测
    for (const FVector& Point : SamplePoints)
    {

        FHitResult Hit;
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            Hit,
            CameraLoc,
            Point,
            ECC_Visibility,
            QueryParams
        );
        // 如果射线没有击中任何物体，或者击中的就是目标自身，则认为该采样点可见
        if (!bHit || Hit.GetActor() == Target)
        {
            return true;   // 只要有一个点通畅，就认为物体可见
        }
    }
    // 所有采样点都被遮挡，物体不可见
    return false;
}

void UPlayerInteractionComponent::ProcessInteraction()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;

    // ------- 球形检测，收集所有实现了接口的 Actor -------
    FVector PawnLocation = PC->GetPawn()->GetActorLocation();
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractionRadius);
    GetWorld()->OverlapMultiByChannel(Overlaps, PawnLocation, FQuat::Identity, ECC_Visibility, Sphere);

    // 本次检测中发现“可见”的 Actor
    TSet<AActor*> NewVisibleActors;

    for (auto& Overlap : Overlaps)
    {
        AActor* Actor = Overlap.GetActor();
        if (Actor && Actor->Implements<UInteractable>())
        {
            if (IsVisibleToCamera(Actor)) // 包含角度、遮挡判断
            {
                NewVisibleActors.Add(Actor);
            }
        }
    }

    // ------- 处理 Widget 的显示/隐藏（只在新出现/消失时触发） -------
    // 清理无效引用
    VisibleActors.Remove(nullptr);

    // 对每个新出现的 Actor 显示 Widget
    for (AActor* Actor : NewVisibleActors)
    {
        if (!VisibleActors.Contains(Actor))
        {
            IInteractable::Execute_ShowWidget(Actor);
        }
    }

    // 对每个已消失的 Actor 隐藏 Widget
    for (AActor* Actor : VisibleActors)
    {
        if (!NewVisibleActors.Contains(Actor))
        {
            IInteractable::Execute_HideWidget(Actor);
        }
    }

    // 更新状态
    VisibleActors = MoveTemp(NewVisibleActors);

    // ------- 从可见物体中选出最佳目标（角度+距离） -------
    AActor* NewBest = nullptr;
    if (VisibleActors.Num() > 0)
    {
        FVector CameraLoc;
        FRotator CameraRot;
        PC->GetPlayerViewPoint(CameraLoc, CameraRot);
        FVector CameraForward = CameraRot.Vector();

        float BestDot = -2.0f;
        float BestDist = FLT_MAX;

        for (AActor* Candidate : VisibleActors)
        {
            FVector ToTarget = Candidate->GetActorLocation() - CameraLoc;
            float Dist = ToTarget.Size();
            if (Dist > PickupRange) continue; // 超出拾取范围

            FVector Dir = ToTarget / Dist;
            float Dot = FVector::DotProduct(CameraForward, Dir);

            if (Dot > BestDot || (FMath::IsNearlyEqual(Dot, BestDot) && Dist < BestDist))
            {
                BestDot = Dot;
                BestDist = Dist;
                NewBest = Candidate;
            }
        }
    }

    // ------- 处理最佳目标变化（Button 控制） -------
    if (NewBest != PreviousBestTarget)
    {
        // 旧目标隐藏 Button
        if (PreviousBestTarget && PreviousBestTarget->Implements<UInteractable>())
        {
            IInteractable::Execute_HideButtonWidget(PreviousBestTarget);
        }
        // 新目标显示 Button
        if (NewBest && NewBest->Implements<UInteractable>())
        {
            IInteractable::Execute_ShowButtonWidget(NewBest);
        }
        PreviousBestTarget = NewBest;
    }

    // ========== 调试绘制 ==========
    if (bDrawDebug)
    {
        // 绘制交互范围和拾取范围的球
        DrawDebugSphere(GetWorld(), PawnLocation, InteractionRadius, 12, FColor::Black, false, 0.2f);
        DrawDebugSphere(GetWorld(), PawnLocation, PickupRange, 12, FColor::Red, false, 0.2f);

        // 绘制所有可见物体的包围盒
        for (AActor* Actor : VisibleActors)
        {
            if (!Actor) continue;

            FBoxSphereBounds Bounds = Actor->GetComponentsBoundingBox();
            FVector Center = Bounds.Origin;
            FVector Extent = Bounds.BoxExtent;

            // 最佳目标红色，其他白色
            FColor BoxColor = (Actor == NewBest) ? FColor::Red : FColor::White;

            DrawDebugBox(GetWorld(), Center, Extent, BoxColor, false, 0.2f, 0, 2.0f);

            // 绘制 8 个角点
            for (int32 x = -1; x <= 1; x += 2)
                for (int32 y = -1; y <= 1; y += 2)
                    for (int32 z = -1; z <= 1; z += 2)
                        DrawDebugPoint(GetWorld(), Center + FVector(x * Extent.X, y * Extent.Y, z * Extent.Z), 10.0f, BoxColor, false, 0.2f);
        }
    }


}

