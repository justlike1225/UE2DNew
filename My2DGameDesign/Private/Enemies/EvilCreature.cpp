#include "Enemies/EvilCreature.h"
#include "Components/EnemyMeleeAttackComponent.h"
#include "Components/TeleportComponent.h"
#include "Components/CapsuleComponent.h"
#include "PaperFlipbookComponent.h"

AEvilCreature::AEvilCreature()
{
	MeleeAttackComponent = CreateDefaultSubobject<UEnemyMeleeAttackComponent>(TEXT("MeleeAttackComponent"));
	TeleportComponent = CreateDefaultSubobject<UTeleportComponent>(TEXT("TeleportComponent"));

	MeleeHit1 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MeleeHit1"));
	if (MeleeHit1)
	{
		if (GetSprite())
		{
			MeleeHit1->SetupAttachment(GetSprite());
		}
		else
		{
			MeleeHit1->SetupAttachment(RootComponent);
		}

		MeleeHit1->SetRelativeLocation(FVector(-14.0f, 0.0f, 19.0f));
		MeleeHit1->SetCapsuleHalfHeight(30.0f);
		MeleeHit1->SetCapsuleRadius(8.0f);
		MeleeHit1->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
		MeleeHit1->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeleeHit1->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
		MeleeHit1->SetCollisionResponseToAllChannels(ECR_Ignore);
		MeleeHit1->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		MeleeHit1->CanCharacterStepUpOn = ECB_No;
		MeleeHit1->SetGenerateOverlapEvents(true);
	}
}

UPrimitiveComponent* AEvilCreature::GetMeleeShapeComponent_Implementation(FName ShapeIdentifier) const
{
	if (ShapeIdentifier == EvilCreatureAttackShapeNames::Melee1)
	{
		return MeleeHit1;
	}
	return nullptr;
}


void AEvilCreature::BeginPlay()
{
	Super::BeginPlay();
	if (MeleeHit1 && GetSprite() && MeleeHit1->GetAttachParent() != GetSprite())
	{
		MeleeHit1->AttachToComponent(GetSprite(), FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (MeleeHit1)
	{
		MeleeHit1->IgnoreActorWhenMoving(this, true);
	}
}
