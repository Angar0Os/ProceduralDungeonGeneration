// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "baseActor.generated.h"

USTRUCT(BlueprintType)
struct FRoomData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
	float FloorArea = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
	bool bMainRoom = false;
};

UCLASS()
class TBG_API AbaseActor : public AActor
{
	GENERATED_BODY()
    
public:    
	AbaseActor();

protected:
	virtual void BeginPlay() override;

public:    
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
	FRoomData RoomData;
};
