// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InspectItem.generated.h"


class USceneComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class USceneCaptureComponent2D;
class URectLightComponent;



USTRUCT(BlueprintType)
struct FItemExamineSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool CanExamine = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> ExamineMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExamineMeshSize = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator ExamineStartRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PivotPointOffset;
};

UCLASS()
class ZOMBIETEMPLATE_API AInspectItem : public AActor
{
	GENERATED_BODY()
	
protected:
    AInspectItem();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:	

    UPROPERTY(EditAnywhere, Category = "InspectItem")
    float BaseLength = 200.0f;

    UPROPERTY(EditAnywhere, Category = "InspectItem")
    FItemExamineSettings ItemExamineSettings;

    // 根场景组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* Scene;

    // 静态网格体组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StaticMesh;

    // 弹簧臂组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpringArmComponent* SpringArm;

    // 2D场景捕获组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneCaptureComponent2D* SceneCaptureComponent2D;

    // 矩形光源组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    URectLightComponent* RectLight;


    UPROPERTY(BlueprintReadWrite, Category = "InspectItemWidget")
    FVector2D DirectionAxis = FVector2D(0.0f);

    UPROPERTY(BlueprintReadWrite, Category = "InspectItemWidget")
    FVector2D RotateAxis = FVector2D(0.0f);

    UPROPERTY(BlueprintReadWrite, Category = "InspectItemWidget")
    float RotateLeftRight = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "InspectItemWidget")
    bool InspectInitialized = false;

    UPROPERTY(BlueprintReadWrite, Category = "InspectItemWidget")
    FVector2D MoveUpDownRange = FVector2D(-15.0f, 15.0f);

    UPROPERTY(BlueprintReadWrite, Category = "InspectItemWidget")
    FVector2D MoveLeftRightRange = FVector2D(-30.0f, 30.0f);

    UPROPERTY(BlueprintReadonly, Category = "InspectItemWidget")
    FVector2D RotateCurrent = FVector2D(0.0f);

    UPROPERTY(BlueprintReadonly, Category = "InspectItemWidget")
    FVector2D DirectionCurrent = FVector2D(0.0f);

    UPROPERTY(BlueprintReadWrite, Category = "InspectItemWidget")
    FVector2D ZoomRange = FVector2D(50.0f, 100.0f);


    UFUNCTION(BlueprintCallable, Category = "InspectItemWidget")
    void ResetTransform();

    UFUNCTION(BlueprintCallable, Category = "InspectItemWidget")
    void ZoomItem(float InValue);


    UFUNCTION()
    void RotateInspectItem(FVector2D InVector);

    UFUNCTION()
    void MoveInspectItem(FVector2D InVector);

    UFUNCTION()
    void RotateInspectItemLeftRight(float InLeftRightValue);

    UFUNCTION()
    void InitItemExamine(FItemExamineSettings InItemExamineSettings);

private:


    UPROPERTY()
    float Seconds = 0.0f;

    UPROPERTY()
    float DeltaSeconds = 0.0f;

    UPROPERTY()
    bool bDoOnceFlag = false;

    UPROPERTY()
    float ItemRotateSpeed = 1.0f;

    UPROPERTY()
    float ItemMovementSpeed = 1.0f;

    UPROPERTY()
    float ItemZoomSpeed = 1.0f;

    UPROPERTY()
    bool CanRotate = true;

    UPROPERTY()
    bool CanMove = true;

    UPROPERTY()
    bool CanZoom = true;
};
