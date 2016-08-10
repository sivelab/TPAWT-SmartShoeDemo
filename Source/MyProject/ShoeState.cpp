// Fill out your copyright notice in the Description page of Project Settings.

#include "MyProject.h"
#include "ShoeState.h"


UShoeState* UShoeState::staticInst = nullptr;

UShoeState::UShoeState(const FObjectInitializer &init) : UObject(init) {
}

UShoeState::UShoeState() {
}

UShoeState::~UShoeState() {

}

UShoeState* UShoeState::Init() {
	if (staticInst == nullptr) {
		staticInst = (UShoeState*)UShoeState::StaticClass()->GetDefaultObject();
		//staticInst = new UShoeState();
	}
	return staticInst;
}

int32 UShoeState::getChamberHeight(EFootEnum foot, uint8 chamber) {
	int offset = 0;
	if (foot == EFootEnum::F_Right) {
		offset = 7;
	}
	return heights[offset + chamber];
}


int32 UShoeState::getChamberPressure(EFootEnum foot, uint8 chamber) {
	int offset = 0;
	if (foot == EFootEnum::F_Right) {
		offset = 7;
	}
	return pressures[offset + chamber];
}


bool UShoeState::getChamberState(EFootEnum foot, uint8 chamber) {
	int offset = 0;
	if (foot == EFootEnum::F_Right) {
		offset = 1;
	}

	return ((states[offset] & (1 << chamber)) >> chamber) == 1;
}

void UShoeState::setChamberHeight(EFootEnum foot, uint8 chamber, int32 height) {
	int offset = 0;
	if (foot == EFootEnum::F_Right) {
		offset = 7;
	}
	heights[offset + chamber] = height;
}

void UShoeState::setChamberPressure(EFootEnum foot, uint8 chamber, int32 pressure) {
	int offset = 0;
	if (foot == EFootEnum::F_Right) {
		offset = 7;
	}
	pressures[offset + chamber] = pressure;
}

void UShoeState::setChamberState(EFootEnum foot, uint8 state) {
	int offset = 0;
	if (foot == EFootEnum::F_Right) {
		offset = 1;
	}
	states[offset] = state;
}

void UShoeState::setUpdateCallback(callback updateCallback) {
	this->updateCallback = updateCallback;
}