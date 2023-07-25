// Copyright Epic Games, Inc. All Rights Reserved.

#include "Menu_SystemCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"


//////////////////////////////////////////////////////////////////////////
// AMenu_SystemCharacter

AMenu_SystemCharacter::AMenu_SystemCharacter():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &AMenu_SystemCharacter::OnCreateSessionComplete)), //Adds the Delegate here
	FindSessionCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &AMenu_SystemCharacter::OnFindSessionComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &AMenu_SystemCharacter::OnJoinSessionComplete))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AMenu_SystemCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}


//Session Created when clicking the 1 key
void AMenu_SystemCharacter::CreateGameSession()
{
	if (!onlineSessionInterface.IsValid())
	{
		return; 
	}


	auto existingSession = onlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (existingSession != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Session Already Exist, Destorying existing Session")));
		onlineSessionInterface->DestroySession(NAME_GameSession);
	}

	onlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	//LocalPlayer is just a property of a PlayerController referencing either a local player or a net connection.
	TSharedPtr<FOnlineSessionSettings> sessionSettings = MakeShareable(new FOnlineSessionSettings());
	sessionSettings->bIsLANMatch = false;
	sessionSettings->NumPublicConnections = 4;
	sessionSettings->bAllowJoinInProgress = true; //Allows players to join while session is in progress
	sessionSettings->bAllowJoinViaPresence = true; //for steam it uses something called presence where it looks for session in our region , we need this in order for our session to work,
	sessionSettings->bShouldAdvertise = true;
	sessionSettings->bUsesPresence = true;
	sessionSettings->bUseLobbiesIfAvailable = true;
	sessionSettings->Set(FName("matchType"), FString("FreeForAll"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	onlineSessionInterface->CreateSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *sessionSettings);
}

//Called when the 2 Key is Pressed
void AMenu_SystemCharacter::JoinGameSession()
{
	if (!onlineSessionInterface)
	{
		return;
	}

	onlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegate);

	// Find game session
	sessionSearch = MakeShareable(new FOnlineSessionSearch());
	sessionSearch->MaxSearchResults = 10000;
	sessionSearch->bIsLanQuery = false;
	sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	onlineSessionInterface->FindSessions(*localPlayer->GetPreferredUniqueNetId(), sessionSearch.ToSharedRef());
}

void AMenu_SystemCharacter::OnCreateSessionComplete(FName sessionName, bool isSuccessful) // This is Added near the constructor
{
	if (isSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f, 
				FColor::Green, 
				FString::Printf(TEXT("Session Created"))
			);
		}

		UWorld* world = GetWorld();
		if (world)
		{
			world->ServerTravel(FString("/Game/Maps/Lobby?listen"));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Red,
				FString::Printf(TEXT("Session Not Created"))
			);
		}
	}
}

void AMenu_SystemCharacter::OnFindSessionComplete(bool isSuccessful)
{
	if (!isSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Red,
				FString::Printf(TEXT("Join Session Not Successful"))
			);
		}
		return;
	}

	if (!onlineSessionInterface.IsValid())
	{
		return;
	}

	for (FOnlineSessionSearchResult result : sessionSearch->SearchResults)
	{
		FString id = result.GetSessionIdStr();
		FString user = result.Session.OwningUserName;
		FString matchType;
		result.Session.SessionSettings.Get(FName("matchType"), matchType);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Red,
				FString::Printf(TEXT("Session Found : (id = %s, User = %s)"), *id, *user)
			);
		}

		if (matchType == FString("FreeForAll"))
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.0f,
					FColor::Green,
					FString::Printf(TEXT("Joining Match Type : %s"), *matchType)
				);
			}

			onlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

			const ULocalPlayer* localplayer = GetWorld()->GetFirstLocalPlayerFromController();
			onlineSessionInterface->JoinSession(*localplayer->GetPreferredUniqueNetId(), NAME_GameSession, result);
		}
	}
}

void AMenu_SystemCharacter::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
	if (!onlineSessionInterface.IsValid())
	{
		return;
	}

	FString address;
	if (onlineSessionInterface->GetResolvedConnectString(NAME_GameSession, address))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Yellow,
				FString::Printf(TEXT("Connect String : %s"), *address)
			);
		}

		APlayerController* playerController = GetGameInstance()->GetFirstLocalPlayerController();
		if (playerController)
		{
			playerController->ClientTravel(address, ETravelType::TRAVEL_Absolute);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// Input

void AMenu_SystemCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMenu_SystemCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMenu_SystemCharacter::Look);

	}

}

void AMenu_SystemCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMenu_SystemCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}