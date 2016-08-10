// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ModuleManager.h"
#include "Core.h"
#include "Engine.h"

#include "ShoeState.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EFootEnum : uint8
{
	F_Right	UMETA(DisplayName = "Right"),
	F_Left 	UMETA(DisplayName = "Left")
};

typedef void(*callback)();

UCLASS(Blueprintable)
class UShoeState : public UObject
{
	GENERATED_UCLASS_BODY()

	static UShoeState* staticInst;
	int32 heights[14];
	int32 pressures[14];
	uint8 states[2];
public:
	UShoeState();
	~UShoeState();

	UFUNCTION(BlueprintCallable, Category = "Shoe State")
	static UShoeState* Init();

	UFUNCTION(BlueprintCallable, Category = "Shoe State")
	int32 getChamberHeight(EFootEnum foot, uint8 chamber);

	UFUNCTION(BlueprintCallable, Category = "Shoe State")
	int32 getChamberPressure(EFootEnum foot, uint8 chamber);

	UFUNCTION(BlueprintCallable, Category = "Shoe State")
	bool getChamberState(EFootEnum foot, uint8 chamber);

	void setChamberHeight(EFootEnum foot, uint8 chamber, int32 height);

	void setChamberPressure(EFootEnum foot, uint8 chamber, int32 pressure);

	void setChamberState(EFootEnum foot, uint8 state);

	void setUpdateCallback(callback updateCallback);

	callback updateCallback;
};
