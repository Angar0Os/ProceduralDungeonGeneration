// Fill out your copyright notice in the Description page of Project Settings.


#include "generator.h"
#include "baseActor.h"

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

			if (GEngine)
			{
				FString DebugText = FString::Printf(TEXT("MainRoom: %s - Area: %.2f"), 
					*RoomActor->GetName(), RoomActor->RoomData.FloorArea);
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, DebugText);
			}
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
		}
	}
}

// Called every frame
void Agenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	float Z = basePoint.Z;

	FVector SpawnLocation(X, Y, Z);

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

float Agenerator::GetFloorSize(const AActor* Actor)
{
	FVector Size = Actor->GetComponentsBoundingBox().GetSize();
	FVector Scale = Actor->GetActorScale3D();

	float Width  = Size.X * Scale.X;
	float Depth  = Size.Y * Scale.Y;
	
	float Area = Width * Depth;

	return Area;
}
