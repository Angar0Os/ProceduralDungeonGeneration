// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "generator.generated.h"

UCLASS()
class TBG_API Agenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	Agenerator();

protected:
	virtual void BeginPlay() override;

	void SpawnRoom();
	void SpawnInstantRooms();

	static float GetFloorSize(const AActor* Actor);

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int basePieceCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector3f basePoint = FVector3f(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float radius = 1.0f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> baseActor = nullptr;

	UPROPERTY(EditAnywhere)
	bool seeSpawn = true;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rooms")
	TArray<AActor*> SpawnedRooms;
	
	int SpawnedCount = 0;
	FTimerHandle SpawnTimer;
};
