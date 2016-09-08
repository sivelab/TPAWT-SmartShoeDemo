// Fill out your copyright notice in the Description page of Project Settings.

#include "MyProject.h"
#include "ShoeRecvThread.h"

FShoeRecvThread* FShoeRecvThread::Runnable = nullptr;

FShoeRecvThread::FShoeRecvThread() : shoeState(UShoeState::Init()), running(true), Connected(false), totalRecv(0)
{
	Thread = FRunnableThread::Create(this, TEXT("FShoeRecvThread"), 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify more
}

FShoeRecvThread::~FShoeRecvThread()
{
	delete Thread;
	Thread = NULL;
}

void FShoeRecvThread::setupNetwork() {
	UE_LOG(LogClass, Warning, TEXT("Connecting..."));
	//const FIPv4Address IP(192, 168, 100, 107); // Local IP
	const FIPv4Address IP(131, 212, 41, 3); // Local IP
    //const FIPv4Address IP(192, 168, 0, 8);
	const int32 Port = 2050;
	TSharedRef<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	Addr->SetIp(IP.Value);
	//Addr->SetIp(IP.GetValue());
	Addr->SetPort(Port);

	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_DGram, TEXT("UDP Recv Socket"), false);
	Connected = Socket->Bind(*Addr);
}

bool FShoeRecvThread::Init() {
	return true;
}

uint32 FShoeRecvThread::Run() {
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);
	while (StopTaskCounter.GetValue() == 0 && !IsFinished())
	{
		/*PrimeNumbers->Add(FindNextPrimeNumber());
		PrimesFoundCount++;*/
		//checkForIncomingData(); NOTE: For some reason when are you debugging the debugger does not go into any functions called from this one. That is why I have moved the code here.
		if (shoeState) {
			//checkForIncomingData();
			if (!Connected) {
				setupNetwork();
			}
			TArray<uint8> ReceivedData;
			bool DataWasReceived = false;
			if (Connected) {
				uint32 Size;
				while (Socket->HasPendingData(Size)) {
					ReceivedData.Init(0, FMath::Min(Size, 65507u));
					DataWasReceived = true;

					int32 Read = 0;
					Socket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);
					for (int i = 0; i < ReceivedData.Num(); i++) {
						Data.push_back(ReceivedData[i]);
					}
					//Data.Insert(ReceivedData, Data.Num());
					totalRecv += ReceivedData.Num();
					shoeState->setChamberHeight(EFootEnum::F_Right, 0, totalRecv);

					uint8 tmp;
					if (Data.size() >= 39) {
						// header: 3 0x7f
						tmp = Data[0];
						Data.erase(Data.begin());
						if (tmp != 0x7f) {
							break;
							//return 0;
						}
						tmp = Data[0];
						Data.erase(Data.begin());
						if (tmp != 0x7f) {
							break;
							//return 0;
						}
						tmp = Data[0];
						Data.erase(Data.begin());
						if (tmp != 0x7f) {
							break;
							//return 0;
						}

						// shoe status first bit decides which shoe we are dealing with
						tmp = Data[0];
						Data.erase(Data.begin());
						bool side = ((tmp & (1 << 7)) >> 7) == 1;
						uint8 status = (tmp & 0x7f);

						EFootEnum foot = EFootEnum::F_Left;
						if (side) {
							foot = EFootEnum::F_Right;
						}

						if (shoeState) {
							shoeState->setChamberState(foot, status);
						}

						// get data for each chamber
						for (uint8 i = 0; i < 7; i++) {
							uint32 height = (((uint32)Data[0] << 8) | ((uint32)Data[1]));
							Data.erase(Data.begin());
							Data.erase(Data.begin());
							uint32 pressure = (((uint32)Data[0] << 16) | ((uint32)Data[1] << 8) | ((uint32)Data[2]));
							Data.erase(Data.begin());
							Data.erase(Data.begin());
							Data.erase(Data.begin());

							if (shoeState) {
								shoeState->setChamberHeight(foot, i, height);
								shoeState->setChamberPressure(foot, i, pressure);
							}
						}
					}

					//shoeState->setChamberHeight(EFootEnum::F_Left, 0, totalRecv);
				}
				if (DataWasReceived) {
					if (shoeState->updateCallback) {
						shoeState->updateCallback();
					}
				}
			}
		}

		//***************************************
		//Show Incremental Results in Main Game Thread!

		//	Please note you should not create, destroy, or modify UObjects here.
		//	  Do those sort of things after all thread are completed.

		//	  All calcs for making stuff can be done in the threads
		//	     But the actual making/modifying of the UObjects should be done in main game thread.
		//ThePC->ClientMessage(FString::FromInt(PrimeNumbers->Last()));
		//***************************************

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//prevent thread from using too many resources
		FPlatformProcess::Sleep(0.01);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	}

	//Run FPrimeNumberWorker::Shutdown() from the timer in Game Thread that is watching
	//to see when FPrimeNumberWorker::IsThreadFinished()

	return 0;
}

void FShoeRecvThread::Stop()
{
	StopTaskCounter.Increment();
}

FShoeRecvThread* FShoeRecvThread::Create() {
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FShoeRecvThread();
	}
	return Runnable;
}

void FShoeRecvThread::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void FShoeRecvThread::Shutdown()
{
	if (Runnable) {
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool FShoeRecvThread::IsThreadFinished()
{
	if (Runnable) return Runnable->IsFinished();
	return true;
}
