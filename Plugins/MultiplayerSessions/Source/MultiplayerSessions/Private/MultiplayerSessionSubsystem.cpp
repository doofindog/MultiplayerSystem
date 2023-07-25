// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Debugger.h"

UMultiplayerSessionSubsystem::UMultiplayerSessionSubsystem()
{
	sessionInterface = IOnlineSubsystem::Get()->GetSessionInterface();
	
	UDebugger::PrintScreenLog(FString::Printf(TEXT("online Session Interface: %s"), *IOnlineSubsystem::Get()->GetSubsystemName().ToString()));
	
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionSubsystem::OnCreateSessionComplete);
	OnFindSessionCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete);
}

void UMultiplayerSessionSubsystem::CreateSession(int maxPublicConnections, FString matchType)
{
	UDebugger::PrintScreenLog("[multiplayer System] Trying to Host/Create Session");
	UDebugger::PrintScreenLog(FString::Printf(TEXT("maxConnections : %d"), maxPublicConnections));

	if (!sessionInterface.IsValid())
	{
		return;
	}

	auto existingSession = sessionInterface->GetNamedSession(NAME_GameSession);
	if (existingSession != nullptr)
	{
		sessionInterface->DestroySession(NAME_GameSession);
	}

	sessionSettings = MakeShareable(new FOnlineSessionSettings());
	sessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false; // if Subsystem name is Null We Choose it to be a lan connection
	sessionSettings->NumPublicConnections = maxPublicConnections;
	sessionSettings->bAllowJoinInProgress = true; //Allows players to join while session is in progress
	sessionSettings->bAllowJoinViaPresence = true; //for steam it uses something called presence where it looks for session in our region , we need this in order for our session to work,
	sessionSettings->bShouldAdvertise = true;
	sessionSettings->bUsesPresence = true;
	sessionSettings->bUseLobbiesIfAvailable = true;
	sessionSettings->Set(FName("matchType"), matchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	CreateSessionCompleteDelegateHandle = sessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!sessionInterface->CreateSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *sessionSettings))
	{
		UDebugger::PrintScreenError("[Multiplayer System] Failed to Create Session");

		sessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionSubsystem::FindSession(int32 maxSearchResult = 10000)
{
	UDebugger::PrintScreenLog("[multiplayer System] Trying to Join Session");

	if (!sessionInterface || !sessionInterface.IsValid())
	{
		UDebugger::PrintScreenLog("[multiplayer System] Session Interface null or not found");
		return;
	}
	
	sessionSearch = MakeShareable(new FOnlineSessionSearch());
	sessionSearch->MaxSearchResults = maxSearchResult;
	sessionSearch->bIsLanQuery = false; //Weather the Quary is Intended for lan matches or not.
	sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true,EOnlineComparisonOp::Equals);

	FindSessionCompleteDelegateHandle = sessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionCompleteDelegate);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if(!sessionInterface->FindSessions(*localPlayer->GetPreferredUniqueNetId(), sessionSearch.ToSharedRef()))
	{
		sessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
		MultiplayerOnFindSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(),false);
	}
}

void UMultiplayerSessionSubsystem::JoinSession(const FOnlineSessionSearchResult& sessionResult)
{
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	
	if(!sessionInterface->JoinSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, sessionResult))
	{
		
	}
}

void UMultiplayerSessionSubsystem::StartSession()
{
}

void UMultiplayerSessionSubsystem::DestroySession()
{
}


#pragma region //--------->CallBacks<---------//

void UMultiplayerSessionSubsystem::OnCreateSessionComplete(FName name, bool isSuccessful)
{
	if(isSuccessful)
	{
		UDebugger::PrintScreenSuccess("Session Created Successful");
	}
	
	if (sessionInterface.IsValid())
	{
		sessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
	
	MultiplayerOnCreateSessionComplete.Broadcast(isSuccessful);
}

void UMultiplayerSessionSubsystem::OnFindSessionComplete(bool isSuccessful)
{
	if(! isSuccessful) return;

	if(sessionInterface.IsValid())
	{
		sessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
	}
	
	for(FOnlineSessionSearchResult result : sessionSearch->SearchResults)
	{
		FString id = result.GetSessionIdStr();
		FString user = result.Session.OwningUserName;
		FString matchType;
		TSharedPtr<FOnlineSessionInfo> sessionInfo =  result.Session.SessionInfo;
		result.Session.SessionSettings.Get(FName("matchType"), matchType);
	}
}

void UMultiplayerSessionSubsystem::OnJoinSessionComplete(FName name, EOnJoinSessionCompleteResult::Type type)
{
}

void UMultiplayerSessionSubsystem::OnStartSessionComplete(FName name, bool isSuccessful)
{
}

void UMultiplayerSessionSubsystem::OnDestroySessionComplete(FName name, bool isSuccessful)
{
}

#pragma endregion


