// Fill out your copyright notice in the Description page of Project Settings.

#include "WaveworksTester.h"
#include "CustomComponents/WaterPhysicsComponent.h"
#include "BoatPhysicsUtil.h"

const float BoatPhysicsUtil::DensityOfWater = 0.00001f; // kg/cm3

const float BoatPhysicsUtil::LinearPressureDrag = 2500.0f;
const float BoatPhysicsUtil::QuadraticPressureDrag = 2500.0f;
const float BoatPhysicsUtil::PressureFalloffPower = 0.5f;

const float BoatPhysicsUtil::LinearSuctionDrag = 2500.0f;
const float BoatPhysicsUtil::QuadraticSuctionDrag = 2500.0f;
const float BoatPhysicsUtil::SuctionFalloffPower = 0.5f;

FVector BoatPhysicsUtil::TriangleVelocity(UStaticMeshComponent* boatMesh, const FVector& triangleCentre)
{
	// velocity at a point on body = velocity in centre of body + (angular velocity of body ^ vector between centre and the point)
	return (boatMesh->GetComponentVelocity() + (boatMesh->GetPhysicsAngularVelocity() ^ (triangleCentre - boatMesh->GetCenterOfMass())));
}

float BoatPhysicsUtil::TriangleArea(const FVector& vertex1, const FVector& vertex2, const FVector& vertex3)
{
	// area = 1/2 * base * height
	return (0.5 * FVector::Dist(vertex2, vertex3) * FMath::PointDistToLine(vertex1, vertex2 - vertex3, vertex2));
}

float BoatPhysicsUtil::ResistanceCoefficient(float velocity, float length)
{
	float viscosity = 0.00001002f;
	float reynoldsNumber = (velocity * length) / viscosity;
	float coefficient = 0.075f / FMath::Pow((FMath::LogX(10.0f, reynoldsNumber) - 2.0f), 2.0f);

	return coefficient;
}

FVector BoatPhysicsUtil::PressureDragForce(float cosVelocityAndNormal, const FVector & triNormal, float surfaceArea)
{
	FVector pressureDragForce;

	if (cosVelocityAndNormal > 0.0f)
	{
		pressureDragForce = -(LinearPressureDrag  + QuadraticPressureDrag) * surfaceArea * FMath::Pow(cosVelocityAndNormal, PressureFalloffPower) * triNormal;
	}
	else
	{
		pressureDragForce = (LinearSuctionDrag + QuadraticSuctionDrag) * surfaceArea * FMath::Pow(FMath::Abs(cosVelocityAndNormal), SuctionFalloffPower) * triNormal;
	}

	return pressureDragForce;
}

FVector BoatPhysicsUtil::CentroidOfTriangle(const FVector& vertex1, const FVector& vertex2, const FVector& vertex3)
{
	// centroid of triangle = Vector composed of the average of the x, y and z components of the vertices.
	return FVector((vertex1.X + vertex2.X + vertex3.X)/3, 
		(vertex1.Y + vertex2.Y + vertex3.Y) / 3,
		(vertex1.Z + vertex2.Z + vertex3.Z) / 3);
}

FVector BoatPhysicsUtil::HydrostaticForce(float weightOfBody, const TArray<float>& heights, float surfaceArea, const FVector& triNormal)
{
	// F = rho * g * height of submersion * Surface Area * normal to the surface

	float hCentre = (heights[0] + heights[1] + heights[2]) / 3;

	FVector hydrostaticForce = (-DensityOfWater * weightOfBody * hCentre * surfaceArea * triNormal);

	// Zero out the residual torque in the xy plane
	//hydrostaticForce.X = 0.0f;
	//hydrostaticForce.Y = 0.0f;

	return hydrostaticForce;
}

FVector BoatPhysicsUtil::ViscousWaterResistanceForce(const FVector& velocity, const FVector& triNormal, float surfaceArea, float resistanceCoefficient)
{
	// Force = 1/2 * Density of Water * velocity squared * surface area of triangle * coefficient of frictional resistance acting on the body

	FVector velocityTangent = FVector::CrossProduct(triNormal, FVector::CrossProduct(triNormal, velocity) / velocity.Size()) / velocity.Size();
	FVector tangentialDirection = velocityTangent.GetSafeNormal() * -1.0f;
	FVector velocityOfFlow = velocity.Size() * tangentialDirection;

	return (0.5f * DensityOfWater * velocityOfFlow.Size() * velocityOfFlow * surfaceArea * resistanceCoefficient);
}

FVector BoatPhysicsUtil::TriangleNormal(const FVector& vertex1, const FVector& vertex2, const FVector& vertex3)
{
	return ((vertex1 - vertex2) ^ (vertex2 - vertex3)).GetSafeNormal();
}