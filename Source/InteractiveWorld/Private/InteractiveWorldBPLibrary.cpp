// Copyright 2023 Sun BoHeng

#include "InteractiveWorldBPLibrary.h"
#include "Engine/TextureRenderTarget2D.h"

UInteractiveWorldBPLibrary::UInteractiveWorldBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void UInteractiveWorldBPLibrary::ResizeRenderTarget2D(UTextureRenderTarget2D* RenderTarget2D,int32 SizeX,int32 SizeY)
{
	if (RenderTarget2D)
	{
		RenderTarget2D->ResizeTarget(SizeX,SizeY);
	}
}

//Editor only!
void UInteractiveWorldBPLibrary::RefreshInteractVolume(AWorldInteractVolume* InteractVolume)
{
#if WITH_EDITOR
	InteractVolume->GetOnVolumeShapeChangedDelegate().Broadcast(*InteractVolume);
#endif
}

void UInteractiveWorldBPLibrary::IW_AddWarning(FString Message)
{
	FMessageLog("PIE").Warning()
	->AddToken(FTextToken::Create(FText::FromString(FString(TEXT("Interactive World : ")))))
	->AddToken(FTextToken::Create(FText::FromString(Message)));
}
