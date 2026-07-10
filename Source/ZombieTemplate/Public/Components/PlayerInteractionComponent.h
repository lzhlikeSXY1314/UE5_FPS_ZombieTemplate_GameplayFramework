// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInteractionComponent.generated.h"

class IInteractable; 

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZOMBIETEMPLATE_API UPlayerInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerInteractionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 检查目标物体是否被摄像机看见（无遮挡）
	bool IsVisibleToCamera(AActor* Target);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionRadius = 400;

	// 可拾取的最大距离（厘米）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float PickupRange = 200.0f;

	// 视野角度（全角），只有在这个锥形范围内的物体才会被考虑
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float InteractionFieldOfView = 60.0f;

	// 当前最佳交互目标（屏幕中心最近且可见的物体）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	AActor* CurrentBestTarget;

	// 用于记录当前正在显示 Widget 的 Actor
	UPROPERTY()
	TSet<AActor*> VisibleActors;

	UPROPERTY()
	AActor* PreviousBestTarget = nullptr;

	void ProcessInteraction();

	/** 获取当前最佳交互目标 */
	AActor* GetCurrentBestTarget() const { return PreviousBestTarget; }
	void ClearBestTarget() { PreviousBestTarget = nullptr; }

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebug = true;
private:
	// 存储候选物体（Actor），使用时再检查接口，避免存储裸接口指针
	TArray<AActor*> Candidates;


};
