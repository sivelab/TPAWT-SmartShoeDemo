// Fill out your copyright notice in the Description page of Project Settings.

#include "MyProject.h"
#include "ShoeUDPClientLibrary.h"

//General Log
DEFINE_LOG_CATEGORY(ShoeLog);

//Logging for output to shoe
DEFINE_LOG_CATEGORY(ShoeOutput);

//Logging for input from shoe
DEFINE_LOG_CATEGORY(ShoeInput);

UShoeUDPClientLibrary* UShoeUDPClientLibrary::instance = nullptr;
//FIPv4Address UShoeUDPClientLibrary::LeftIP = FIPv4Address(131, 212, 41, 37);
FIPv4Address UShoeUDPClientLibrary::LeftIP = FIPv4Address(131, 212, 41, 25);
//FIPv4Address UShoeUDPClientLibrary::LeftIP = FIPv4Address(192, 168, 100, 107);
//FIPv4Address UShoeUDPClientLibrary::LeftIP = FIPv4Address(192, 168, 0, 8);

//FIPv4Address UShoeUDPClientLibrary::RightIP = FIPv4Address(131, 212, 41, 69);
FIPv4Address UShoeUDPClientLibrary::RightIP = FIPv4Address(131, 212, 41, 5);
//FIPv4Address UShoeUDPClientLibrary::RightIP = FIPv4Address(192, 168, 100, 107);
//FIPv4Address UShoeUDPClientLibrary::RightIP = FIPv4Address(192, 168, 0, 8);
int32 UShoeUDPClientLibrary::Port = 2000;

UShoeUDPClientLibrary::UShoeUDPClientLibrary(const FObjectInitializer &init) : UObject(init), shoeState(UShoeState::Init()) {
	shoeState->setUpdateCallback(UShoeUDPClientLibrary::shoeStateChanged);
}

UShoeUDPClientLibrary::~UShoeUDPClientLibrary() {
	FShoeRecvThread::Shutdown();
}

UShoeUDPClientLibrary* UShoeUDPClientLibrary::Constructor() {
	
	return (UShoeUDPClientLibrary*)NewObject<UShoeUDPClientLibrary>();
}

UShoeUDPClientLibrary* UShoeUDPClientLibrary::Init() {
	UE_LOG(ShoeLog, Log, TEXT("Initing... v3"));

	if (!instance) {
		instance = Constructor();
	}

	instance->LeftEndpoint = FIPv4Endpoint(UShoeUDPClientLibrary::LeftIP, Port);
	instance->RightEndpoint = FIPv4Endpoint(UShoeUDPClientLibrary::RightIP, Port);

	instance->recvThread = nullptr;

	instance->shoeState = UShoeState::Init();
	instance->shoeState->setUpdateCallback(UShoeUDPClientLibrary::shoeStateChanged);

	instance->SocketRight = nullptr;
	instance->SocketLeft = nullptr;

	/*instance->LeftInAir = false;
	instance->RightInAir = false;*/

	return instance;
}

UShoeUDPClientLibrary* UShoeUDPClientLibrary::GetInstance() {
	if (instance == nullptr) {
		UShoeUDPClientLibrary::Init();
	}
	return instance;
}

bool UShoeUDPClientLibrary::connect() {
	UE_LOG(ShoeLog, Log, TEXT("Connecting..."));
	LeftConnected = SocketLeft && SocketLeft->GetConnectionState() == ESocketConnectionState::SCS_Connected;
	RightConnected = SocketRight && SocketRight->GetConnectionState() == ESocketConnectionState::SCS_Connected;

	if (!LeftConnected) {
		// Connect Left
		SocketLeft = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_DGram, TEXT("Left Shoe Socket Out"), false);
		LeftConnected = SocketLeft->Connect(*LeftEndpoint.ToInternetAddr());
	}

	if (!RightConnected) {
		// Connect right
		SocketRight = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_DGram, TEXT("Right Foot Socket Out"), false);
		RightConnected = SocketRight->Connect(*RightEndpoint.ToInternetAddr());
	}

	if (!recvThread) {
		recvThread = FShoeRecvThread::Create();
	}

	return LeftConnected && RightConnected;
}

bool UShoeUDPClientLibrary::sendDataToFoot(EFootEnum foot, int32 data) {
	connect();

	FString footString = FString(TEXT("R"));
	if (foot == EFootEnum::F_Left) {
		footString = FString(TEXT("L"));
	}

	UE_LOG(ShoeOutput, Log, TEXT("%s %d"), *footString, data);

	TArray<uint8> OutData;
	const uint8 * Bytes = reinterpret_cast<const uint8*>(&data);
	int32 size = 3;
	for (int i = 0; i < size; ++i) // int = 4 bytes
		OutData.Add(Bytes[i]);
	if (foot == EFootEnum::F_Left) {
		if (LeftConnected) {
			int32 sent;
			SocketLeft->Send(OutData.GetData(), size, sent);
			return sent == size;
		}
		else {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Left Shoe not connected"));
		}
	}
	if (foot == EFootEnum::F_Right) {
		if (RightConnected) {
			int32 sent;
			SocketRight->Send(OutData.GetData(), size, sent);
			return sent == size;
		}
		else {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Right Shoe not connected"));
		}
	}
	return false;
}

bool UShoeUDPClientLibrary::setDesiredState(EFootEnum foot, uint8 chamber, float height) {
	if (!instance) {
		UShoeUDPClientLibrary::Init();
	}

	int offset = 0;
	if (foot == EFootEnum::F_Right) {
		offset = NUM_OF_CHAMBERS;
	}
	instance->desiredHeights[offset + chamber] = instance->cmToShoeUnits(height);
	return true;
}

// Makes all the heights in the desired state less than or equal to 0
// This is needed before the shoe react properly
bool UShoeUDPClientLibrary::normalizeDesiredState() {
	if (!instance) {
		UShoeUDPClientLibrary::Init();
	}
	int maxHeight = INT_MIN;

	for (int i = 0; i < NUM_OF_CHAMBERS*2; i++) {
		if (instance->desiredHeights[i] > maxHeight) {
			maxHeight = instance->desiredHeights[i];
		}
	}

	for (int i = 0; i < NUM_OF_CHAMBERS * 2; i++) {
		instance->desiredHeights[i] -= maxHeight;
	}

	UE_LOG(ShoeOutput, Log, TEXT("Time: %f"), FPlatformTime::Seconds());
	for (int i = 0; i < NUM_OF_CHAMBERS; i++) {
		UE_LOG(ShoeOutput, Log, TEXT("Desired Height Left %d: %d"), i, instance->desiredHeights[i]);
	}
	for (int i = 0; i < NUM_OF_CHAMBERS; i++) {
		UE_LOG(ShoeOutput, Log, TEXT("Desired Height Right %d: %d"), i, instance->desiredHeights[i+NUM_OF_CHAMBERS]);
	}

	return true;
}

// This function is for calibration.
// It should be called when the shoe is at it's highest.
bool UShoeUDPClientLibrary::setCurrentStateToZero(EFootEnum foot) {
    int offset = 0;
    if (foot == EFootEnum::F_Right) {
        offset = NUM_OF_CHAMBERS;
    }
    for (int i=0; i < NUM_OF_CHAMBERS; i++) {
        zeroHeights[offset + i] = shoeState->getChamberHeight(foot, i);
    }
    return true;
}

void UShoeUDPClientLibrary::shoeStateChanged() {
	if (!instance || !instance->LeftConnected || !instance->RightConnected) {
		return;
	}

	// This needs to be changed to a better measurement.
	/*bool LeftInAir = true;
	bool RightInAir = true;
	for (int i = 0; i < NUM_OF_CHAMBERS; i++) {
		instance->shoeState->getChamberPressure(EFootEnum::F_Right, 0) <= RESTING_PRESSURE;
		instance->shoeState->getChamberPressure(EFootEnum::F_Left, 0) <= RESTING_PRESSURE;
	}*/

	// Log the current state
	// TODO: maybe make a throttler
	UE_LOG(ShoeInput, Log, TEXT("Time: %f"), FPlatformTime::Seconds());
	for (int i = 0; i < NUM_OF_CHAMBERS; i++) {
		UE_LOG(ShoeInput, Log, TEXT("Chamber Left %d: %d %d"), i, instance->shoeState->getChamberState(EFootEnum::F_Left, i), instance->shoeState->getChamberHeight(EFootEnum::F_Left, i));
	}
	for (int i = 0; i < NUM_OF_CHAMBERS; i++) {
		UE_LOG(ShoeInput, Log, TEXT("Chamber Right %d: %d %d"), i, instance->shoeState->getChamberState(EFootEnum::F_Right, i), instance->shoeState->getChamberHeight(EFootEnum::F_Right, i));
	}
	

	if (instance->LeftInAir) {
		bool isOneClosed = false;
		for (int i = 0; i < NUM_OF_CHAMBERS; i++) {
			isOneClosed |= !instance->shoeState->getChamberState(EFootEnum::F_Left, i);
			if (isOneClosed) {
				break;
			}
		}
		if (isOneClosed) {
			instance->sendDataToFoot(EFootEnum::F_Left, 0xeeee);
			instance->sendDataToFoot(EFootEnum::F_Left, 0xeeee);
		}
	}
	else {
		for (int i = 0; i < NUM_OF_CHAMBERS; i++) {
			bool desiredState = false;
			if (instance->shoeState->getChamberHeight(EFootEnum::F_Left, i) > instance->desiredHeights[i] + instance->zeroHeights[i]) {
				desiredState = true;
			}

			if (instance->shoeState->getChamberState(EFootEnum::F_Left, i) != desiredState) {
				int32 data = 0;
				if (desiredState) {
					data |= 0xee00;
				}
				else {
					data |= 0xff00;
				}
				data |= (i + 1);
				instance->sendDataToFoot(EFootEnum::F_Left, data);
			}
		}
	}
	if (instance->RightInAir) {
		bool isOneClosed = false;
		for (int i = 0; i < NUM_OF_CHAMBERS; i++) {
			isOneClosed |= !instance->shoeState->getChamberState(EFootEnum::F_Right, i);
			if (isOneClosed) {
				break;
			}
		}
		if (isOneClosed) {
			instance->sendDataToFoot(EFootEnum::F_Right, 0xeeee);
			instance->sendDataToFoot(EFootEnum::F_Right, 0xeeee);
		}
	}
	else {
		for (int i = 0; i < NUM_OF_CHAMBERS; i++) {
			bool desiredState = false;
			if (instance->shoeState->getChamberHeight(EFootEnum::F_Right, i) > instance->desiredHeights[i+NUM_OF_CHAMBERS] + instance->zeroHeights[i+NUM_OF_CHAMBERS]) { // plus NUM_OF_CHAMBERS because this is the right shoe
				desiredState = true;
			}

			if (instance->shoeState->getChamberState(EFootEnum::F_Right, i) != desiredState) {
				int32 data = 0;
				if (desiredState) {
					data |= 0xee00;
				}
				else {
					data |= 0xff00;
				}
				data |= (i + 1);
				instance->sendDataToFoot(EFootEnum::F_Right, data);
			}
		}
	}
	//instance->ShoeStateUpdated();
}

void UShoeUDPClientLibrary::ShoeStateUpdated_Implementation() {
}


void UShoeUDPClientLibrary::setIsLeftInAir(bool isInAir) {
	instance->LeftInAir = isInAir;
}

void UShoeUDPClientLibrary::setIsRightInAir(bool isInAir) {
	instance->RightInAir = isInAir;
}

int32 UShoeUDPClientLibrary::cmToShoeUnits(double cm) {
    return (int32)(cm*SHOE_UNITS_PER_CM);
}