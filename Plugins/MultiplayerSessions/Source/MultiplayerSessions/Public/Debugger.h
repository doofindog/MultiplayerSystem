// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Debugger.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UDebugger : public UObject
{
	GENERATED_BODY()

private:
	static void PrintScreen(int key, float time, FColor color, FString text);

public:
	static void PrintScreenLog(FString text);
	static void PrintScreenError(FString text);
	static void PrintScreenSuccess(FString text);
};
