#include "Actors/DamageNumberActor.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h" 
#include "UObject/ConstructorHelpers.h" 

ADamageNumberActor::ADamageNumberActor()
{
    PrimaryActorTick.bCanEverTick = false; 

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    DamageWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidgetComp"));
    if (DamageWidgetComp)
    {
        DamageWidgetComp->SetupAttachment(RootComponent);
        DamageWidgetComp->SetWidgetSpace(EWidgetSpace::Screen); 
        DamageWidgetComp->SetDrawAtDesiredSize(true); 
        DamageWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    InitialLifeSpan = LifeSpan; 
}

void ADamageNumberActor::BeginPlay()
{
    Super::BeginPlay();

    
    if (DamageWidgetClass)
    {
        DamageWidgetComp->SetWidgetClass(DamageWidgetClass);
    }
  
    else 
    {
        UE_LOG(LogTemp, Warning, TEXT("DamageWidgetClass is not valid!"));
    }
    
}

void ADamageNumberActor::SetDamageText(int32 DamageAmount,FColor TextColor )
{
    if (!DamageWidgetComp) return;

   
    UUserWidget* Widget = DamageWidgetComp->GetUserWidgetObject();
    if (Widget)
    {
       
        UTextBlock* TextBlock = Cast<UTextBlock>(Widget->GetWidgetFromName(FName("DamageText")));
        if (TextBlock)
        {
            TextBlock->SetText(FText::AsNumber(DamageAmount));
            TextBlock->SetColorAndOpacity(FSlateColor(TextColor));
        }
     
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DamageWidgetComp is not valid!"));
    }
   
}