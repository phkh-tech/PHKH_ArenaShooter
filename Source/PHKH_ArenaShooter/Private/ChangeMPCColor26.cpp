// Fill out your copyright notice in the Description page of Project Settings.


#include "ChangeMPCColor26.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"


// Sets default values for this component's properties
UChangeMPCColor26::UChangeMPCColor26()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UChangeMPCColor26::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UChangeMPCColor26::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
 
	// ...
}

void UChangeMPCColor26::ChangeMaterialColor(FLinearColor NewColor)
{
	if (ChangeColors && !ParameterName.IsNone())
	{
		UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(), 
			ChangeColors, ParameterName, NewColor);
	}
}

