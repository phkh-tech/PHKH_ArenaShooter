#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/EngineTypes.h" // for FHitResult
#include "TimerManager.h"
#include "MyBullet.generated.h"

UCLASS()
class AMyBullet : public AActor // removed module API macro to avoid mismatched macro errors
{
	GENERATED_BODY()

public:
	AMyBullet();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void SelfDestroy();

	FTimerHandle DestroyTimerHandle;
	FTimerHandle DelayTimerHandle;

public:
	virtual void Tick(float DeltaTime) override;

	// === Components ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovement;

	// === Owner & Spawn Logic ===
	UPROPERTY(BlueprintReadWrite, Category = "Bullet")
	AActor* OwnerActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	float SpawnForwardOffset = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	float SpawnUpOffset = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	float SpawnRightOffset = 5.f;
	
};
