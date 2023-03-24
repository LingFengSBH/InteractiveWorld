// Copyright 2023 Sun BoHeng

#pragma once

#include "WorldInteractVolume.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InteractiveWorldBPLibrary.generated.h"

UCLASS()
class UInteractiveWorldBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	//Resize RenderTarget2D in blueprint
	//This will call RenderTarget2D->ResizeTarget(SizeX,SizeY)
	//Resizes the render target without recreating the FTextureResource.  Will not flush commands unless the render target resource doesnt exist
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Resize Rendertarget2D", Keywords = "InteractiveWorld"), Category = "ToolLibrary")
    static void ResizeRenderTarget2D(UTextureRenderTarget2D* RenderTarget2D,int32 SizeX,int32 SizeY);

	//Convert Vector3 to Vector2.Why there are not a FVector.XY?
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Vector3 to Vector2", Keywords = "vector"), Category = "ToolLibrary")
	static FVector2D Vector3ToVector2(FVector inVector) {return FVector2D(inVector.X,inVector.Y);}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Refresh Interact Volumes", Keywords = "vector"), Category = "ToolLibrary")
	static void RefreshInteractVolume(AWorldInteractVolume* InteractVolume);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "IW Add Warning", Keywords = "warning"), Category = "ToolLibrary")
	static void IW_AddWarning(FString Message);
};