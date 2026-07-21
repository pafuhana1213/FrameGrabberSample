// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "FrameGrabber.h"
#include "Engine/GameEngine.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "FrameGrabberActor.generated.h"

class FFrameGrabber;

UCLASS()
class FRAMEGRABBERSAMPLE_API AFrameGrabberActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFrameGrabberActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	bool StartFrameGrab();

	UFUNCTION(BlueprintCallable)
	void StopFrameGrab();

	UFUNCTION(BlueprintCallable)
	void SetMaterialInstanceDynamic(UMaterialInstanceDynamic* MI);

private:
	void ReleaseFrameGrabber();
	void Capture();

public:

	UPROPERTY()
	UMaterialInstanceDynamic* MaterialInstanceDynamic;

	UPROPERTY()
	UTexture2D* CaptureFrameTexture;

private:

	TSharedPtr<FFrameGrabber>	FrameGrabber;
};
