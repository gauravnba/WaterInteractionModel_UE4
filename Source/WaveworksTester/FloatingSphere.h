// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "FloatingSphere.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WAVEWORKSTESTER_API UFloatingSphere : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFloatingSphere();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void OnRecievedWaveWorksDisplacement(TArray<FVector4> OutDisplacements);
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WaveWorks)
	AActor* WaveWorksActor;
private:
	class UWaveWorksComponent* WaveWorksComponent;
	FVector4 WaveWorksOutDisplacement;
	FVector InitialPosition;
	FVectorArrayDelegate WaveWorksRecieveDisplacementDelegate;
};
