// Copyright Epic Games, Inc. All Rights Reserved.

#include "TBGGameMode.h"
#include "TBGCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATBGGameMode::ATBGGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
