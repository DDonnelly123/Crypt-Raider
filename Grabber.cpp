// Fill out your copyright notice in the Description page of Project Settings.


#include "Grabber.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UPhysicsHandleComponent* PhysicsHandle = GetPhysicsHandle();
	
	if ((PhysicsHandle != nullptr) && (PhysicsHandle->GetGrabbedComponent()))
	{
		FVector TargetLocation = GetComponentLocation() + (GetForwardVector() * HoldDistance);
		PhysicsHandle->SetTargetLocationAndRotation(TargetLocation, GetComponentRotation());
	}
}



void UGrabber::Release()
{
	UPhysicsHandleComponent* PhysicsHandle = GetPhysicsHandle();

	if ((PhysicsHandle != nullptr) && (PhysicsHandle->GetGrabbedComponent()))
	{
		PhysicsHandle->GetGrabbedComponent()->WakeAllRigidBodies();
		AActor* TargetActor = PhysicsHandle->GetGrabbedComponent()->GetOwner();
		TargetActor->Tags.Remove("Grabbed");
		PhysicsHandle->ReleaseComponent();
	}
}



void UGrabber::Grab()
{
	UPhysicsHandleComponent* PhysicsHandle = GetPhysicsHandle();
	if (PhysicsHandle == nullptr)
	{
		return;
	}
	
	FHitResult HitResult;
	
	if (GetGrabbableInReach(HitResult))
	{
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		HitComponent->WakeAllRigidBodies();
		HitComponent->SetSimulatePhysics(true);
		AActor* TargetActor = HitResult.GetActor();
		TargetActor->Tags.Add("Grabbed");
		TargetActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		PhysicsHandle->GrabComponentAtLocationWithRotation(
			HitComponent,
			NAME_None,
			HitResult.ImpactPoint,
			GetComponentRotation()
			);
	} 
}



UPhysicsHandleComponent* UGrabber::GetPhysicsHandle() const
{
	// Get the physics handle of the owner (Grabber)
	UPhysicsHandleComponent* Result = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (Result == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No physics handle found"));
	}

	return Result;
}



bool UGrabber::GetGrabbableInReach(FHitResult &OutHitResult) const
{
	// Get start and end vector to create sweeping line
	FVector Start = GetComponentLocation();
	FVector End =  Start + (GetForwardVector() * MaxGrabDistance);
	DrawDebugLine(GetWorld(),Start,End,FColor::Red);

	// Set radius of spherical line
	FCollisionShape Sphere = FCollisionShape::MakeSphere(GrabRadius);

	// Check if object collides with line
	return GetWorld()->SweepSingleByChannel(
		OutHitResult,
		Start, End,
		FQuat::Identity,
		ECC_GameTraceChannel2,
		Sphere
		);
}



