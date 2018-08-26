// Fill out your copyright notice in the Description page of Project Settings.

#include "FrameGrabberActor.h"

#if WITH_EDITOR
	#include "Editor.h"
	#include "Editor/EditorEngine.h"
	#include "ILevelViewport.h"
#endif

// Sets default values
AFrameGrabberActor::AFrameGrabberActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFrameGrabberActor::BeginPlay()
{
	Super::BeginPlay();
}

void AFrameGrabberActor::BeginDestroy()
{
	Super::BeginDestroy();

	ReleaseFrameGrabber();

	if (CaptureFrameTexture)
	{
		CaptureFrameTexture->ConditionalBeginDestroy();
		CaptureFrameTexture = nullptr;
	}
}

// Called every frame
void AFrameGrabberActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (FrameGrabber.IsValid())
	{
		Capture();
	}
}

void AFrameGrabberActor::Capture()
{
	if (FrameGrabber.IsValid() && CaptureFrameTexture)
	{
		FrameGrabber->CaptureThisFrame(FFramePayloadPtr());
		TArray<FCapturedFrameData> Frames = FrameGrabber->GetCapturedFrames();

		if (Frames.Num())
		{
			FCapturedFrameData& LastFrame = Frames.Last();

			CaptureFrameData.Empty();

			for (int32 i = 0; i < LastFrame.ColorBuffer.Num(); i++)
			{
				//FColor (R,G,B,A)->BGRA 
				CaptureFrameData.Add(LastFrame.ColorBuffer[i].B);
				CaptureFrameData.Add(LastFrame.ColorBuffer[i].G);
				CaptureFrameData.Add(LastFrame.ColorBuffer[i].R);
				CaptureFrameData.Add(LastFrame.ColorBuffer[i].A);
			}


			auto Region = new FUpdateTextureRegion2D(0, 0, 0, 0, LastFrame.BufferSize.X, LastFrame.BufferSize.Y);
			CaptureFrameTexture->UpdateTextureRegions(0, 1, Region, 4 * LastFrame.BufferSize.X, 4, CaptureFrameData.GetData());
		}
	}
	
}

bool  AFrameGrabberActor::StartFrameGrab()
{
	TSharedPtr<FSceneViewport> SceneViewport;

	// Get SceneViewport
	// ( quoted from FRemoteSessionHost::OnCreateChannels() )
#if WITH_EDITOR
	if (GIsEditor)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE)
			{
				FSlatePlayInEditorInfo* SlatePlayInEditorSession = GEditor->SlatePlayInEditorMap.Find(Context.ContextHandle);
				if (SlatePlayInEditorSession)
				{
					if (SlatePlayInEditorSession->DestinationSlateViewport.IsValid())
					{
						TSharedPtr<ILevelViewport> DestinationLevelViewport = SlatePlayInEditorSession->DestinationSlateViewport.Pin();
						SceneViewport = DestinationLevelViewport->GetSharedActiveViewport();
					}
					else if (SlatePlayInEditorSession->SlatePlayInEditorWindowViewport.IsValid())
					{
						SceneViewport = SlatePlayInEditorSession->SlatePlayInEditorWindowViewport;
					}
				}
			}
		}
	}
	else
#endif
	{
		UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
		SceneViewport = GameEngine->SceneViewport;
	}
	if (!SceneViewport.IsValid())
	{
		return false;
	}

	// Setup Texture
	if (!CaptureFrameTexture)
	{
		CaptureFrameTexture = UTexture2D::CreateTransient(SceneViewport.Get()->GetSize().X, SceneViewport.Get()->GetSize().Y, PF_B8G8R8A8);
		CaptureFrameTexture->UpdateResource();

		MaterialInstanceDynamic->SetTextureParameterValue(FName("Texture"), CaptureFrameTexture);
	}
	
	// Capture Start
	ReleaseFrameGrabber();
	FrameGrabber = MakeShareable(new FFrameGrabber(SceneViewport.ToSharedRef(), SceneViewport->GetSize()));
	FrameGrabber->StartCapturingFrames();

	return true;
}

void  AFrameGrabberActor::StopFrameGrab()
{
	ReleaseFrameGrabber();
}

void AFrameGrabberActor::ReleaseFrameGrabber()
{
	if (FrameGrabber.IsValid())
	{
		FrameGrabber->StopCapturingFrames();
		FrameGrabber->Shutdown();
		FrameGrabber.Reset();
	}
}

void AFrameGrabberActor::SetMaterialInstanceDynamic(UMaterialInstanceDynamic* MI)
{
	MaterialInstanceDynamic = MI;
}