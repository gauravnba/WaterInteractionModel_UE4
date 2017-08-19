// Fill out your copyright notice in the Description page of Project Settings.

#include "WaveworksTester.h"
#include "WaterPhysicsComponent.h"
#include "Utility/BoatPhysicsUtil.h"

//PhysX includes
#include "PhysicsPublic.h"
#include "PhysXPublic.h"
#include "PhysXIncludes.h"
#include "ThirdParty/PhysX/PhysX_3.4/Include/geometry/PxTriangleMesh.h"
#include "ThirdParty/PhysX/PxShared/include/foundation/PxSimpleTypes.h"

#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodySetup.h"

UWaterPhysicsComponent::Tri::Tri(int32 index1, int32 index2, int32 index3) :
	Index1(index1), Index2(index2), Index3(index3), Submerged(Submersion::None), Area(0.0f)
{
}

// Sets default values for this component's properties
UWaterPhysicsComponent::UWaterPhysicsComponent() :
	mWaveWorksDisplacementLock(new FCriticalSection), mSurfaceAreaOfBoat(0.0f), mLengthOfBoat(0.0f), mLengthOfSubmerged(0.0f)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}

UWaterPhysicsComponent::~UWaterPhysicsComponent()
{
	delete mWaveWorksDisplacementLock;
}

// Called when the game starts
void UWaterPhysicsComponent::BeginPlay()
{
	Super::BeginPlay();

	UActorComponent* component = WaveWorksActor->GetComponentByClass(UWaveWorksComponent::StaticClass());
	mWaveWorksComponent = Cast<UWaveWorksComponent>(component);

	mWaveworksDisplacementDelegate = FVectorArrayDelegate::CreateUObject(this, &UWaterPhysicsComponent::OnRecievedWaveWorksDisplacement);

	// Get array of triangles and the number of triangles in the mesh.
	PxTriangleMesh* triMesh = mMeshComponent->GetBodySetup()->TriMeshes[0];

	const void* triangles = triMesh->getTriangles();
	const int32 numTris = triMesh->getNbTriangles();

	// Create an array of all triangles present on the mesh.
	for (int32 triIndex = 0; triIndex < numTris; ++triIndex)
	{
		const PxU16* indices = static_cast<const PxU16*>(triangles);

		int32 index1 = indices[(triIndex * 3) + 0];
		int32 index2 = indices[(triIndex * 3) + 1];
		int32 index3 = indices[(triIndex * 3) + 2];
		mTriLocations.Add(Tri(index1, index2, index3));
	}

	// Calculate total surface area of mesh.
	CalculateVertexLocations();

	if (mVertices.Num() > 0)
	{
		for (auto& tri : mTriLocations)
		{
			tri.Area = BoatPhysicsUtil::TriangleArea(mVertices[tri.Index1], mVertices[tri.Index2], mVertices[tri.Index3]) * 1e-4f; // convert area from cm2 to m2 for our calculations
			mSurfaceAreaOfBoat += tri.Area;
		}
	}

	mLengthOfBoat = FVector::Dist(mVertices[mTriLocations[0].Index1], mVertices[mTriLocations[mTriLocations.Num() - 1].Index3]);
}

// Called every frame
void UWaterPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CalculateWaterIntersection();
}

void UWaterPhysicsComponent::CalculateWaterIntersection()
{
	CalculateVertexLocations();
	MarkSubmergedTriangles();

#ifdef DRAW_DEBUG
	for (auto& tri : mTriLocations)
	{
#ifdef DRAW_MESH_DEBUG
		DrawDebugLine(GetWorld(), mVertices[tri.Index1], mVertices[tri.Index2], FColor::Red, false, -1, 0, 1.0f);
		DrawDebugLine(GetWorld(), mVertices[tri.Index2], mVertices[tri.Index3], FColor::Red, false, -1, 0, 1.0f);
		DrawDebugLine(GetWorld(), mVertices[tri.Index3], mVertices[tri.Index1], FColor::Red, false, -1, 0, 1.0f);

		// Normals
		FVector triNormal = ((mVertices[tri.Index1] - mVertices[tri.Index2]) ^ (mVertices[tri.Index2] - mVertices[tri.Index3])).GetSafeNormal();

		FVector centroid((mVertices[tri.Index1].X + mVertices[tri.Index2].X + mVertices[tri.Index3].X) / 3,
			(mVertices[tri.Index1].Y + mVertices[tri.Index2].Y + mVertices[tri.Index3].Y) / 3,
			(mVertices[tri.Index1].Z + mVertices[tri.Index2].Z + mVertices[tri.Index3].Z) / 3);

		DrawDebugLine(GetWorld(), centroid, centroid + (100 * triNormal), FColor::Red, false, -1, 0, 2.0f);
#endif

#ifdef DRAW_PROJECTION_DEBUG
		if (mWaveworksDisplacements.Num() > 0)
		{
			if (tri.Submerged == Submersion::Full)
			{
				DrawDebugLine(GetWorld(), FVector(mVertices[tri.Index1].X, mVertices[tri.Index1].Y, mWaveworksDisplacements[tri.Index1]), FVector(mVertices[tri.Index2].X, mVertices[tri.Index2].Y, mWaveworksDisplacements[tri.Index2]), FColor::Red, false, -1, 0, 2.0f);
				DrawDebugLine(GetWorld(), FVector(mVertices[tri.Index2].X, mVertices[tri.Index2].Y, mWaveworksDisplacements[tri.Index2]), FVector(mVertices[tri.Index3].X, mVertices[tri.Index3].Y, mWaveworksDisplacements[tri.Index3]), FColor::Red, false, -1, 0, 2.0f);
				DrawDebugLine(GetWorld(), FVector(mVertices[tri.Index3].X, mVertices[tri.Index3].Y, mWaveworksDisplacements[tri.Index3]), FVector(mVertices[tri.Index1].X, mVertices[tri.Index1].Y, mWaveworksDisplacements[tri.Index1]), FColor::Red, false, -1, 0, 2.0f);
			}
		}
#endif
	}
#endif
}

void UWaterPhysicsComponent::CalculateVertexLocations()
{
	check(mMeshComponent);
	check(mMeshComponent->IsValidLowLevel());

	FTransform meshTransform = mMeshComponent->GetComponentTransform();

	// Get the reference to the Triangle Mesh of the MeshComponent.
	auto triMesh = mMeshComponent->GetBodySetup()->TriMeshes[0];
	check(triMesh);

	mVertices.Reset();
	TArray<FVector2D> vertexXYPositions;

	// Get the number of vertices and the pointer to vertices array.
	PxU32 vertexCount = triMesh->getNbVertices();
	const PxVec3* vertices = triMesh->getVertices();

	// For each vertex, transform the position to match the component Transform 
	for (PxU32 i = 0; i < vertexCount; i++)
	{
		mVertices.Add(meshTransform.TransformPosition(P2UVector(vertices[i])));
		vertexXYPositions.Add(FVector2D(mVertices[i].X / 100, mVertices[i].Y / 100));
	}

	mWaveWorksComponent->SampleDisplacements(vertexXYPositions, mWaveworksDisplacementDelegate);
}

void UWaterPhysicsComponent::OnRecievedWaveWorksDisplacement(TArray<FVector4> OutDisplacements)
{
	FScopeLock scopeLock(mWaveWorksDisplacementLock);
	mWaveworksDisplacements.Reset();
	for (int32 i = 0; (i < OutDisplacements.Num()); ++i)
	{
		mWaveworksDisplacements.Add((OutDisplacements[i].Z * 100.0f) + mWaveWorksComponent->SeaLevel);
	}
}

void UWaterPhysicsComponent::MarkSubmergedTriangles()
{
	if (mWaveworksDisplacements.Num() > 0)
	{
		FScopeLock scopeLock(mWaveWorksDisplacementLock);
		float submergedArea = 0.0f;

		for (auto& tri : mTriLocations)
		{
			TArray<float> heights;
			heights.Reserve(3);
			heights.Add(mVertices[tri.Index1].Z - mWaveworksDisplacements[tri.Index1]);
			heights.Add(mVertices[tri.Index2].Z - mWaveworksDisplacements[tri.Index2]);
			heights.Add(mVertices[tri.Index3].Z - mWaveworksDisplacements[tri.Index3]);

			// If all vertices are submerged
			if ((heights[0] < 0.0f) && (heights[1] < 0.0f) && (heights[2] < 0.0f))
			{
				tri.Submerged = Submersion::Full;
				ApplyHydrostaticForce(tri, heights);
				submergedArea += tri.Area;
			}
			// If any two vertices are submerged
			else if (((heights[0] < 0.0f) && (heights[1] < 0.0f)) || ((heights[1] < 0.0f) && (heights[2] < 0.0f)) || ((heights[2] < 0.0f) && (heights[0] < 0.0f)))
			{
				tri.Submerged = Submersion::PartialTwoVertices;
			}
			// If any one vertex is submerged
			else if ((heights[0] < 0.0f) || (heights[1] < 0.0f) || (heights[2] < 0.0f))
			{
				tri.Submerged = Submersion::PartialSingleVertex;
			}
			// If no vertices are submerged
			else
			{
				tri.Submerged = Submersion::None;
			}
		}

		float ratioOfSubmergedArea = submergedArea / mSurfaceAreaOfBoat;
		mLengthOfSubmerged = ratioOfSubmergedArea * mLengthOfBoat;
	}
}

inline void UWaterPhysicsComponent::ApplyHydrostaticForce(const Tri& submergedTri, TArray<float>& heights)
{
	// Calculate the normal of the triangle by getting the cross product of two edges
	FVector triNormal = BoatPhysicsUtil::TriangleNormal(mVertices[submergedTri.Index1], mVertices[submergedTri.Index2], mVertices[submergedTri.Index3]);

	// If the normal is pointing downwards, apply the force.
	if (triNormal.Z < 0)
	{
		FVector centroid = BoatPhysicsUtil::CentroidOfTriangle(mVertices[submergedTri.Index1], mVertices[submergedTri.Index2], mVertices[submergedTri.Index3]);
		FVector triangleVelocity = BoatPhysicsUtil::TriangleVelocity(mMeshComponent, centroid);

		FVector hydrostaticForce = BoatPhysicsUtil::HydrostaticForce(GetWorld()->GetGravityZ() * mMeshComponent->GetBodyInstance()->GetBodyMass(), heights, submergedTri.Area, triNormal);

		FVector viscousWaterResistance = BoatPhysicsUtil::ViscousWaterResistanceForce(triangleVelocity, triNormal, submergedTri.Area, BoatPhysicsUtil::ResistanceCoefficient(mMeshComponent->GetComponentVelocity().Size(), mLengthOfSubmerged));

		float cosVelocityAndNormal = FVector::DotProduct(triangleVelocity.GetSafeNormal(), triNormal);
		FVector pressureDragForce = BoatPhysicsUtil::PressureDragForce(cosVelocityAndNormal, triNormal, submergedTri.Area);

#ifdef DRAW_DEBUG
#ifdef DRAW_FORCES_DEBUG
		DrawDebugLine(GetWorld(), centroid, centroid + (0.01 * hydrostaticForce), FColor::Red, false, -1, 0, 2.0f);
		DrawDebugLine(GetWorld(), centroid, centroid + (0.01 * viscousWaterResistance), FColor::Green, false, -1, 0, 2.0f);
		DrawDebugLine(GetWorld(), centroid, centroid + (0.01 * pressureDragForce), FColor::Blue, false, -1, 0, 2.0f);
#endif
#endif
		mMeshComponent->AddImpulseAtLocation(hydrostaticForce + viscousWaterResistance + pressureDragForce, centroid);
	}
}