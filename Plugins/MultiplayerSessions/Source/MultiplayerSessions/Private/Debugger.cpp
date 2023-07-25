// Fill out your copyright notice in the Description page of Project Settings.


#include "Debugger.h"

const int KEY = -1;
const float DISPLAY_TIME = 15.0f;
const FColor LOG_COLOR = FColor::Blue;
const FColor ERROR_COLOR = FColor::Red;
const FColor SUCCESS_COLOR = FColor::Green;

void UDebugger::PrintScreen(int key, float time, FColor color, FString text)
{
	if (!GEngine) { return; }

	GEngine->AddOnScreenDebugMessage(key, time, color, text);
}

void UDebugger::PrintScreenLog(FString text)
{
	PrintScreen(KEY, DISPLAY_TIME, LOG_COLOR, text);
}

void UDebugger::PrintScreenError(FString text)
{
	PrintScreen(KEY, DISPLAY_TIME, ERROR_COLOR, text);
}

void UDebugger::PrintScreenSuccess(FString text)
{
	PrintScreen(KEY, DISPLAY_TIME, SUCCESS_COLOR, text);
}
