// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/Interactable.h"
#include "Components/WidgetComponent.h"  
#include "SaveSystem/SaveableActor.h"
#include "InventorySystem/Widgets/ItemWidget.h"
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



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	FVector MeshScale = FVector(1.0f);

	// 3D 控件组件（例如显示物品名字、交互提示）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* InteractionWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Widget Data")
	FInventoryItemPayload InventoryItemPayload;

	UFUNCTION(BlueprintCallable, Category = "Item Widget Data")
	FText GetItemInteractMes(bool bShowAmount = true) const
	{
		if (bShowAmount)
		{
			return FText::Format(
				NSLOCTEXT("ItemName", "ItemAmount", "{0} X {1}"),
				FText::FromString(InventoryItemPayload.ItemName),
				FText::AsNumber(InventoryItemPayload.ItemAmount)
			);
		}
		else
		{
			return FText::FromString(InventoryItemPayload.ItemName);
		}
	}


//接口重写：

	virtual void OnInteract_Implementation(AActor* Interactor) override;

//接口

	UFUNCTION(BlueprintNativeEvent, Category = "Inventory")
    void DiscardItemInInventory(int32 Quantity);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;
};
