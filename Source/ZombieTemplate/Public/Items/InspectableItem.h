// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/Interactable.h"
#include "Components/WidgetComponent.h"  
#include "SaveSystem/SaveableActor.h"
#include "InspectableItem.generated.h"

UCLASS()
class ZOMBIETEMPLATE_API AInspectableItem : public AActor, public IInteractable, public ISaveableActor
{
	GENERATED_BODY()
	
public:	
	//------存档接口------//
	virtual FName GetUniqueSaveID_Implementation() const override;
	virtual FActorSaveData GetSaveData_Implementation() const override;
	virtual void RestoreState_Implementation(const FActorSaveData& Data) override;
	virtual void ResetToDefault_Implementation() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FName SaveActorID;
protected:
	FTransform OriginalWorldTransform;


public:


	// Sets default values for this actor's properties
	AInspectableItem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* VisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	FVector MeshScale = FVector(1.0f);


//接口重写：
	virtual FText GetItemName_Implementation() const override { return ItemName; }
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual int32 GetAmmoAmount_Implementation() const override { return 0; }
//接口

	// 3D 控件组件（例如显示物品名字、交互提示）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* InteractionWidget;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;
};
