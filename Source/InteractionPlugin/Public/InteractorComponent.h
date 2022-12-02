// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractorComponent.generated.h"


UCLASS( ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent) )
class INTERACTIONPLUGIN_API UInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UInteractorComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// API

public:

	/* On press Interact Input */
	UFUNCTION(BlueprintCallable)
	void BeginInteract();

	/* On release Interact Input */
	UFUNCTION(BlueprintCallable)
	void EndInteract();

	/* Change if can Interact */
	UFUNCTION(BlueprintCallable)
	void SetCanInteract(bool Value);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetCanInteract() const { return bCanInteract; }

	FORCEINLINE class UInteractableComponent* GetInteractable() const { return ViewedInteractionComponent; }

	bool IsInteracting() const;
	float GetRemainingInteractTime() const;

	// Config

protected:

	UPROPERTY(EditDefaultsOnly)
	float InteractionCheckFrequency;

	UPROPERTY(EditDefaultsOnly)
	float InteractionCheckDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_CanInteract)
	bool bCanInteract;

	UFUNCTION()
	void OnRep_CanInteract();

	// Internal Use

protected:

	void Interact();

	UPROPERTY(BlueprintReadOnly)
	class UInteractableComponent* ViewedInteractionComponent;

	UPROPERTY(BlueprintReadOnly)
	float LastInteractionCheckTime;

	UPROPERTY(BlueprintReadOnly)
	bool bInteractHeld;

	UPROPERTY(BlueprintReadOnly)
	AActor* CurrentInteractingActor;

	UFUNCTION(BlueprintImplementableEvent)
	void OnFindNewInteractingActor(AActor* NewInteractingActor);

private:

	UPROPERTY()
	APawn* OwnerPawn;

	/* Called from Tick for checking any close Interactable */
	void PerformInteractionCheck();

	void CouldntFindInteractable();
	void FoundNewInteractable(AActor* Actor, class UInteractableComponent* Interactable);

	/* Replicates the BeginInteract() */
	UFUNCTION(Server, Reliable)
	void ServerBeginInteract();

	/* Replicates the EndInteract() */
	UFUNCTION(Server, Reliable)
	void ServerEndInteract();

	/* Replicates the SetCanInteract(bool) */
	UFUNCTION(Server, Reliable)
	void ServerSetCanInteract(bool Value);

	FTimerHandle TimerHandle_Interact;

	UPROPERTY()
	UInputComponent* InputComponent;

};
