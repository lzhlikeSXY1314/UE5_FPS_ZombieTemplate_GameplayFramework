// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/Functions/InventoryStaticFunctions.h"
#include"kismet/KismetMathLibrary.h"

#include <ZombieGameInstance.h>
#include "InventorySystem/Components/InventoryHUDComponent.h"
#include "GameFramework/Character.h"
#include "Characters/ZombiePlayer.h"

UInventoryData* UInventoryStaticFunctions::GetInventoryOptions(UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	UGameInstance* GI = WorldContextObject->GetWorld()->GetGameInstance();
	UZombieGameInstance* ZombieGI = Cast<UZombieGameInstance>(GI);
	return ZombieGI ? ZombieGI->InventoryOptions : nullptr;
}

USoundBase* UInventoryStaticFunctions::GetInventorySound(UObject* WorldContextObject, E_InventorySoundType SoundType)
{
	UInventoryData* InvData = GetInventoryOptions(WorldContextObject);
	if (!InvData) return nullptr;
	USoundBase** SoundPtr = InvData->InventorySounds.Find(SoundType);
	return (SoundPtr && *SoundPtr) ? *SoundPtr : nullptr;
}

UInventoryHUDComponent* UInventoryStaticFunctions::GetInventoryHUDComponent(UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	APlayerController* PC = WorldContextObject->GetWorld()->GetFirstPlayerController();
	if (!PC) return nullptr;
	ACharacter* PlayerChar = Cast<ACharacter>(PC->GetPawn());
	if (!PlayerChar) return nullptr;
	return PlayerChar->FindComponentByClass<UInventoryHUDComponent>();
}



int32 UInventoryStaticFunctions::GetColumn(const int32 ArrayIndex, const int32 Columns)
{
	double Reminder;
	UKismetMathLibrary::FMod(ArrayIndex, Columns, Reminder);
	return UKismetMathLibrary::FTrunc(Reminder);
}

int32 UInventoryStaticFunctions::GetRow(const int32 ArrayIndex, const int32 Columns)
{
	if (Columns <= 0) return -1;
	const double TempNum = ArrayIndex / Columns;
	return UKismetMathLibrary::FTrunc(TempNum);
}

AZombiePlayer* UInventoryStaticFunctions::GetPlayerRef(UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return nullptr;

	return Cast<AZombiePlayer>(PC->GetPawn());
}
