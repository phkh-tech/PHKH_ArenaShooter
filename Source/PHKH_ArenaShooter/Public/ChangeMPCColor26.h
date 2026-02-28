// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChangeMPCColor26.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PHKH_ARENASHOOTER_API UChangeMPCColor26 : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UChangeMPCColor26();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MPC)
	FName ParameterName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MPC)
	UMaterialParameterCollection* ChangeColors;
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable, Category = MPC)
	void ChangeMaterialColor(FLinearColor NewColor);
};
