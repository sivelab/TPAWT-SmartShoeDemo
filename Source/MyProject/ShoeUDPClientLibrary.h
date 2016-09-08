// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ModuleManager.h"
#include "Core.h"
#include "Engine.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "Networking.h"
#include "ShoeRecvThread.h"
#include "ShoeState.h"
#include <limits.h>

#include "ShoeUDPClientLibrary.generated.h"

// This is the pressure of the chambers when idle without any weight.
#define RESTING_PRESSURE 100000
#define NUM_OF_CHAMBERS 7
#define SHOE_UNITS_PER_CM 100.0
#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White,text)

//General Log
DECLARE_LOG_CATEGORY_EXTERN(ShoeLog, Log, All);

//Logging for output to shoe
DECLARE_LOG_CATEGORY_EXTERN(ShoeOutput, Log, All);

//Logging for input from shoe
DECLARE_LOG_CATEGORY_EXTERN(ShoeInput, Log, All);

/**
 * 
 */

UCLASS(Blueprintable)
class UShoeUDPClientLibrary : public UObject
{
	GENERATED_UCLASS_BODY()

	FSocket* SocketRight;
	FSocket* SocketLeft;
	bool RightConnected;
	bool LeftConnected;
	FShoeRecvThread* recvThread;
	UShoeState* shoeState;

	FIPv4Endpoint LeftEndpoint;
	FIPv4Endpoint RightEndpoint;

	static UShoeUDPClientLibrary* instance;
	static FIPv4Address LeftIP;
	static FIPv4Address RightIP;
	static int32 Port;

	int32 desiredHeights[NUM_OF_CHAMBERS*2];
	int32 zeroHeights[NUM_OF_CHAMBERS * 2];

	bool LeftInAir = false;
	bool RightInAir = false;
    
    int32 cmToShoeUnits(double cm);
    bool sendDataToFoot(EFootEnum foot, int32 data);

	static UShoeUDPClientLibrary* Init();
    
public:

	static UShoeUDPClientLibrary* Constructor();
	UFUNCTION(BlueprintCallable, Category = "Shoe UDP Client")
	static UShoeUDPClientLibrary* GetInstance();

	//UShoeUDPClientLibrary();
	~UShoeUDPClientLibrary();
	UFUNCTION(BlueprintCallable, Category = "Shoe UDP Client")
	bool connect();
    
    
    UFUNCTION(BlueprintCallable, Category = "Shoe UDP Client")
    bool setCurrentStateToZero(EFootEnum foot);
    
    // height must be in cm. it will be converted to the shoes units for you.
    // height equals the number of cm that you would like lower the chamber by
    // TODO: make this static
	UFUNCTION(BlueprintCallable, Category = "Shoe UDP Client")
	static bool setDesiredState(EFootEnum foot, uint8 chamber, float height);

	UFUNCTION(BlueprintCallable, Category = "Shoe UDP Client")
	static bool normalizeDesiredState();
    
	static void shoeStateChanged();
	UFUNCTION(BlueprintCallable, Category = "Shoe UDP Client")
	static void setIsLeftInAir(bool isInAir);
	UFUNCTION(BlueprintCallable, Category = "Shoe UDP Client")
	static void setIsRightInAir(bool isInAir);

    // This is a callback function which is to be called when new data is received from the shoe.
	UFUNCTION(BlueprintNativeEvent)
	void ShoeStateUpdated();
};