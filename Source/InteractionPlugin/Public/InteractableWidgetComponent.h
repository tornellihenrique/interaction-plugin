// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractableWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class INTERACTIONPLUGIN_API UInteractableWidgetComponent : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdateInteractableWidget(class UInteractableComponent* InteractionComponent);

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateInteractionWidget();

	UPROPERTY(BlueprintReadOnly, Category = "Interaction", meta = (ExposeOnSpawn))
	class UInteractableComponent* OwningInteractionComponent;
	
};
