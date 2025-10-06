// Fill out your copyright notice in the Description page of Project Settings.


#include "generator.h"
#include "baseActor.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

// Sets default values
Agenerator::Agenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void Agenerator::BeginPlay()
{
	Super::BeginPlay();

	if (seeSpawn)
	{
		GetWorld()->GetTimerManager().SetTimer(
			SpawnTimer,
			this,
			&Agenerator::SpawnRoom,
			0.1f,
			true
		);
	}
	else
	{
		for (int i = 0; i < basePieceCount; i++)
		{
			SpawnInstantRooms();
		}
	}

	SpawnedRooms.Sort([](const AActor& A, const AActor& B)
		{
			const AbaseActor* RoomA = Cast<AbaseActor>(&A);
			const AbaseActor* RoomB = Cast<AbaseActor>(&B);

			if (!RoomA || !RoomB) return false;

			return RoomA->RoomData.FloorArea > RoomB->RoomData.FloorArea;
		});

	int32 NumRooms = SpawnedRooms.Num();
	int32 NumMainRooms = FMath::CeilToInt(NumRooms * 0.2f);
	NumMainRooms = FMath::Max(NumMainRooms, 3);
	NumMainRooms = FMath::Min(NumMainRooms, NumRooms);

	for (int32 i = 0; i < NumMainRooms; i++)
	{
		if (AbaseActor* RoomActor = Cast<AbaseActor>(SpawnedRooms[i]))
		{
			RoomActor->RoomData.bMainRoom = true;
		}
	}

	for (AActor* Actor : SpawnedRooms)
	{
		if (AbaseActor* Room = Cast<AbaseActor>(Actor))
		{
			if (!Room->RoomData.bMainRoom)
			{
				Room->SetActorHiddenInGame(true);
				Room->SetActorEnableCollision(false);
				Room->SetActorTickEnabled(false);
			}
			else
			{
				MainRooms.Add(Room);
			}
		}
	}

	ComputeSuperTriangle();
	BuildDelaunay();

	EnableInput(GetWorld()->GetFirstPlayerController());
	InputComponent->BindAction("NextDebugStep", IE_Pressed, this, &Agenerator::NextDebugStep);
}

// Called every frame
void Agenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (showSuperTriangle && SuperTriangleComputed)
	{
		DrawDebugLine(GetWorld(), SuperTriangleA, SuperTriangleB, FColor::Blue, false, 0.f, 0, 10.f);
		DrawDebugLine(GetWorld(), SuperTriangleB, SuperTriangleC, FColor::Blue, false, 0.f, 0, 10.f);
		DrawDebugLine(GetWorld(), SuperTriangleC, SuperTriangleA, FColor::Blue, false, 0.f, 0, 10.f);
	}

	if (ShowDelaunay)
	{
		for (const FDelaunayTriangle& Tri : Triangles)
		{
			FVector A3D(Tri.A.X, Tri.A.Y, 0.f);
			FVector B3D(Tri.B.X, Tri.B.Y, 0.f);
			FVector C3D(Tri.C.X, Tri.C.Y, 0.f);

			DrawDebugLine(GetWorld(), A3D, B3D, FColor::Yellow, false, 0.f, 0, 2.f);
			DrawDebugLine(GetWorld(), B3D, C3D, FColor::Yellow, false, 0.f, 0, 2.f);
			DrawDebugLine(GetWorld(), C3D, A3D, FColor::Yellow, false, 0.f, 0, 2.f);
		}
	}

	if (ShowMST)
	{
		for (const FDelaunayEdge& Edge : MSTEdges)
		{
			FVector A3D(Edge.A.X, Edge.A.Y, 0.f);
			FVector B3D(Edge.B.X, Edge.B.Y, 0.f);

			DrawDebugLine(GetWorld(), A3D, B3D, FColor(0, 100, 0), false, 0.f, 0, 6.f);
		}
	}
}

void Agenerator::SpawnRoom()
{
	if(SpawnedCount >= basePieceCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
		return;
	}
	
	SpawnInstantRooms();
	SpawnedCount++;
}

void Agenerator::SpawnInstantRooms() 
{
	float angle = FMath::FRand() * 2 * PI; 

	float r = FMath::Sqrt(FMath::FRand()) * radius; 
	float X = basePoint.X + FMath::Cos(angle) * r; 
	float Y = basePoint.Y + FMath::Sin(angle) * r; 
	float Z = basePoint.Z; FVector SpawnLocation(X, Y, Z); 
	
	if (AActor* spawnedRoom = GetWorld()->SpawnActor<AActor>(baseActor, SpawnLocation, FRotator::ZeroRotator)) 
	{ 
		SpawnedRooms.Add(spawnedRoom); 

		float ScaleX = FMath::FRandRange(0.5f, 3.f); 
		float ScaleY = FMath::FRandRange(0.5f, 3.f); 
		float ScaleZ = FMath::FRandRange(0.5f, 3.f); 
		
		spawnedRoom->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ)); 
		
		if (AbaseActor* RoomActor = Cast<AbaseActor>(spawnedRoom)) 
		{ 
			RoomActor->RoomData.FloorArea = GetFloorSize(spawnedRoom); 
			RoomActor->RoomData.bMainRoom = false; 
		} 
	} 
}

void Agenerator::ComputeSuperTriangle()
{
	if (MainRooms.Num() == 0)
		return;

	float MinX =  FLT_MAX;
	float MaxX = -FLT_MAX;
	float MinY =  FLT_MAX;
	float MaxY = -FLT_MAX;

	for (AActor* Room : MainRooms)
	{
		FVector Pos = Room->GetActorLocation();
		MinX = FMath::Min(MinX, Pos.X);
		MaxX = FMath::Max(MaxX, Pos.X);
		MinY = FMath::Min(MinY, Pos.Y);
		MaxY = FMath::Max(MaxY, Pos.Y);
	}

	FVector2D Center((MinX + MaxX) / 2.0f, (MinY + MaxY) / 2.0f);
	
	float Width  = (MaxX - MinX) + SuperTriangleGap;
	float Height = (MaxY - MinY) + SuperTriangleGap;
	float Radius = FMath::Max(Width, Height);

	FVector2D P1 = Center + FVector2D(0, Radius); 
	FVector2D P2 = Center + FVector2D(-Radius * FMath::Sin(PI / 3), -Radius / 2);
	FVector2D P3 = Center + FVector2D( Radius * FMath::Sin(PI / 3), -Radius / 2);

	SuperTriangleA = FVector(P1.X, P1.Y, 0);
	SuperTriangleB = FVector(P2.X, P2.Y, 0);
	SuperTriangleC = FVector(P3.X, P3.Y, 0);

	SuperTriangleComputed = true;
}

float Agenerator::GetFloorSize(const AActor* Actor)
{
	FVector Size = Actor->GetComponentsBoundingBox().GetSize();
	FVector Scale = Actor->GetActorScale3D();

	float Width  = Size.X * Scale.X;
	float Depth  = Size.Y * Scale.Y;
	
	float Area = Width * Depth;

	return Area;
}

void Agenerator::BuildDelaunay()
{
	Triangles.Empty();

	FDelaunayTriangle SuperTri(
		FVector2D(SuperTriangleA.X, SuperTriangleA.Y),
		FVector2D(SuperTriangleB.X, SuperTriangleB.Y),
		FVector2D(SuperTriangleC.X, SuperTriangleC.Y)
	);

	Triangles.Add(SuperTri);

	DelaunayPoints.Empty();
	for (AActor* Room : MainRooms)
	{
		FVector Pos = Room->GetActorLocation();
		DelaunayPoints.Add(FVector2D(Pos.X, Pos.Y));
	}

	for (int i = 0; i < DelaunayPoints.Num(); ++i)
	{
		int j = FMath::RandRange(0, DelaunayPoints.Num() - 1);
		DelaunayPoints.Swap(i, j);
	}

	if (StepByStep)
	{
		CurrentPointIndex = 0;
		GetWorld()->GetTimerManager().SetTimer(
			DelaunayStepTimer,
			this,
			&Agenerator::DelaunayStep,
			StepDelay,
			true
		);
	}
	else
	{
		for (const FVector2D& P : DelaunayPoints)
		{
			InsertPointDelaunay(P);
		}
		CleanupTriangles();
	}
}

void Agenerator::DelaunayStep()
{
	if (CurrentPointIndex >= DelaunayPoints.Num())
	{
		GetWorld()->GetTimerManager().ClearTimer(DelaunayStepTimer);
		CleanupTriangles();
		return;
	}

	const FVector2D& P = DelaunayPoints[CurrentPointIndex];
	BadTrianglesStep.Empty();
	PolygonEdgesStep.Empty();

	for (const FDelaunayTriangle& Tri : Triangles)
	{
		if (IsPointInsideCircumcircle(P, Tri))
		{
			BadTrianglesStep.Add(Tri);
		}
	}

	for (const FDelaunayTriangle& BadTri : BadTrianglesStep)
	{
		FDelaunayEdge Edges[3] = {
			FDelaunayEdge(BadTri.A, BadTri.B),
			FDelaunayEdge(BadTri.B, BadTri.C),
			FDelaunayEdge(BadTri.C, BadTri.A)
		};

		for (const FDelaunayEdge& E : Edges)
		{
			bool bShared = false;
			for (const FDelaunayTriangle& Other : BadTrianglesStep)
			{
				if (&BadTri == &Other) continue;
				if (TriangleHasEdge(Other, E))
				{
					bShared = true;
					break;
				}
			}
			if (!bShared)
			{
				PolygonEdgesStep.Add(E);
			}
		}
	}

	for (const FDelaunayTriangle& BT : BadTrianglesStep)
	{
		Triangles.Remove(BT);
	}

	for (const FDelaunayEdge& E : PolygonEdgesStep)
	{
		FDelaunayTriangle NewTriangle(E.A, E.B, P);
		Triangles.Add(NewTriangle);
	}

	DrawDebugSphere(GetWorld(), FVector(P.X, P.Y, 10.f), 50.f, 12, FColor::Cyan, false, StepDelay, 0, 2.f);

	for (const FDelaunayTriangle& T : BadTrianglesStep)
	{
		DrawDebugLine(GetWorld(), FVector(T.A.X, T.A.Y, 0), FVector(T.B.X, T.B.Y, 0), FColor::Red, false, StepDelay, 0, 3.f);
		DrawDebugLine(GetWorld(), FVector(T.B.X, T.B.Y, 0), FVector(T.C.X, T.C.Y, 0), FColor::Red, false, StepDelay, 0, 3.f);
		DrawDebugLine(GetWorld(), FVector(T.C.X, T.C.Y, 0), FVector(T.A.X, T.A.Y, 0), FColor::Red, false, StepDelay, 0, 3.f);
	}

	for (const FDelaunayEdge& E : PolygonEdgesStep)
	{
		DrawDebugLine(GetWorld(), FVector(E.A.X, E.A.Y, 0), FVector(E.B.X, E.B.Y, 0), FColor::Green, false, StepDelay, 0, 5.f);
	}

	CurrentPointIndex++;
}


void Agenerator::InsertPointDelaunay(const FVector2D& P)
{
	TArray<FDelaunayTriangle> BadTriangles;

	for (const FDelaunayTriangle& Tri : Triangles)
	{
		if (IsPointInsideCircumcircle(P, Tri))
		{
			BadTriangles.Add(Tri);
		}
	}

	TArray<FDelaunayEdge> PolygonEdges;
	for (const FDelaunayTriangle& BadTri : BadTriangles)
	{
		FDelaunayEdge Edges[3] = {
			FDelaunayEdge(BadTri.A, BadTri.B),
			FDelaunayEdge(BadTri.B, BadTri.C),
			FDelaunayEdge(BadTri.C, BadTri.A)
		};

		for (const FDelaunayEdge& E : Edges)
		{
			bool bShared = false;
			for (const FDelaunayTriangle& Other : BadTriangles)
			{
				if (&BadTri == &Other) continue; 
				if (TriangleHasEdge(Other, E))
				{
					bShared = true;
					break;
				}
			}
			if (!bShared)
			{
				PolygonEdges.Add(E);
			}
		}
	}

	for (const FDelaunayTriangle& BT : BadTriangles)
	{
		Triangles.Remove(BT);
	}

	for (const FDelaunayEdge& E : PolygonEdges)
	{
		FDelaunayTriangle NewTriangle(E.A, E.B, P);
		Triangles.Add(NewTriangle);
	}
}

bool Agenerator::IsPointInsideCircumcircle(const FVector2D& P, const FDelaunayTriangle& Tri) const
{
	double ax = Tri.A.X - P.X;
	double ay = Tri.A.Y - P.Y;
	double bx = Tri.B.X - P.X;
	double by = Tri.B.Y - P.Y;
	double cx = Tri.C.X - P.X;
	double cy = Tri.C.Y - P.Y;

	double det = (ax * ax + ay * ay) * (bx * cy - cx * by)
			   - (bx * bx + by * by) * (ax * cy - cx * ay)
			   + (cx * cx + cy * cy) * (ax * by - bx * ay);

	return det > 0.0;
}

bool Agenerator::TriangleHasEdge(const FDelaunayTriangle& Tri, const FDelaunayEdge& Edge) const
{
	FDelaunayEdge E1(Tri.A, Tri.B);
	FDelaunayEdge E2(Tri.B, Tri.C);
	FDelaunayEdge E3(Tri.C, Tri.A);

	return (E1 == Edge) || (E2 == Edge) || (E3 == Edge);
}

void Agenerator::CleanupTriangles()
{
	FVector2D SA(SuperTriangleA.X, SuperTriangleA.Y);
	FVector2D SB(SuperTriangleB.X, SuperTriangleB.Y);
	FVector2D SC(SuperTriangleC.X, SuperTriangleC.Y);

	Triangles.RemoveAll([&](const FDelaunayTriangle& T)
	{
		return (T.A == SA) || (T.A == SB) || (T.A == SC) ||
			   (T.B == SA) || (T.B == SB) || (T.B == SC) ||
			   (T.C == SA) || (T.C == SB) || (T.C == SC);
	});
}

void Agenerator::NextDebugStep()
{
	DebugStep++;

	if (DebugStep > 1)
		DebugStep = 0;

	switch (DebugStep)
	{
	case 0:
		ShowDelaunay = true;
		ShowMST = false;
		break;

	case 1: 
		ShowDelaunay = false;
		ShowMST = true;
		BuildMST();

		if (bGenerateCorridors)
			BuildCorridors();
		break;
	}
}

void Agenerator::BuildMST()
{
	MSTEdges.Empty();

	TArray<FVector2D> Points;
	for (AActor* Room : MainRooms)
	{
		if (!Room) continue;
		FVector Pos = Room->GetActorLocation();
		Points.Add(FVector2D(Pos.X, Pos.Y));
	}

	if (Points.Num() == 0) return;

	TArray<int32> Visited;
	Visited.Add(0); 

	while (Visited.Num() < Points.Num())
	{
		float MinDist = FLT_MAX;
		int32 BestA = -1, BestB = -1;

		for (int32 A : Visited)
		{
			for (int32 B = 0; B < Points.Num(); ++B)
			{
				if (Visited.Contains(B)) continue;

				float Dist = FVector2D::Distance(Points[A], Points[B]);
				if (Dist < MinDist)
				{
					MinDist = Dist;
					BestA = A;
					BestB = B;
				}
			}
		}

		if (BestA != -1 && BestB != -1)
		{
			MSTEdges.Add(FDelaunayEdge(Points[BestA], Points[BestB]));
			Visited.Add(BestB);
		}
	}
}

void Agenerator::BuildCorridors()
{
	if (!bGenerateCorridors || MSTEdges.Num() == 0 || !baseActor)
		return;

	for (const FDelaunayEdge& Edge : MSTEdges)
	{
		FVector2D A2D = Edge.A;
		FVector2D B2D = Edge.B;

		FVector A3D(A2D.X, A2D.Y, basePoint.Z);
		FVector B3D(B2D.X, B2D.Y, basePoint.Z);

		bool bHorizontal = FMath::IsNearlyEqual(A3D.Y, B3D.Y, 1.f);
		bool bVertical = FMath::IsNearlyEqual(A3D.X, B3D.X, 1.f);

		if (bHorizontal || bVertical)
		{
			DrawDebugLine(GetWorld(), A3D, B3D, FColor::Cyan, false, 5000.f, 0, 3.f);
		}
		else
		{
			FVector Intermediate(A3D.X, B3D.Y, basePoint.Z);

			DrawDebugLine(GetWorld(), A3D, Intermediate, FColor::Cyan, false, 5000.f, 0, 3.f);
			DrawDebugLine(GetWorld(), Intermediate, B3D, FColor::Cyan, false, 5000.f, 0, 3.f);
		}
	}

	FinalizeRooms();
}

void Agenerator::FinalizeRooms()
{
	TSet<AActor*> FinalRooms;

	// 1?? On garde toutes les pièces principales
	for (AActor* Room : MainRooms)
	{
		if (!Room) continue;
		FinalRooms.Add(Room);
		Room->SetActorHiddenInGame(false);
		Room->SetActorEnableCollision(true);
		Room->SetActorTickEnabled(true);
	}

	// 2?? Ajouter toutes les pièces secondaires qui se trouvent sur les chemins MST
	for (const FDelaunayEdge& Edge : MSTEdges)
	{
		FVector2D Start2D = Edge.A;
		FVector2D End2D = Edge.B;
		FVector2D Dir = End2D - Start2D;

		for (AActor* Room : SpawnedRooms)
		{
			if (!Room || FinalRooms.Contains(Room)) continue;

			// On ne regarde que les pièces secondaires
			if (AbaseActor* RoomActor = Cast<AbaseActor>(Room))
			{
				if (RoomActor->RoomData.bMainRoom)
					continue;
			}

			FVector Pos3D = Room->GetActorLocation();
			FVector2D Pos2D(Pos3D.X, Pos3D.Y);

			FVector2D StartToPoint = Pos2D - Start2D;

			// Projection du point sur la ligne
			float T = FVector2D::DotProduct(StartToPoint, Dir) / Dir.SizeSquared();
			T = FMath::Clamp(T, 0.f, 1.f);

			FVector2D ClosestPoint = Start2D + Dir * T;

			float Dist = FVector2D::Distance(Pos2D, ClosestPoint);

			if (Dist < 50.f) // Tolérance de 50 unités
			{
				FinalRooms.Add(Room);
				Room->SetActorHiddenInGame(false);
				Room->SetActorEnableCollision(true);
				Room->SetActorTickEnabled(true);
			}
		}
	}

	// 3?? Supprimer toutes les pièces non finales
	for (AActor* Room : SpawnedRooms)
	{
		if (!FinalRooms.Contains(Room))
		{
			Room->Destroy();
		}
	}

	// Met à jour la liste des pièces actuelles
	SpawnedRooms = FinalRooms.Array();
}







