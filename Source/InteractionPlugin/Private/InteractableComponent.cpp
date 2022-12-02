// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableComponent.h"

#include "InteractableWidgetComponent.h"
#include "InteractorComponent.h"

UInteractableComponent::UInteractableComponent()
{
	InteractionTime = 0.f;
	InteractionDistance = 500.f;
	InteractableNameText = FText::FromString("Interactable Object");
	InteractableActionText = FText::FromString("Interact");
	bAllowMultipleInteractors = true;

	Space = EWidgetSpace::Screen;
	DrawSize = FIntPoint(600, 100);
	bDrawAtDesiredSize = true;

	SetHiddenInGame(true);
	SetActive(true);
}

void UInteractableComponent::RefreshWidget()
{
	if (UInteractableWidgetComponent* InteractionWidget = Cast<UInteractableWidgetComponent>(GetUserWidgetObject()))
	{
		InteractionWidget->UpdateInteractableWidget(this);
	}
}

void UInteractableComponent::BeginFocus(APawn* Pawn)
{
	if (!IsActive() || !GetOwner() || !Pawn)
	{
		return;
	}

	OnBeginFocus.Broadcast(Pawn);

	if (GetNetMode() != NM_DedicatedServer)
	{
		SetHiddenInGame(false);

		TArray<UActorComponent*> FoundComponents;
		GetOwner()->GetComponents(UPrimitiveComponent::StaticClass(), FoundComponents);

		for (auto& FoundComponent : FoundComponents)
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(FoundComponent))
			{
				Prim->SetRenderCustomDepth(true);
			}
		}
	}

	RefreshWidget();
}

void UInteractableComponent::EndFocus(APawn* Pawn)
{
	OnEndFocus.Broadcast(Pawn);

	if (GetNetMode() != NM_DedicatedServer)
	{
		SetHiddenInGame(true);

		TArray<UActorComponent*> FoundComponents;
		GetOwner()->GetComponents(UPrimitiveComponent::StaticClass(), FoundComponents);

		for (auto& FoundComponent : FoundComponents)
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(FoundComponent))
			{
				Prim->SetRenderCustomDepth(false);
			}
		}
	}
}

void UInteractableComponent::BeginInteract(APawn* Pawn)
{
	if (CanInteract(Pawn))
	{
		Interactors.AddUnique(Pawn);
		OnEndInteract.Broadcast(Pawn);
	}
}

void UInteractableComponent::EndInteract(APawn* Pawn)
{
	Interactors.RemoveSingle(Pawn);
	OnEndInteract.Broadcast(Pawn);
}

void UInteractableComponent::Interact(APawn* Pawn) const
{
	if (CanInteract(Pawn))
	{
		OnInteract.Broadcast(Pawn);
	}
}

float UInteractableComponent::GetInteractPercentage()
{
	if (Interactors.IsValidIndex(0))
		if (APawn* Pawn = Interactors[0])
			if (UInteractorComponent* Interactor = Cast<UInteractorComponent>(Pawn->GetComponentByClass(UInteractorComponent::StaticClass())))
				if (Interactor && Interactor->IsInteracting())
					return 1.f - FMath::Abs(Interactor->GetRemainingInteractTime() / InteractionTime);

	return 0.f;
}

void UInteractableComponent::SetInteractableNameText(const FText& NewNameText)
{
	InteractableNameText = NewNameText;
	RefreshWidget();
}

void UInteractableComponent::SetInteractableActionText(const FText& NewActionText)
{
	InteractableActionText = NewActionText;
	RefreshWidget();
}

void UInteractableComponent::Deactivate()
{
	Super::Deactivate();

	for (int32 i = Interactors.Num() - 1; i >= 0; --i)
	{
		if (APawn* Interactor = Interactors[i])
		{
			EndFocus(Interactor);
			EndInteract(Interactor);
		}
	}

	Interactors.Empty();
}

bool UInteractableComponent::CanInteract(const APawn* Pawn) const
{
	const bool bPlayerAlreadyInteracting = !bAllowMultipleInteractors && Interactors.Num() >= 1;
	return !bPlayerAlreadyInteracting && IsActive() && GetOwner() && Pawn;
}
