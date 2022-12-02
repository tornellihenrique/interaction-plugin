// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractorComponent.h"

#include "InteractableComponent.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

UInteractorComponent::UInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	InteractionCheckFrequency = 0.f;
	InteractionCheckDistance = 1000.f;
	bCanInteract = true;

	SetIsReplicatedByDefault(true);
}

void UInteractorComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPawn = Cast<APawn>(GetOwner());
}

void UInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetWorld()->TimeSince(LastInteractionCheckTime) > InteractionCheckFrequency)
	{
		PerformInteractionCheck();
	}
}

void UInteractorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bCanInteract);
}

void UInteractorComponent::BeginInteract()
{
	if (!OwnerPawn) return;

	if (!OwnerPawn->HasAuthority())
	{
		ServerBeginInteract();
	}

	if (OwnerPawn->HasAuthority())
	{
		PerformInteractionCheck();
	}

	bInteractHeld = true;

	if (UInteractableComponent* Interactable = GetInteractable())
	{
		Interactable->BeginInteract(OwnerPawn);

		if (FMath::IsNearlyZero(Interactable->GetInteractionTime()))
		{
			Interact();
		}
		else
		{
			OwnerPawn->GetWorldTimerManager().SetTimer(TimerHandle_Interact, this, &ThisClass::Interact, Interactable->GetInteractionTime(), false);
		}
	}
}

void UInteractorComponent::EndInteract()
{
	if (!OwnerPawn) return;

	if (!OwnerPawn->HasAuthority())
	{
		ServerEndInteract();
	}

	bInteractHeld = false;

	OwnerPawn->GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UInteractableComponent* Interactable = GetInteractable())
	{
		Interactable->EndInteract(OwnerPawn);
	}
}

void UInteractorComponent::SetCanInteract(bool Value)
{
	if (!OwnerPawn) return;

	if (OwnerPawn->HasAuthority())
	{
		bCanInteract = Value;

		OnRep_CanInteract();
	}
	else
	{
		ServerSetCanInteract(Value);
	}
}

bool UInteractorComponent::IsInteracting() const
{
	if (!OwnerPawn) return false;

	return OwnerPawn->GetWorldTimerManager().IsTimerActive(TimerHandle_Interact);
}

float UInteractorComponent::GetRemainingInteractTime() const
{
	if (!OwnerPawn) return false;

	return OwnerPawn->GetWorldTimerManager().GetTimerRemaining(TimerHandle_Interact);
}

void UInteractorComponent::OnRep_CanInteract()
{
	if (!bCanInteract)
	{
		CouldntFindInteractable();
	}
}

void UInteractorComponent::Interact()
{
	if (!OwnerPawn) return;

	OwnerPawn->GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UInteractableComponent* Interactable = GetInteractable())
	{
		Interactable->Interact(OwnerPawn);
	}
}

void UInteractorComponent::PerformInteractionCheck()
{
	if (!OwnerPawn) return;

	AController* Controller = OwnerPawn->GetController();
	if (!Controller) return;

	LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	FVector EyesLoc;
	FRotator EyesRot;

	Controller->GetPlayerViewPoint(EyesLoc, EyesRot);

	FVector TraceStart = EyesLoc;
	FVector TraceEnd = (EyesRot.Vector() * InteractionCheckDistance) + TraceStart;
	FHitResult TraceHit;

	FCollisionQueryParams QueryParams;

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(OwnerPawn);

	QueryParams.AddIgnoredActors(IgnoredActors);

	if (bCanInteract && GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECC_Camera, QueryParams))
	{
		if (AActor* FoundActor = TraceHit.GetActor())
		{
			if (UInteractableComponent* InteractionComponent = Cast<UInteractableComponent>(FoundActor->GetComponentByClass(UInteractableComponent::StaticClass())))
			{
				float Distance = (TraceStart - TraceHit.ImpactPoint).Size();

				if (InteractionComponent != GetInteractable() && Distance <= InteractionComponent->GetInteractionDistance())
				{
					FoundNewInteractable(FoundActor, InteractionComponent);
				}
				else if (Distance > InteractionComponent->GetInteractionDistance() && GetInteractable())
				{
					CouldntFindInteractable();
				}

				return;
			}
		}
	}

	CouldntFindInteractable();
}

void UInteractorComponent::CouldntFindInteractable()
{
	if (!OwnerPawn) return;

	if (OwnerPawn->GetWorldTimerManager().IsTimerActive(TimerHandle_Interact))
	{
		OwnerPawn->GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	}

	if (UInteractableComponent* Interactable = GetInteractable())
	{
		Interactable->EndFocus(OwnerPawn);

		if (bInteractHeld)
		{
			EndInteract();
		}
	}

	ViewedInteractionComponent = nullptr;

	OnFindNewInteractingActor(nullptr);
	CurrentInteractingActor = nullptr;
}

void UInteractorComponent::FoundNewInteractable(AActor* Actor, UInteractableComponent* Interactable)
{
	if (!OwnerPawn) return;

	EndInteract();

	if (UInteractableComponent* OldInteractable = GetInteractable())
	{
		OldInteractable->EndFocus(OwnerPawn);
	}

	ViewedInteractionComponent = Interactable;
	Interactable->BeginFocus(OwnerPawn);

	OnFindNewInteractingActor(Actor);
	CurrentInteractingActor = Actor;
}

void UInteractorComponent::ServerSetCanInteract_Implementation(bool Value)
{
	SetCanInteract(Value);
}

void UInteractorComponent::ServerEndInteract_Implementation()
{
	EndInteract();
}

void UInteractorComponent::ServerBeginInteract_Implementation()
{
	BeginInteract();
}
