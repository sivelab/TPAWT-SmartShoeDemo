// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Core.h"
#include "ShoeState.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "Networking.h"
#include <vector>

/**
 * 
 */
class FShoeRecvThread : public FRunnable
{
	/** Singleton instance, can access the thread any time via static accessor, if it is active! */
	static FShoeRecvThread* Runnable;

	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	/** The Data Ptr */
	UShoeState* shoeState;

	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;

	//The actual finding of prime numbers
	//void checkForIncomingData();

	void setupNetwork();

	FSocket* Socket;
	bool Connected;
	uint32 totalRecv;
	//TArray<uint8> Data;
	std::vector<uint8> Data;

private:
	bool	running;
public:
	//Done?
	bool IsFinished() const
	{
		return !running;
	}

	//~~~ Thread Core Functions ~~~

	//Constructor / Destructor
	FShoeRecvThread();
	virtual ~FShoeRecvThread();

	// Begin FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	// End FRunnable interface

	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();



	//~~~ Starting and Stopping Thread ~~~



	/*
	Start the thread and the worker from static (easy access)!
	This code ensures only 1 Prime Number thread will be able to run at a time.
	This function returns a handle to the newly started instance.
	*/
	static FShoeRecvThread* Create();

	/** Shuts down the thread. Static so it can easily be called from outside the thread context */
	static void Shutdown();

	static bool IsThreadFinished();
};
