// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "InteractableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginInteract, class APawn*, Pawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndInteract, class APawn*, Pawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginFocus, class APawn*, Pawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndFocus, class APawn*, Pawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, class APawn*, Pawn);

UCLASS( ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent) )
class INTERACTIONPLUGIN_API UInteractableComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:

	UInteractableComponent();

	// API

public:

	/***Refresh the interaction widget and its custom widgets.
	An example of when we'd use this is when we take 3 items out of a stack of 10, and we need to update the widget
	so it shows the stack as having 7 items left. */
	void RefreshWidget();

	//Called on the client when the players interaction check trace begins/ends hitting this item
	void BeginFocus(class APawn* Pawn);
	void EndFocus(class APawn* Pawn);

	//Called on the client when the player begins/ends interaction with the item
	void BeginInteract(class APawn* Pawn);
	void EndInteract(class APawn* Pawn);

	void Interact(class APawn* Pawn) const;

	//Return a value from 0-1 denoting how far through the interact we are. 
	//On server this is the first interactors percentage, on client this is the local interactors percentage
	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetInteractPercentage();

	//Call this to change the name of the interactable. Will also refresh the interaction widget.
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractableNameText(const FText& NewNameText);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractableActionText(const FText& NewActionText);

	//[local + server] Called when the player presses the interact key whilst focusing on this interactable actor
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginInteract OnBeginInteract;

	//[local + server] Called when the player releases the interact key, stops looking at the interactable actor, or gets too far away after starting an interact
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndInteract OnEndInteract;

	//[local + server] Called when the player presses the interact key whilst focusing on this interactable actor
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginFocus OnBeginFocus;

	//[local + server] Called when the player releases the interact key, stops looking at the interactable actor, or gets too far away after starting an interact
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndFocus OnEndFocus;

	//[local + server] Called when the player has interacted with the item for the required amount of time
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginInteract OnInteract;

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetInteractionTime() const { return InteractionTime; };

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetInteractionDistance() const { return InteractionDistance; };

	UFUNCTION(BlueprintPure)
	FORCEINLINE FText GetInteractableNameText() const { return InteractableNameText; };

	UFUNCTION(BlueprintPure)
	FORCEINLINE FText GetInteractableActionText() const { return InteractableActionText; };

	// Config

public:

	//The time the player must hold the interact key to interact with this object
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionTime;

	//The max distance the player can be away from this actor before you can interact
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionDistance;

	//The name that will come up when the player looks at the interactable
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText InteractableNameText;

	//The verb that describes how the interaction works, ie "Sit" for a chair, "Eat" for food, "Light" for a fireplace
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText InteractableActionText;

	//Whether we allow multiple players to interact with the item, or just one at any given time.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bAllowMultipleInteractors;

	// Internal

protected:

	// Called when the game starts
	virtual void Deactivate() override;

	bool CanInteract(const APawn* Pawn) const;

	//On the server, this will hold all interactors. On the local player, this will just hold the local player (provided they are an interactor)
	UPROPERTY()
	TArray<class APawn*> Interactors;

};
