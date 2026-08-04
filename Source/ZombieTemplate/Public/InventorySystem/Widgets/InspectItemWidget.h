// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InspectItemWidget.generated.h"

class UImage;
class AInspectItem;

UCLASS()
class ZOMBIETEMPLATE_API UInspectItemWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent) override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "InspectItem|Data")
	AInspectItem* InspectItem = nullptr;
};
