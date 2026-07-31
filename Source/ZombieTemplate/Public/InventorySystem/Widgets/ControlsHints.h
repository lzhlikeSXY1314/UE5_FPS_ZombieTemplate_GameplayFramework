// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include"InventorySystem/Structs/InventoryTypes.h"
#include "ControlsHints.generated.h"


USTRUCT(BlueprintType)
struct FKeyHints
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Hint")
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Hint")
    TArray<TObjectPtr<UTexture2D>> ImagesPC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Hint")
    FVector2D Size = FVector2D(1.0f, 1.0f);

    FKeyHints()
        : Name(FText::GetEmpty())
        , Size(FVector2D(1.0f, 1.0f))
    {
    }
};

USTRUCT(BlueprintType)
struct FKeyHintsArray
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Hints")
    TArray<FKeyHints> Array;
};


UCLASS()
class ZOMBIETEMPLATE_API UControlsHints : public UUserWidget
{
	GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Hints")
    EInventoryStatus Status = EInventoryStatus::Opened;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Key Hints|Function")
    void UpdateControlHint(EInventoryStatus InStatus);
    virtual void UpdateControlHint_Implementation(EInventoryStatus InStatus);


};
