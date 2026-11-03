// Weichao Qiu @ 2018
#include "Sensors/CVNormalCamSensor.h"
#include "CVCameraLog.h"

UCVNormalCamSensor::UCVNormalCamSensor(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	this->ShowFlags.SetPostProcessing(true);
	this->ShowFlags.SetPostProcessMaterial(true);

	FString NormalPPMaterialPath = TEXT("Material'/CVCamera/WorldNormal.WorldNormal'");
	ConstructorHelpers::FObjectFinder<UMaterial> Material(*NormalPPMaterialPath);
	if (IsValid(Material.Object))
	{
		this->SurfaceNormalPPMaterial = Material.Object;
		SetPostProcessMaterial(SurfaceNormalPPMaterial);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load normal post process material: %s"), *NormalPPMaterialPath);
	}
}

// void UCVNormalCamSensor::SetupRenderTarget()
// {
// 	bool bUseLinearGamma = false;
// 	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);
// }

void UCVNormalCamSensor::InitTextureTarget(int filmWidth, int filmHeight)
{
	Super::InitTextureTarget(filmWidth, filmHeight);
	if (this->SurfaceNormalPPMaterial)
	{
		SetPostProcessMaterial(this->SurfaceNormalPPMaterial);
	}
}

void UCVNormalCamSensor::CaptureNormal(TArray<FColor>& ImageData, int& Width, int& Height)
{
	if (!CheckTextureTarget())
	{
		InitTextureTarget(this->FilmWidth, this->FilmHeight);
		if (!CheckTextureTarget())
		{
			UE_LOG(LogCVCamera, Error, TEXT("Failed to initialize TextureTarget."));
			return;
		}
	}
	Capture(ImageData, Width, Height);
	// if (ImageData.Num() != 0)
	// {
	// 	if (Width > 0 && Height > 0 && static_cast<uint32>(Width * Height) == ImageData.Num())
	// 	{
	// 		SetAlpha(ImageData);
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogCVCamera, Warning, TEXT("Invalid Width or Height for ImageData in CaptureNormal"));
	// 	}
	// }
}
