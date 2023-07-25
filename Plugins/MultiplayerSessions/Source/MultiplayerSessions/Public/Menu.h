// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

private:
	int32 _maxPublicConnections;
	FString _matchType;
	
public:

	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 maxPublicConnections, FString matchType);

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	//
	// CallBacks for the custom delegates on the MultiplayerSessionSubsystem
	//
	UFUNCTION() //We need to call UFunction else it will not successfully be bound to the delegate
	void OnCreateSession(bool isSuccessful);
	void OnFindSession( const TArray<FOnlineSessionSearchResult> &sessionResult, bool isSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type type);
	void OnDestroySession(bool isSuccessful);
	void OnStartSession(bool isSuccessful);
	

private:
	UPROPERTY(meta = (BindWidget))
		class UButton* hostButton;
	
	UPROPERTY(meta = (BindWidget))
		UButton* joinButton;

	UFUNCTION()
		void HostButtonClicked();

	UFUNCTION()
		void JoinButtonClicked();

	UFUNCTION()
	void MenuTearDown();
	
	class UMultiplayerSessionSubsystem* multiplayerSessionSubsystem;
};
