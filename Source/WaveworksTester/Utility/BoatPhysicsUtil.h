// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 
 */

class WAVEWORKSTESTER_API BoatPhysicsUtil
{
public:
	~BoatPhysicsUtil() = default;

	static FVector TriangleVelocity(UStaticMeshComponent* boatMesh, const FVector& triangleCentre);

	static float TriangleArea(const FVector& vertex1, const FVector& vertex2, const FVector& vertex3);

	static FVector HydrostaticForce(float weightOfBody, const TArray<float>& heights, float surfaceArea, const FVector& triNormal);

	static FVector ViscousWaterResistanceForce(const FVector& velocity, const FVector& triNormal, float surfaceArea, float resistanceCoefficient);

	static float ResistanceCoefficient(float velocity, float length);

	static FVector PressureDragForce(float cosVelocityAndNormal, const FVector& triNormal, float surfaceArea);

	static FVector CentroidOfTriangle(const FVector& vertex1, const FVector& vertex2, const FVector& vertex3);

	static FVector TriangleNormal(const FVector& vertex1, const FVector& vertex2, const FVector& vertex3);

	//static FVector SlammingForce();

private:
	BoatPhysicsUtil() = default;

	// Density Constants
	static const float DensityOfWater;

	// Pressure Forces coefficients
	static const float LinearPressureDrag;
	static const float QuadraticPressureDrag;
	static const float PressureFalloffPower;

	// Suction Forces coefficients
	static const float LinearSuctionDrag;
	static const float QuadraticSuctionDrag;
	static const float SuctionFalloffPower;
};
