// Fill out your copyright notice in the Description page of Project Settings.

#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionSubsystem.h"
#include "Debugger.h"


const int MAX_SEARCH = 10000;

void UMenu::MenuSetup(int32 maxPublicConnections, FString matchType)
{
	UDebugger::PrintScreenSuccess("Menu Initialised");
	_matchType = matchType;
	_maxPublicConnections = maxPublicConnections;
	
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* world = GetWorld();
	if (world)
	{
		APlayerController* playerController = world->GetFirstPlayerController();
		if (playerController)
		{
			FInputModeUIOnly inputMode;  //Input for UI Mode
			inputMode.SetWidgetToFocus(TakeWidget());
			inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			playerController->SetInputMode(inputMode);
			playerController->SetShowMouseCursor(true);
		}
	}

	multiplayerSessionSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSessionSubsystem>();
	if (multiplayerSessionSubsystem)
	{
		multiplayerSessionSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		multiplayerSessionSubsystem->MultiplayerOnFindSessionComplete.AddUObject(this, &ThisClass::OnFindSession);
		multiplayerSessionSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		multiplayerSessionSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		multiplayerSessionSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}


bool UMenu::Initialize()
{
	if (!Super::Initialize()) { return false; }


	//Note :- We don't Set it up in the constructor because it will be too early to set this up for in the Widget
	if (hostButton) { hostButton->OnClicked.AddDynamic(this, &UMenu::HostButtonClicked); }
	if (joinButton) { joinButton->OnClicked.AddDynamic(this, &UMenu::JoinButtonClicked); }

	return true;
}

void UMenu::NativeDestruct()
{
	MenuTearDown();

	Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool isSuccessful)
{
	if(!isSuccessful)
	{
		UDebugger::PrintScreenLog("[Multiplayer Session] Session Not created successfully");
		return;
	}
	
	UDebugger::PrintScreenLog("[Multiplayer Session] OnCreateSession call in Menu.cpp");
	
	UWorld* world = GetWorld();
	if(world)
	{
		world->ServerTravel(FString("/Game/Maps/Lobby?listen"));
	}
}

void UMenu::OnFindSession(const TArray<FOnlineSessionSearchResult>& sessionResults, bool isSuccessful)
{
	if(!isSuccessful)
	{
		UDebugger::PrintScreenError("[Multiplayer Session] Find Session Not Successful");
		return;
	}

	for(FOnlineSessionSearchResult result : sessionResults)
	{
		FString sessionType;
		result.Session.SessionSettings.Get(FName("MatchType"),sessionType);
		if(_matchType == sessionType)
		{
			UDebugger::PrintScreenLog("[Menu] Find Result by : " + result.Session.OwningUserName);
			//multiplayerSessionSubsystem->JoinSession(result);
		}
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type type)
{
	if(multiplayerSessionSubsystem == nullptr)
	{
		UDebugger::PrintScreenError("[Menu System] No MultiplayerSessionSubsystem is null" );
		return;
	}
}

void UMenu::OnDestroySession(bool isSuccessful)
{
	
}

void UMenu::OnStartSession(bool isSuccessful)
{
	
}


void UMenu::HostButtonClicked()
{
	if (!multiplayerSessionSubsystem) 
	{ 
		return; 
	}

	UDebugger::PrintScreenLog(FString::Printf(TEXT("[Menu] Host Button Pressed")));
	multiplayerSessionSubsystem->CreateSession(_maxPublicConnections, _matchType);
}

void UMenu::JoinButtonClicked()
{
	if (!multiplayerSessionSubsystem)
	{
		return;
	}

	UDebugger::PrintScreenLog(FString::Printf(TEXT("[Menu] Join Button Pressed")));
	multiplayerSessionSubsystem->FindSession(MAX_SEARCH);
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* world = GetWorld();
	if (world)
	{
		APlayerController* playerController = world->GetFirstPlayerController();
		if (playerController)
		{
			FInputModeGameOnly inputMode; //Input for Game Mode
			playerController->SetInputMode(inputMode);
			playerController->SetShowMouseCursor(false);
		}
	}
}
