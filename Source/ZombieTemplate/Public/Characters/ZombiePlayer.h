// ZombiePlayer.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <EnhancedInputLibrary.h>
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
//#include <ContextualAnimSceneActorComponent.h>
#include "ZombiePlayer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponFired);

class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class UInventoryHUDComponent;


UENUM(BlueprintType)
enum class EPlayerAnimState : uint8
{
    NoWeapon_AnimState    UMETA(DisplayName = "No Weapon Anim"),
    CG_Handgun_AnimState  UMETA(DisplayName = "CG Handgun Anim")
};

UCLASS()
class ZOMBIETEMPLATE_API AZombiePlayer : public ACharacter
{
    GENERATED_BODY()

public:
    AZombiePlayer();



    UPROPERTY(VisibleAnywhere, Category = Camera)
    class UCameraComponent* FPS_Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
    USkeletalMeshComponent* FPSSkeletalMesh;



    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    EPlayerAnimState CurrentAnimState = EPlayerAnimState::NoWeapon_AnimState;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
    class AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void PlayPickUpMontage();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    bool InitializeFinish = false;

#pragma region UI
public:

    UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
    FOnWeaponFired OnWeaponFired;

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    UUserWidget* CrosshairWidget;

private:

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> CrosshairWidgetClass;



#pragma endregion

#pragma region InputHandlers
public:
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* IA_Move;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* IA_Look;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* IA_Jump;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* IA_Interact;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* IA_DropWeapon;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* IA_Aim;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* IA_Fire;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* IA_Reload;



    UPROPERTY(BlueprintReadOnly, Category = "Aiming")
    bool bIsAiming = false;

    UPROPERTY(BlueprintReadOnly, Category = "Input")
    FVector2D LookInput;

    UPROPERTY(BlueprintReadOnly, Category = "Input")
    FVector2D MoveInput;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* IA_ToggleFlashlight;

    // 冲刺输入动作
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* IA_Sprint;

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void DropWeaponInInventory();

private:
    void Move(const FInputActionValue& Value);
    void MoveEnd();
    void Look(const FInputActionValue& Value);
    void LookEnd();
    void JumpStart();
    void JumpStop();
    void Interact();
    void DropWeapon();
    void StartAiming();
    void StopAiming();

    void ToggleFlashlight();
    void Reload();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void PerformFireTrace();

    UFUNCTION()
    bool CanAim(AActor* Target) const;

    void StartSprinting();
    void StopSprinting();
#pragma endregion

#pragma region Montage
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation | Montages")
    UAnimMontage* PickupWeaponMontage = nullptr;
#pragma endregion

#pragma region Movement
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Speed")
    float NormalWalkSpeed = 500.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Speed")
    float ADSWalkSpeed = 300.0f;

    // 冲刺速度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    float SprintSpeed = 700.0f;

    // 是否正在冲刺
    UPROPERTY(BlueprintReadOnly, Category = "Speed")
    bool bIsSprinting = false;
#pragma endregion

#pragma region Weapon
public:
    UFUNCTION(BlueprintCallable, Category = "Aiming")
    bool IsAiming() const { return bIsAiming; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawShootTrace = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aiming")
    UAudioComponent* ADSAudioComponent;

    UPROPERTY(EditAnywhere, Category = "Sound")
    USoundBase* OnADSSound;

    UPROPERTY(EditAnywhere, Category = "Sound")
    USoundBase* OffADSSound;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aiming")
    UTimelineComponent* FOVTimeline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
    UCurveFloat* FOVCurve;

    FOnTimelineFloat FOVTimelineCallback;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|Weapon")
    TArray<AWeaponBase*> AllOwnerWeapons;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Weapon")
    void RemoveWeaponRefByName(FString WeaponName);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Weapon")
    AWeaponBase* FindWeaponRefByName(FString WeaponName);

    UFUNCTION()
    void UpdateFOV(float Value);

    void ClearCurrentWeapon();

    void EquipWeaponDirect(AWeaponBase* Weapon);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Weapon")
    void RequestToggleWeapon(const FString& WeaponName, bool IsEquipped = false);

private:
    void PlayADSSound(bool bStart);
    void SetAimingState(bool bAiming);
    void UpdateArmsFromFOV(float FOVValue);
    float CalculateDamage(float Distance, FName BoneName) const;


    FVector DefaultArmsRelativeLocation = FVector::ZeroVector;
    FVector TargetArmsLocation;
    float NormalFOV = 70.0f;
    float ADSFOV = 60.0f;

    bool bPendingIsEquipped = false;

    UPROPERTY()
    class AWeaponBase* CurrentWeapon = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    UAnimMontage* EquipMontage;

    UPROPERTY()
    FString PendingToggleWeaponName;

    void PerformToggleWeapon(const FString& WeaponName);

#pragma endregion

#pragma region Ammo
public:

    UFUNCTION()
    void ConsumeAmmoFromInventory();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon|Ammo")
    int32 GetWeaponAmmoFromInventory();

    UFUNCTION()
    void UpdateAllWeaponAmmoWidget();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ammo")
    TMap<FString, FString> FindWeaponAmmo;

#pragma endregion

#pragma region Health
    public:
        DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);

        /** 最大生命值 */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Health")
        float MaxHealth = 100.0f;

        /** 当前生命值 */
        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Health")
        float CurrentHealth = 100.0f;

        /** 是否显示调试血量 */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Health")
        bool bDebugShowHealth = false;

        /** 玩家死亡时触发的委托 */
        UPROPERTY(BlueprintAssignable, Category = "Player|Health")
        FOnPlayerDeath OnPlayerDeath;

        /** 重写 TakeDamage 处理伤害 */
        virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;
#pragma endregion

#pragma region CameraShake
public:
    /** 走路时镜头晃动的摄像机动画类 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraShake")
    TSubclassOf<UCameraShakeBase> WalkCameraShakeClass;

    /** 跑步时镜头晃动的摄像机动画类 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraShake")
    TSubclassOf<UCameraShakeBase> RunCameraShakeClass;

    /** 开火时镜头晃动的摄像机动画类 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraShake")
    TSubclassOf<UCameraShakeBase> FireCameraShakeClass;

    /** 跳跃时镜头晃动的摄像机动画类 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraShake")
    TSubclassOf<UCameraShakeBase> JumpCameraShakeClass;

    /** 走路晃动强度 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraShake")
    float WalkShakeScale = 8.0f;

    /** 跑步晃动强度 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraShake")
    float RunShakeScale = 10.0f;

    /** 开火晃动强度 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraShake")
    float FireShakeScale = 0.15f;

    /** 跳跃晃动强度 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraShake")
    float JumpShakeScale = 0.05f;

    /** 触发移动晃动的最小速度 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraShake")
    float ShakeMinSpeed = 10.0f;


private:
    TWeakObjectPtr<UCameraShakeBase> ActiveCameraShakeInstance;
    int32 CurrentShakeState = 0;   // 0=无, 1=走路, 2=跑步
    void UpdateCameraShake();
    void PlayCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale);
    void StopCurrentCameraShake();

    
#pragma endregion

#pragma region Inventory
public:
    

private:

    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    UInventoryHUDComponent* InventoryComponent;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* IA_OpenInventory;

    void OpenInventory();
#pragma endregion

};
