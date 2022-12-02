// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableWidgetComponent.h"

void UInteractableWidgetComponent::UpdateInteractableWidget(UInteractableComponent* InteractionComponent)
{
	OwningInteractionComponent = InteractionComponent;
	OnUpdateInteractionWidget();
}
