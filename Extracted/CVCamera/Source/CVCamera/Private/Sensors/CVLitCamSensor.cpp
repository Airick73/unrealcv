// Weichao Qiu @ 2017
#include "Sensors/CVLitCamSensor.h"
#include "CVCameraLog.h"

#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "TextureResource.h"

DECLARE_CYCLE_STAT(TEXT("UCVLitCamSensor::CaptureLit"), STAT_CaptureLit, STATGROUP_CVCamera);

UCVLitCamSensor::UCVLitCamSensor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

// void UCVLitCamSensor::SetupRenderTarget()
// {
// 	bool bUseLinearGamma = false;
// 	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);
// }

// bool bUseLinearGamma = false;
// bool bUseLinearGamma = true;  // true by default
// bUseLinearGamma requires CaptureEveryFrame!
// TextureTarget->InitAutoFormat(Width, Height);
// TextureTarget->TargetGamma = GEngine->GetDisplayGamma();
// TextureTarget->TargetGamma = 1;

void UCVLitCamSensor::InitTextureTarget(int filmWidth, int filmHeight)
{
	TextureTarget = NewObject<UTextureRenderTarget2D>(this); 
	TextureTarget->InitAutoFormat(filmWidth, filmHeight);
	TextureTarget->TargetGamma = GEngine->GetDisplayGamma();
}

void UCVLitCamSensor::CaptureLit(TArray<FColor>& Image, int& Width, int& Height)
{
	SCOPE_CYCLE_COUNTER(STAT_CaptureLit);
	if (!CheckTextureTarget())
	{
		InitTextureTarget(this->FilmWidth, this->FilmHeight);
		if (!CheckTextureTarget())
		{
			UE_LOG(LogCVCamera, Error, TEXT("Failed to initialize TextureTarget."));
			return;
		}
	}
	UpdateCineCameraView();
	this->CaptureScene();
	FReadSurfaceDataFlags ReadSurfaceDataFlags;
	ReadSurfaceDataFlags.SetLinearToGamma(false); 
	// TextureTarget->GetRenderTargetResource()->ReadPixels(Image, ReadSurfaceDataFlags);
	TextureTarget->GameThread_GetRenderTargetResource()->ReadPixels(Image, ReadSurfaceDataFlags);
	if (Image.Num() == 0)
	{
		UE_LOG(LogCVCamera, Warning, TEXT("Captured lit data is empty."));
	}
	Width = GetFilmWidth();
	Height = GetFilmHeight();
}
