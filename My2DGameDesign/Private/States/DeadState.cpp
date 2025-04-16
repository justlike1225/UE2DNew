
#include "States/DeadState.h"
#include "Interfaces/Context/HeroStateContext.h" 
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h" 

void UDeadState::OnEnterState_Implementation()
{
	Super::OnEnterState_Implementation();
	

	
	if (AnimListener)
	{
        
        
		AnimListener->Execute_OnDeathState(AnimListener.GetObject(), nullptr); 
	}
}

