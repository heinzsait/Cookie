// Copyright Epic Games, Inc. All Rights Reserved.

#include "CookieGameMode.h"
#include "CookieCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACookieGameMode::ACookieGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Characters/BP_PlayerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
