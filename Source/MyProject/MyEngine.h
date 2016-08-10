#pragma once
#include "Engine.h"
#include "MyEngine.generated.h"

UClass(Config=Engine)
class UMyEngine : public UGameEngine
{
	GENERATED_BODY()

public:
	UFunction(Config)
	void UGameEngine::Init(IEngineLoop * InEngineLoop) {
		DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGameEngine Init"), STAT_GameEngineStartup, STATGROUP_LoadTime);
		DECLARE_LOG_CATEGORY_EXTERN(MyLogCategory, Log, All);
		DEFINE_LOG_CATEGORY(MyLogCategory);
		UE_LOG(MyLogCategory, Log, TEXT("This is my message that prints to the log. It works like printf"));

		// Call base.
		UGameEngine::Init(InEngineLoop);
		// Creates the initial world context. For GameEngine, this should be the only WorldContext that ever gets created.
		FWorldContext &InitialWorldContext = CreateNewWorldContext(EWorldType::Game);
		UE_LOG(LogTemp, Warning, TEXT("INIT"));
		UE_LOG(LogInit, Display, TEXT("KAJSHDKJASHDKJASHDKJASHDKAJSHDKAHS."));
	}
	
};
