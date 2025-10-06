// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "generator.generated.h"

USTRUCT()
struct FDelaunayEdge
{
	GENERATED_BODY()

	FVector2D A;
	FVector2D B;

	FDelaunayEdge() {}
	FDelaunayEdge(const FVector2D& InA, const FVector2D& InB) : A(InA), B(InB) {}

	// Determines if two edges connect the same points, regardless of order.
	bool operator==(const FDelaunayEdge& Other) const
	{
		return (A == Other.A && B == Other.B) || (A == Other.B && B == Other.A);
	}
};

USTRUCT()
struct FDelaunayTriangle
{
	GENERATED_BODY()

	FVector2D A;
	FVector2D B;
	FVector2D C;

	FDelaunayTriangle() {}
	FDelaunayTriangle(const FVector2D& InA, const FVector2D& InB, const FVector2D& InC) : A(InA), B(InB), C(InC) {}

	// Determines if two triangles have the same three vertices, regardless of order.
	bool operator==(const FDelaunayTriangle& Other) const
	{
		TArray<FVector2D> This = { A, B, C };
		TArray<FVector2D> That = { Other.A, Other.B, Other.C };

		for (const FVector2D& P : This)
		{
			if (!That.ContainsByPredicate([&](const FVector2D& Q) { return Q == P; }))
			{
				return false;
			}
		}
		return true;
	}
};

UCLASS()
class TBG_API Agenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	Agenerator();
	
protected:
    virtual void BeginPlay() override;

public:	
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, Category="Spawn")
    TSubclassOf<AActor> baseActor;

    UPROPERTY(EditAnywhere, Category="Spawn")
    bool seeSpawn = false;

    UPROPERTY(EditAnywhere, Category="Spawn")
    int32 basePieceCount = 100;

    UPROPERTY(EditAnywhere, Category="Spawn")
    float radius = 1000.f;

    UPROPERTY(EditAnywhere, Category="Spawn")
    FVector basePoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug|SuperTriangle")
    bool showSuperTriangle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug|SuperTriangle")
    float SuperTriangleGap = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug|Delaunay")
    bool ShowDelaunay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Delaunay Debug")
	bool StepByStep = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Delaunay Debug")
	float StepDelay = 0.5f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	bool bGenerateCorridors = false;

	UFUNCTION()
	void BuildCorridors();

	UFUNCTION()
	void FinalizeRooms();

	UPROPERTY()
	TArray<AActor*> SpawnedRooms;

	UPROPERTY()
	TArray<AActor*> MainRooms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	int32 DebugStep = 0; 

	UPROPERTY()
	TArray<FDelaunayEdge> MSTEdges;

	void BuildMST();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool ShowMST = false;

	UFUNCTION()
	void NextDebugStep();
	
	FTimerHandle DelaunayStepTimer;
	int32 CurrentPointIndex = 0;
	TArray<FVector2D> DelaunayPoints;

	TArray<FDelaunayTriangle> BadTrianglesStep;
	TArray<FDelaunayEdge> PolygonEdgesStep;

private:
    FTimerHandle SpawnTimer;
    int32 SpawnedCount = 0;

    FVector SuperTriangleA;
    FVector SuperTriangleB;
    FVector SuperTriangleC;
    bool SuperTriangleComputed = false;

    TArray<FDelaunayTriangle> Triangles;

public:
    void SpawnRoom();
    void SpawnInstantRooms();
    float GetFloorSize(const AActor* Actor);

    void ComputeSuperTriangle();

    void BuildDelaunay();
	void DelaunayStep();
    void InsertPointDelaunay(const FVector2D& P);
    bool IsPointInsideCircumcircle(const FVector2D& P, const FDelaunayTriangle& Tri) const;
    bool TriangleHasEdge(const FDelaunayTriangle& Tri, const FDelaunayEdge& Edge) const;
    void CleanupTriangles();
};
