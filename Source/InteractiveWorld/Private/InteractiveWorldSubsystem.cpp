// Copyright 2023 Sun BoHeng


#include "InteractiveWorldSubsystem.h"
#include "InteractiveWorldBPLibrary.h"
#include "Kismet/GameplayStatics.h"


void UInteractiveWorldSubsystem::Tick(float DeltaTime)
{
	if (DrawingBoards.Num() > 0)
	{
		AllocateBrushes();
	}
}

void UInteractiveWorldSubsystem::RegisterBrush(UInteractBrush* Brush)
{
	Brushes.AddUnique(Brush);
	UE_LOG(LogTemp, Log, TEXT("%s Registered"), *Brush->GetName())
}

void UInteractiveWorldSubsystem::UnregisterBrush(UInteractBrush* Brush)
{
	Brushes.Remove(Brush);
	UE_LOG(LogTemp, Log, TEXT("%s UnRegistered"), *Brush->GetName())
}

void UInteractiveWorldSubsystem::RegisterDrawingBoard(AWorldDrawingBoard* DrawingBoard)
{
	DrawingBoards.AddUnique(DrawingBoard);
	UE_LOG(LogTemp, Log, TEXT("%s Registered"), *DrawingBoard->GetName())
}

void UInteractiveWorldSubsystem::UnregisterDrawingBoard(AWorldDrawingBoard* DrawingBoard)
{
	DrawingBoards.Remove(DrawingBoard);
	UE_LOG(LogTemp, Log, TEXT("%s UnRegistered"), *DrawingBoard->GetName())
}

bool UInteractiveWorldSubsystem::PrepareBrushes(TArray<UInteractBrush*>& BrushesNeedDrawing)
{
	TArray<TSubclassOf<AWorldDrawingBoard>> NoVolumeDrawingBoardClass;
	TArray<AWorldDrawingBoard*> InvalidDrawingBoard;
	//Find DrawingBoards that do not use InteractVolumes
	for (auto DrawingBoard : DrawingBoards)
	{
		if (!DrawingBoard)
		{
			InvalidDrawingBoard.Add(DrawingBoard);
			continue;
		}
		if (!DrawingBoard->GetUseInteractVolume())
		{
			NoVolumeDrawingBoardClass.Add(DrawingBoard->GetClass());
		}
	}
	//Clear invalid DrawingBoards
	for (auto DrawingBoard : InvalidDrawingBoard)
	{
		DrawingBoards.Remove(DrawingBoard);
	}
	
	bool bHasAnyBrushNeedDrawing = false;
	BrushesNeedDrawing.Reset();
	TArray<UInteractBrush*> InvalidBrushes;
	
	if (BrushCullDistance < 0)
	{
		//No distance culling
		for (auto Brush : Brushes)
		{
			if (Brush)
			{
				if (Brush->PrepareForDrawing(NoVolumeDrawingBoardClass))
				{
					BrushesNeedDrawing.Add(Brush);
					bHasAnyBrushNeedDrawing = true;
				}
			}
			else
			{
				InvalidBrushes.Add(Brush);
			}
		}
	}
	else
	{
		//Use distance culling
		const FVector CameraLocation = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation();
		for (auto Brush : Brushes)
		{
			if (Brush)
			{
				if (FVector::DistXY(CameraLocation, Brush->GetComponentLocation()) <= BrushCullDistance + Brush->
					GetCullRadius()
					&& Brush->PrepareForDrawing(NoVolumeDrawingBoardClass))
				{
					BrushesNeedDrawing.Add(Brush);
					bHasAnyBrushNeedDrawing = true;
				}
			}
			else
			{
				InvalidBrushes.Add(Brush);
			}
		}
	}
	for (auto BrushToRemove : InvalidBrushes)
	{
		Brushes.Remove(BrushToRemove);
	}
	return bHasAnyBrushNeedDrawing;
}

void UInteractiveWorldSubsystem::AllocateBrushes()
{
	TArray<UInteractBrush*> BrushesNeedDrawing;
	if (PrepareBrushes(BrushesNeedDrawing))
	{
		TArray<UInteractBrush*> BrushesForDrawingBoard;
		for (const auto DrawingBoard : DrawingBoards)
		{
			if (!DrawingBoard->GetActiveState())
			{
				//This DrawingBoard is not active
				continue;
			}
			//Update location before we allocate,so that the DrawingBoard range culling will be correct
			DrawingBoard->UpdateDrawingBoardState();
			BrushesForDrawingBoard.Reset();
			if (DrawingBoard->GetShouldDrawOn())
			{
				for (auto Brush : BrushesNeedDrawing)
				{
					//Cull InteractBrushes outside the DrawingBoard area
					if (Brush->ShouldDrawOn(DrawingBoard)
						&& DrawingBoard->GetNearestDistance(
							UInteractiveWorldBPLibrary::Vector3ToVector2(Brush->GetCurrentTransform().GetLocation())) <
						Brush->GetCullRadius())
					{
						BrushesForDrawingBoard.Add(Brush);
					}
				}
			}
			DrawingBoard->PrepareForSimulate(BrushesForDrawingBoard);
		}
		for (const auto Brush : BrushesNeedDrawing)
		{
			Brush->FinishDraw();
		}
	}
	else
	{
		for (const auto DrawingBoard : DrawingBoards)
		{
		    DrawingBoard->UpdateDrawingBoardState();
			//No InteractBrushes,but the simulation should be continue,like water's wave
			DrawingBoard->PrepareForSimulate();
		}
	}
}

void UInteractiveWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
}
