// Copyright Epic Games, Inc. All Rights Reserved.

#include "GizmoGameMode.h"
#include "GizmoCharacter.h"
#include "GizmoPlayerController.h"
#include "UObject/ConstructorHelpers.h"

AGizmoGameMode::AGizmoGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
		PlayerControllerClass = AGizmoPlayerController::StaticClass();

	}
}
