#include "MyBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Engine/World.h"

AMyBullet::AMyBullet()
{
	PrimaryActorTick.bCanEverTick = true;

	// Collision setup
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(1.0f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->SetNotifyRigidBodyCollision(true);
	RootComponent = CollisionComp;

	// Mesh setup
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Projectile movement setup
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 9000.f;
	ProjectileMovement->MaxSpeed = 9000.f;
	ProjectileMovement->Velocity = FVector(3000.0f, 0.0f, 0.0f);
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	// Bind hit event
	CollisionComp->OnComponentHit.AddDynamic(this, &AMyBullet::OnHit);
}

void AMyBullet::BeginPlay()
{
	Super::BeginPlay();
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		const FVector Forward = MyOwner->GetActorForwardVector();
		const FVector Right = MyOwner->GetActorRightVector();
		const FVector Up = MyOwner->GetActorUpVector();

		// === Equivalent to your Blueprint "Calculate Bullet Spawn Transform" ===
		const FVector NewLocation =
			MyOwner->GetActorLocation() +
			Forward * SpawnForwardOffset +
			Right * SpawnRightOffset +
			Up * SpawnUpOffset;

		SetActorLocationAndRotation(NewLocation, MyOwner->GetActorRotation());

		// Ignore collision with the owner
		CollisionComp->IgnoreActorWhenMoving(MyOwner, true);
	}
}

void AMyBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

static TMap<TWeakObjectPtr<UPrimitiveComponent>, TArray<TOptional<FLinearColor>>> GOriginalColors;

void AMyBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this && OtherActor != OwnerActor && OtherComp)
	{
		USkeletalMeshComponent* SkeletalMesh = OtherActor->FindComponentByClass<USkeletalMeshComponent>();
		UStaticMeshComponent* StaticMesh = OtherActor->FindComponentByClass<UStaticMeshComponent>();

		// Character or skeletal-based actor (red → white)
		if (SkeletalMesh)
		{
			UMeshComponent* TargetMesh = Cast<UMeshComponent>(SkeletalMesh);
			int32 MatCount = TargetMesh->GetNumMaterials();

			for (int32 i = 0; i < MatCount; ++i)
			{
				UMaterialInstanceDynamic* DynMat = TargetMesh->CreateAndSetMaterialInstanceDynamic(i);
				if (DynMat)
				{
					DynMat->SetVectorParameterValue(FName("Paint Tint"), FLinearColor::Red);

					FTimerHandle LocalTimerHandle;
					GetWorld()->GetTimerManager().SetTimer(
						LocalTimerHandle,
						[this, DynMat]()
						{
							DynMat->SetVectorParameterValue(FName("Paint Tint"), FLinearColor::White);
						},
						0.2f, false);
				}
			}
		}

		// Static-mesh-based actor (blue → original)
  else if (StaticMesh)
        {
            UStaticMeshComponent* WallMesh = Cast<UStaticMeshComponent>(StaticMesh);
            if (!WallMesh) return;

            int32 MatCount = WallMesh->GetNumMaterials();
            if (MatCount <= 0) return;

            // Prepare (ensure an array exists in the global map for this component)
            TWeakObjectPtr<UPrimitiveComponent> WeakComp(WallMesh);
            TArray<TOptional<FLinearColor>>& ColorArray = GOriginalColors.FindOrAdd(WeakComp);
            if (ColorArray.Num() < MatCount)
                ColorArray.SetNum(MatCount); // fills with unset optionals

            for (int32 i = 0; i < MatCount; ++i)
            {
                // Try to get existing material instance on the component
                UMaterialInterface* CurrentMatInterface = WallMesh->GetMaterial(i);
                UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(CurrentMatInterface);

                // If it's not a dynamic instance yet, create one and set it on the component.
                if (!DynMat)
                {
                    DynMat = WallMesh->CreateAndSetMaterialInstanceDynamic(i);
                }

                if (!DynMat)
                {
                    // failed to get/create dynamic material
                    continue;
                }

                // Only store original the first time (so multiple bullets don't overwrite original)
                if (!ColorArray[i].IsSet())
                {
                    // Try a few common parameter names to read the original color.
                    FLinearColor OutColor;
                    bool bGot = false;

                    static const FName CandidateParams[] = { FName("Base Color")};
                    for (const FName& Param : CandidateParams)
                    {
                        if (DynMat->GetVectorParameterValue(Param, OutColor))
                        {
                            bGot = true;
                            // store param name? (we'll try same order on revert)
                            break;
                        }
                    }

                    if (!bGot)
                    {
                        // fallback default if parameter not found
                        OutColor = FLinearColor::White;
                    }

                    ColorArray[i] = OutColor;
                }

                // Now set the "hit" color (try the candidate names in same order)
                {
                    bool bSet = false;
                    static const FName CandidateParams[] = { FName("Base Color")};
                    for (const FName& Param : CandidateParams)
                    {
                        // We attempt to set regardless; if param doesn't exist it will silently no-op,
                        // but at least we try multiple common names.
                        DynMat->SetVectorParameterValue(Param, FLinearColor::Blue);
                        bSet = true; // assume success — can't reliably detect except via GetVectorParameterValue
                    }
                }

                // Schedule revert using the *same* dynamic instance and look up original color from map
                FTimerDelegate RevertDelegate;
                RevertDelegate.BindLambda([WeakComp, i]()
                {
                    // Safely check component
                    UPrimitiveComponent* Comp = WeakComp.Get();
                    if (!Comp) // component destroyed
                    {
                        // cleanup map entry if any
                        GOriginalColors.Remove(WeakComp);
                        return;
                    }

                    // Find stored original color
                    TArray<TOptional<FLinearColor>>* FoundArr = GOriginalColors.Find(WeakComp);
                    if (!FoundArr || FoundArr->Num() <= i || !(*FoundArr)[i].IsSet())
                    {
                        // nothing to revert to
                        return;
                    }

                    FLinearColor Original = (*FoundArr)[i].GetValue();

                    // Get the dynamic material currently set at this index
                    UMaterialInstanceDynamic* CurrentDyn = Cast<UMaterialInstanceDynamic>(Comp->GetMaterial(i));
                    if (CurrentDyn)
                    {
                        // Try the same candidate parameter names to revert
                        static const FName CandidateParams[] = { FName("Base Color")};
                        for (const FName& Param : CandidateParams)
                        {
                            CurrentDyn->SetVectorParameterValue(Param, Original);
                        }
                    }

                    // Remove stored original if we can (cleanup). If the array becomes all unset, remove the key.
                    bool bAnyLeft = false;
                    for (int32 k = 0; k < FoundArr->Num(); ++k)
                    {
                        if ((*FoundArr)[k].IsSet())
                        {
                            // clear this index
                            (*FoundArr)[k].Reset();
                        }
                    }

                    // After resetting all, remove the whole entry to avoid growth
                    GOriginalColors.Remove(WeakComp);
                });

                FTimerHandle LocalTimerHandle;
                if (GetWorld())
                {
                    GetWorld()->GetTimerManager().SetTimer(LocalTimerHandle, RevertDelegate, 0.2f, false);
                }
            } 
        }
    } 

	// Destroy bullet after short delay
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AMyBullet::SelfDestroy, 0.2f, false);
	}
}
void AMyBullet::SelfDestroy()
{
	Destroy();
}
