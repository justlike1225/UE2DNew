#include "MyGameHUD.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "Blueprint/UserWidget.h" 
#include "Components/RageComponent.h"
#include "GameFramework/PlayerController.h" 
#include "Interfaces/UI/RageBarWidgetInterface.h"
class URageComponent;
void AMyGameHUD::HandleRageChanged(float CurrentRage, float MaxRage)
{
	if (CurrentPlayerHUDWidget && CurrentPlayerHUDWidget->GetClass()->ImplementsInterface(URageBarWidgetInterface::StaticClass()))
	{
		IRageBarWidgetInterface::Execute_UpdateRage(CurrentPlayerHUDWidget, CurrentRage, MaxRage);
	}
}
void AMyGameHUD::BeginPlay()
{
	Super::BeginPlay();
	if (PlayerHUDWidgetClass)
	{
		APlayerController* PC = GetOwningPlayerController();
		if (PC)
		{
			CurrentPlayerHUDWidget = CreateWidget<UUserWidget>(PC, PlayerHUDWidgetClass);
			if (CurrentPlayerHUDWidget)
			{
				CurrentPlayerHUDWidget->AddToViewport();
			}
			APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(PC->GetPawn());
			URageComponent* RageComp = Hero ? Hero->GetRageComponent() : nullptr; 
			if (RageComp && CurrentPlayerHUDWidget->GetClass()->ImplementsInterface(URageBarWidgetInterface::StaticClass()))
			{
				RageComp->OnRageChanged.AddDynamic(this, &AMyGameHUD::HandleRageChanged);
				HandleRageChanged(RageComp->GetCurrentRage(), RageComp->GetMaxRage());
			}
			else
			{
				if (!RageComp) UE_LOG(LogTemp, Warning, TEXT("AMyGameHUD::BeginPlay - Could not find RageComponent on Player Pawn."));
				if (!CurrentPlayerHUDWidget->GetClass()->ImplementsInterface(URageBarWidgetInterface::StaticClass())) UE_LOG(LogTemp, Warning, TEXT("AMyGameHUD::BeginPlay - Player HUD Widget does not implement IRageBarWidgetInterface."));
			}
		}
	}
}