// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSystem/ZombieSaveGame.h"

void UZombieSaveGame::Clear()
{
    PlayerLocation = FVector::ZeroVector;
    PlayerRotation = FRotator::ZeroRotator;
    PlayerHealth = 100.0f;
    LevelName.Empty();
    Weapons.Empty();
    SaveTimestamp = FDateTime::Now();
    SaveDisplayName.Empty();
}
