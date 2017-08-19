// Fill out your copyright notice in the Description page of Project Settings.

#include "WaveworksTester.h"
#include "FloatingSphere.h"
//#include ""

// Sets default values for this component's properties
UFloatingSphere::UFloatingSphere()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UFloatingSphere::BeginPlay()
{
	Super::BeginPlay();

	// ...
	UActorComponent* component = WaveWorksActor->GetComponentByClass(UWaveWorksComponent::StaticClass());
	WaveWorksComponent = Cast<UWaveWorksComponent>(component);
	InitialPosition = GetOwner()->GetActorLocation();

	WaveWorksRecieveDisplacementDelegate = FVectorArrayDelegate::CreateUObject(this, &UFloatingSphere::OnRecievedWaveWorksDisplacement);
}

DECLARE_DELEGATE_OneParam(FStringDelegate, FString);
DECLARE_DELEGATE_OneParam(FArrayDelegate, TArray<FVector4>);

// Called every frame
void UFloatingSphere::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	TArray<FVector2D> samplePoints;
	FVector2D samplePos(InitialPosition.X / 100.0f, InitialPosition.Y / 100.0f);
	samplePoints.Add(samplePos);
	
	WaveWorksComponent->SampleDisplacements(samplePoints, WaveWorksRecieveDisplacementDelegate);
	
	FVector newActorPosition;
	newActorPosition.X = WaveWorksOutDisplacement.X * 100.0f + InitialPosition.X;
	newActorPosition.Y = WaveWorksOutDisplacement.Y * 100.0f + InitialPosition.Y;
	newActorPosition.Z = WaveWorksOutDisplacement.Z * 100.0f + WaveWorksComponent->SeaLevel;
	GetOwner()->SetActorLocation(newActorPosition);
}

void UFloatingSphere::OnRecievedWaveWorksDisplacement(TArray<FVector4> OutDisplacements)
{
	if (OutDisplacements.Num() > 0)
	{
		WaveWorksOutDisplacement = OutDisplacements[0];
	}
}