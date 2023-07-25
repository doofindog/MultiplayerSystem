// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MultiplayerSessionSubsystem.generated.h"


//
// Declaring our own custom delegate for the menu clas to bind callbacks to!
// DYNAMIC - The macro is used declare a dynamic Delegate which can be bound to both C++ and Blueprint functions, They are slower because they use reflection to allow binding to blueprint functions.
// MULTICAST - The macro allows for multiple functions to be bound to the same delegate instance.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, isSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionComplete, const TArray<FOnlineSessionSearchResult> &sessionResult, bool isSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, isSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, isSuccessful);

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
#pragma region 
private:
	IOnlineSessionPtr sessionInterface;
	TSharedPtr<FOnlineSessionSettings> sessionSettings;
	TSharedPtr<FOnlineSessionSearch> sessionSearch;
	
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate OnFindSessionCompleteDelegate;
	FDelegateHandle FindSessionCompleteDelegateHandle;

	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	
public:
	//
	// Our own Custom Delegate for the Menu class to bind callbacks to
	//
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionComplete MultiplayerOnFindSessionComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;
	
	
#pragma endregion 

public:
	UMultiplayerSessionSubsystem();


	//
	//	To Handle session functionality, The Menu class will call these
	//
	void CreateSession(int maxPlayers, FString matchType);
	void FindSession(int32 maxSearchResult);
	void JoinSession(const FOnlineSessionSearchResult& sessionResult);
	void StartSession();
	void DestroySession();



protected:
	void OnCreateSessionComplete(FName name, bool isSuccessful);
	void OnFindSessionComplete(bool isSuccessful);
	void OnJoinSessionComplete(FName name, EOnJoinSessionCompleteResult::Type type);
	void OnStartSessionComplete(FName name, bool isSuccessful);
	void OnDestroySessionComplete(FName name, bool isSuccessful);
};
