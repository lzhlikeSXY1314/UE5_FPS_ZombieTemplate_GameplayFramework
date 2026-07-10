// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

class ZOMBIETEMPLATE_API IInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	FText GetItemName() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction|UI")
	void ShowWidget();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction|UI")
	void HideWidget();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction|UI")
	void ShowButtonWidget();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction|UI")
	void HideButtonWidget();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void OnInteract(AActor* Interactor);
	virtual void OnInteract_Implementation(AActor* Interactor) = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	int32 GetAmmoAmount() const;
	virtual int32 GetAmmoAmount_Implementation() const { return 0; }

};
