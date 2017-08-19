// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Misc/ScopeLock.h"
#include "Components/ActorComponent.h"
#include "WaterPhysicsComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WAVEWORKSTESTER_API UWaterPhysicsComponent : public UActorComponent
{
#define DRAW_DEBUG
//#define DRAW_MESH_DEBUG
//#define DRAW_PROJECTION_DEBUG
#define DRAW_FORCES_DEBUG
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWaterPhysicsComponent();

	~UWaterPhysicsComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, Category = "Debug", DisplayName = "Log Data")
	bool mLogEnable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WaveWorks)
	AActor* WaveWorksActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WaterInteraction, Meta = (DisplayName = "BoatHull"))
	class UStaticMeshComponent* mMeshComponent;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	enum class Submersion
	{
		None,
		PartialSingleVertex,
		PartialTwoVertices,
		Full
	};

	struct Tri
	{
		int32 Index1;
		int32 Index2;
		int32 Index3;

		Submersion Submerged;
		float Area;

		Tri(int32 index1, int32 index2, int32 index3);
	};

	void CalculateWaterIntersection();

	void CalculateVertexLocations();

	void OnRecievedWaveWorksDisplacement(TArray<FVector4> OutDisplacements);

	void MarkSubmergedTriangles();

	void ApplyHydrostaticForce(const Tri& submergedTriangle, TArray<float>& heights);

	TArray<FVector> mVertices;
	TArray<float> mWaveworksDisplacements;
	TArray<Tri> mTriLocations;

	FVectorArrayDelegate mWaveworksDisplacementDelegate;
	class UWaveWorksComponent* mWaveWorksComponent;
	FCriticalSection* mWaveWorksDisplacementLock;

	float mSurfaceAreaOfBoat;
	float mLengthOfBoat;
	float mLengthOfSubmerged;
};