// Copyright 2023 Sun BoHeng

#include "WorldDrawingBoard.h"

#include "InteractiveWorldSubsystem.h"
#include "WorldInteractVolume.h"
#include "Engine/Public/TimerManager.h"
#include "Engine/Canvas.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"

// Sets default values
AWorldDrawingBoard::AWorldDrawingBoard()
{
}

void AWorldDrawingBoard::UpdateActive()
{
	if ((!bUseInteractVolume || ActiveVolumes.Num() > 0) && bActive)
	{
		GetWorld()->GetTimerManager().ClearTimer(StopActiveHandle);
		bDrawingBoardActiveAuto = true;
	}
	else
	{
		if (SleepTime <= 0)
			ShutOff();
		else
			GetWorld()->GetTimerManager().SetTimer(StopActiveHandle, this, &AWorldDrawingBoard::ShutOff, SleepTime);
	}
}

void AWorldDrawingBoard::SetPreviousParameters()
{
	PreviousCanvasWorldLocation = CanvasWorldLocation;
	PreviousCanvasWorldSize = CanvasWorldSize;
	PreviousCanvasWorldYaw = CanvasWorldYaw;
	PreviousPixelWorldSize = PixelWorldSize;
	PreviousRTSize = RTSize;
}

void AWorldDrawingBoard::ReBindInteractVolumes(bool bBind)
{
	if (!bBind)
	{
		for (const auto InteractVolume : InteractVolumes)
		{
			if (InteractVolume)
			{
				InteractVolume->UnBindDrawingBoard(this);
			}
		}
	}
	else
	{
		for (const auto InteractVolume : InteractVolumes)
		{
			if (InteractVolume)
			{
				InteractVolume->BindDrawingBoard(this);
			}
		}
	}
}

// Called when the game starts or when spawned
void AWorldDrawingBoard::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetSubsystem<UInteractiveWorldSubsystem>()->RegisterDrawingBoard(this);

	UpdateActive();
	
}

void AWorldDrawingBoard::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ReBindInteractVolumes(false);
	GetWorld()->GetSubsystem<UInteractiveWorldSubsystem>()->UnregisterDrawingBoard(this);
}

void AWorldDrawingBoard::UpdateDrawingBoardState_Implementation()
{
}

void AWorldDrawingBoard::PrepareForSimulate(TArray<UInteractBrush*> Brushes)
{
	if (Brushes.Num() > 0)
	{
		//Brush will draw on this frame,so TimeFromLastDraw = 0
		TimeFromLastDraw = 0;
		
		PreSimulate();
		if (RTBrushDrawOn)
		{
			DrawBrushes(Brushes, RTBrushDrawOn);
		}
		PostSimulate();
		
		//Only set when successfully updated
		SetPreviousParameters();
	}
	else
	{
		//No brush,use another version
		PrepareForSimulate();
	}
}

void AWorldDrawingBoard::PrepareForSimulate()
{
	//No drawing,so increase TimeFromLastDraw
	TimeFromLastDraw += GetWorld()->DeltaTimeSeconds;
	if (TimeFromLastDraw > SleepTime && SleepTime >= 0)
	{
		// Do nothing
	}
	else
	{
		PreSimulate();
		PostSimulate();
		//Only set when successfully updated
		SetPreviousParameters();
	}
}

void AWorldDrawingBoard::PreSimulate_Implementation()
{
}

void AWorldDrawingBoard::DrawBrushes(TArray<UInteractBrush*> Brushes, UTextureRenderTarget2D* RTDrawOn)
{
	UCanvas* CanvasDrawOn;
	FVector2D CanvasSize;
	FDrawToRenderTargetContext DrawContext;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, RTDrawOn, CanvasDrawOn, CanvasSize, DrawContext);
	for (const auto Brush : Brushes)
	{
		Brush->PreDrawOnRT(this, CanvasDrawOn, CanvasSize);
	}
	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, DrawContext);
}

void AWorldDrawingBoard::PostSimulate_Implementation()
{
}

void AWorldDrawingBoard::SetCanvasWorldLocation(FVector2D NewLocation, bool bPixelAligned, float PixelSizeScale)
{
	if (bPixelAligned)
	{
		CanvasWorldLocation = FVector2D(floor((NewLocation / (PixelWorldSize * PixelSizeScale)).X),
		                                floor((NewLocation / (PixelWorldSize * PixelSizeScale)).Y)) * PixelWorldSize *
			PixelSizeScale;
	}
	else
	{
		CanvasWorldLocation = NewLocation;
	}
}

float AWorldDrawingBoard::WorldToCanvasRotation(float WorldRotation)
{
	return WorldRotation - CanvasWorldYaw;
}

FVector2D AWorldDrawingBoard::WorldToCanvasSize(FVector2D WorldSize)
{
	return WorldSize / PixelWorldSize;
}

FVector2D AWorldDrawingBoard::WorldToCanvasUV(FVector2D WorldLocation)
{
	return UKismetMathLibrary::GetRotated2D((WorldLocation - CanvasWorldLocation) / CanvasWorldSize,
	                                        CanvasWorldYaw * -1) + FVector2D(0.5, 0.5);
}

float AWorldDrawingBoard::GetNearestDistance(FVector2D WorldLocation) const
{
	FVector2D DistanceVector = UKismetMathLibrary::GetRotated2D(WorldLocation - CanvasWorldLocation,
	                                                            CanvasWorldYaw * -1);
	DistanceVector = FVector2D(FMath::Abs(DistanceVector.X), FMath::Abs(DistanceVector.Y)) - CanvasWorldSize;
	return FVector2D(FMath::Max(DistanceVector.X, 0), FMath::Max(DistanceVector.Y, 0)).Length();
}

void AWorldDrawingBoard::WorldToCanvasBrush(FVector2D BrushLocation, FVector2D BrushSize, float BrushRotation,
                                            FVector2D& OutScreenPosition, FVector2D& OutScreenSize,
                                            float& OutScreenRotation)
{
	OutScreenSize = WorldToCanvasSize(BrushSize);
	OutScreenPosition = WorldToCanvasUV(BrushLocation) * RTSize - OutScreenSize / 2;
	OutScreenRotation = WorldToCanvasRotation(BrushRotation);
}

void AWorldDrawingBoard::ResetUseInteractVolume(bool NewUseInteractVolume)
{
	if (bUseInteractVolume == NewUseInteractVolume)
	{
		return;
	}
	bUseInteractVolume = NewUseInteractVolume;
	ReBindInteractVolumes(bUseInteractVolume);
}

void AWorldDrawingBoard::ResetInteractVolumes(TArray<AWorldInteractVolume*> NewInteractVolumes)
{
	ReBindInteractVolumes(false);
	InteractVolumes = NewInteractVolumes;
	ReBindInteractVolumes(true);
}

void AWorldDrawingBoard::InteractVolumeActive(bool bNewActive, AWorldInteractVolume* TargetInteractVolume)
{
	if (!bUseInteractVolume)
	{
		return;
	}
	if (bNewActive)
	{
		ActiveVolumes.AddUnique(TargetInteractVolume);
	}
	else
	{
		ActiveVolumes.Remove(TargetInteractVolume);
	}
	UpdateActive();
}

void AWorldDrawingBoard::SetDrawingBoardActive(bool NewActive)
{
	bActive = NewActive;
	UpdateActive();
}

bool AWorldDrawingBoard::GetIsSimulating()
{
	return GetActiveState() && (SleepTime < 0 || SleepTime >= TimeFromLastDraw);
}

void AWorldDrawingBoard::SetRTDrawOn(UTextureRenderTarget2D* NewRT)
{
	if(!NewRT)
		return;
	RTBrushDrawOn = NewRT;
	RTSize = FVector2d(static_cast<float>(RTBrushDrawOn->SizeX),static_cast<float>(RTBrushDrawOn->SizeY));
	PixelWorldSize = CanvasWorldSize/RTSize;
}
