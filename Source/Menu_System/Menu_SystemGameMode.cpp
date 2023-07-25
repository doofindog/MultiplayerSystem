// Copyright Epic Games, Inc. All Rights Reserved.

#include "Menu_SystemGameMode.h"
#include "Menu_SystemCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMenu_SystemGameMode::AMenu_SystemGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
