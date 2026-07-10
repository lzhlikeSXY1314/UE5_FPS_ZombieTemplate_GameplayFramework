// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DismembermentComponent.generated.h"




UENUM(BlueprintType)
enum class EDismemberColorChannel : uint8
{
	R_Channel UMETA(DisplayName = "R"),
	G_Channel UMETA(DisplayName = "G"),
	B_Channel UMETA(DisplayName = "B"),
	A_Channel UMETA(DisplayName = "A")
};


/*
 *  用于缓存断肢事件的数据。
 *  因为动画要到下一帧才会更新，
 *  所以为了让断肢能正确复制父物体的姿势，我们必须延迟一帧执行。
 */

USTRUCT()
struct FDismemberedLimbFrameDelay
{
	GENERATED_BODY()

	FDismemberedLimbFrameDelay() {};
	FDismemberedLimbFrameDelay(const FName InName, USkeletalMeshComponent* InMesh)
		: BoneName(InName), SkeletalMeshComponent(InMesh) {
	}

	UPROPERTY()
	FName BoneName = NAME_None;
	UPROPERTY()
	USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
	UPROPERTY()
	FVector Impulse = FVector(0);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPreDismemberment, FName, BoneName, FVector, Impulse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPostDismemberment, FName, BoneName, USkeletalMeshComponent*, DismemberedMesh);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent, DisplayName = "Dismemberment Component"))
class DISMEMBERMENTSYSTEM_API UDismembermentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDismembermentComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


private:
	UPROPERTY()
	class USkeletalMeshComponent* SkeletalMeshComponent;



public:

	USkeletalMesh* GetMesh() const;

	UFUNCTION(BlueprintCallable, Category = "Dismemberment")
	void SetSkeletalMeshComponentToDismember(class USkeletalMeshComponent* InSkeletalMeshComponent) { SkeletalMeshComponent = InSkeletalMeshComponent; };

	class USkeletalMeshComponent* GetSkeletalMeshComponent() const;

/*
	Override Skeletal Mesh Components
*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Source Mesh", meta = (ToolTip = "Optional setting to manually set the skeletal mesh(s) that should be used for dismemberment by the system. If false, the system will use the first Skeletal Mesh Component found on the Owning Actor"))
	bool bOverrideSourceMesh = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Source Mesh", meta = (EditCondition = "bOverrideSkeletalMesh", EditConditionHides = "true", GetOptions = "GetSkeletalMeshOptions", ToolTip = "Use this to manually set the skeletal mesh(s) that should be used for dismemberment by the system."))
	TArray<FString> SourceMeshComponents;

	UPROPERTY()
	TArray<class USkeletalMeshComponent*> DismembermentMeshComponents;


	/* All Meshes attached to the Main Mesh. */  //获取当前角色身上所有附加的骨骼网格体组件 子类网格体
	TArray<class USkeletalMeshComponent*> GetAllAttachedMeshes() const;
	TArray<class USkeletalMeshComponent*> GetAllAttachedMeshes(USkeletalMeshComponent* Component) const;
	TArray<class USkeletalMeshComponent*> GetAllAttachedMeshes(FName LimbName) const;

	/* 获取组成角色的所有骨骼网格体（自身网格 + 所有已附加的网格体）*/ //附加暂时没有实现
	TArray<class USkeletalMeshComponent*> GetAllMeshes() const;



/*
*	Config
*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Dismemberment")
	bool bDisableDismemberment = false; 

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dismemberment", meta = (EditCondition = "bOverrideSkeletalMesh == false", EditConditionHides))
	bool bSupportAttachedChildMeshes = false;  //bSupportAttachedChildMeshes 子类的骨骼一起肢解

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOverrideSkeletalMesh = false;

	/** 该组件的物理场景信息，包含一个带有多个碰撞形状的单一刚体。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Limb Collision", meta = (ShowOnlyInnerProperties, SkipUCSModifiedProperties, ToolTip = "The Collision settings that are used by the Limbs when dismembered."))
	FBodyInstance LimbCollisionSettings;  //肢解部位的碰撞信息；




/*
*	Limb Data
*/
	UPROPERTY(BlueprintReadOnly, Category="LimbMap")
	class ULimbMap* LimbMap;


	bool IsLimbMapOutdated() const;
	void UpdateLimbMap();

	ULimbMap* CreateLimbMap();

/*
 *  一张记录所有「已丢失/已切断骨头」和「为其生成的肢体组件」的映射表。
 *  肢体的所有子骨头都会被添加到这张表中，
 *  这样我们就不会尝试去切断那些**已经不存在**的肢体了。
 */
	UPROPERTY()
	TMap<FName, USkeletalMeshComponent*> MissingLimbs;
	void UpdateMissingLimbs(USkeletalMeshComponent* InDismemberedLimb, FName BoneName);


/*
*	Defaults
*/
	UPROPERTY(EditAnywhere,Category = "Dismemberment", meta = (GetOptions = "GetBoneNameOptions", ToolTip = "These are bone names that will never get dismembered. Useful for Root or Pelvis bones."))
	TArray<FName> ExcludedBones;		 // 黑名单 → 这些骨头**永远不能切**
	UPROPERTY(EditAnywhere, Category = "Dismemberment", meta = (ToolTip = "Opposite of Excluded Bones. If this is used then only bones contained in this list will be used."))
	TArray<FName> WhitelistBones;	    // 白名单 → **只有这些骨头能切**


	/* 如果骨骼名称通过了骨骼过滤器，则返回 true。 */
	bool CheckBoneFilter(FName BoneName) const;

	UPROPERTY(EditAnywhere, Category = "Dismemberment", meta = (ToolTip = "These are bone names that when dismembered, They will use the mapped name instead. Useful for pelvis bones."))
	TMap<FName, FName> RemappedBones;




	UPROPERTY(EditAnywhere, Category = "Dismemberment", meta = (ToolTip = "Set this to your Dismemberment Anim Instance. If Null, this will be set to the Ue4 Manniquin version that comes with the plugin"))
	TSubclassOf<class UDismemberedAnimInstance> DismemberedAnimInstance;

	int32 GetVertexNum(const USkeletalMeshComponent* InMesh, const int32 LOD = 0) const;


/*
*	Dismemberment
*/
	UPROPERTY()
	TArray<FDismemberedLimbFrameDelay> FrameDelayedDismemberedLimbs;

	UFUNCTION(BlueprintCallable, Category = "Dismemberment", meta = (ToolTip = "The Main function to call when Dismembering a Limb"))
	virtual void DismemberLimb(FName BoneName, FVector Impulse = FVector(0));

	UFUNCTION(BlueprintCallable, Category = "Dismemberment", meta = (ToolTip = "This will hide a limb on the Owning Mesh without spawning a dismembered limb with it."))
	virtual void DestroyLimb(const FName BoneName);

	/* 这是当一根骨骼被切断时，实际执行的核心代码 */
	UFUNCTION()
	virtual void DismemberLimb_Internal(FName BoneName, const FVector& Impulse);

	UFUNCTION()
	void BeginFrameDelayedDismemberment(USkeletalMeshComponent* Component, FName BoneName, FVector Impulse);
	UFUNCTION()
	void DismemberLimbFrameDelayed();

	USkeletalMeshComponent* CreateDismemberedLimb(const FName BoneName);

	void SetDismemberedAnimInstance(USkeletalMeshComponent* Component, FName BoneName) const;

	void RemovePreviouslyDismemberedLimbs(USkeletalMeshComponent* Component);

	void CreateDismemberPhysicsAsset(USkeletalMeshComponent* Component, FName InLimb) const;
	void TerminatePhysicsBodies(UPhysicsAsset* PhysicsAsset, int32 Index) const;


	/*
	*	Delegates
	*/
	UPROPERTY(BlueprintAssignable)
	FOnPreDismemberment OnPreDismemberment;
	virtual void PreDismemberment(const FName BoneName, FVector Impulse);
	UPROPERTY(BlueprintAssignable)
	FOnPostDismemberment OnPostDismemberment;
	virtual void PostDismemberment(const FName BoneName, USkeletalMeshComponent* DismemberMesh);


/*
*  肢体相关
*/
/* 用于在切断父肢体时，保留已被切断的子肢体 | 示例：先切断小臂，再切断大臂。 */

	UPROPERTY()
	TArray<FLinearColor> DismemberedVertices;

	FLinearColor GetCurrentVertexColor(const USkeletalMeshComponent* Mesh, const int32 VertexIndex, const int32 LOD = 0) const;

	TArray<FLinearColor> GetCurrentVertexColors(const USkeletalMeshComponent* Mesh, const int32 LODIndex = 0) const;

	void CopyVertexColorsToMesh(const USkeletalMeshComponent* FromMesh, USkeletalMeshComponent* ToMesh);




/*
*	Vertex Colors
*/

	static void SetColorOfChannel(FLinearColor& Color, float Value, EDismemberColorChannel Channel);
	static void MaxColorOfChannel(FLinearColor& Color, float Value, EDismemberColorChannel Channel);

	static void SetLinearColorChannel(TArray<FLinearColor>& Colors, float Value, EDismemberColorChannel Channel);
	static void MaxLinearColorChannel(TArray<FLinearColor>& Colors, TArray<float>& Values, const EDismemberColorChannel Channel);
};



