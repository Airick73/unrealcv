// Weichao Qiu @ 2017
#include "Sensors/CVDepthCamSensor.h"
#include "Sensors/CVAnnotationCamSensor.h"
#include "TextureResource.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"

UCVDepthCamSensor::UCVDepthCamSensor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	// this->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	this->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;
	bIgnoreTransparentObjects = false;
}

void UCVDepthCamSensor::InitTextureTarget(int filmWidth, int filmHeight)
{
	EPixelFormat PixelFormat = EPixelFormat::PF_FloatRGBA;
	bool bUseLinearGamma = true;
 	TextureTarget->InitCustomFormat(filmWidth, filmHeight, EPixelFormat::PF_FloatRGBA, bUseLinearGamma);
}

void UCVDepthCamSensor::CaptureDepth(TArray<float>& DepthData, int& Width, int& Height)
{
	// Default behaviour (inherited from UnrealCV): render depth from the annotation proxies, which are
	// opaque copies of every mesh, so glass/water etc. still produce depth.
	// If the world has not been annotated there are no proxies, and a show-only list that is empty would
	// render nothing, so fall back to the regular scene in that case.
	TArray<TWeakObjectPtr<UPrimitiveComponent> > ComponentList;
	if (!bIgnoreTransparentObjects)
	{
		UCVAnnotationCamSensor::GetAnnotationComponents(this->GetWorld(), ComponentList);
	}
	if (ComponentList.Num() > 0)
	{
		this->ShowOnlyComponents = ComponentList;
		this->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
		this->ShowFlags.SetMaterials(false); // This will make annotation component visible
	}
	else
	{
		this->ShowOnlyComponents.Empty();
		this->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
		this->ShowFlags.SetMaterials(true);
	}

	if (!CheckTextureTarget()) return;
	UpdateCineCameraView();
	this->CaptureScene();
	Width = this->TextureTarget->SizeX, Height = TextureTarget->SizeY;
	DepthData.Empty();
	DepthData.AddZeroed(Width * Height); // or AddUninitialized(FloatColorDepthData.Num());
	FTextureRenderTargetResource* RenderTargetResource = this->TextureTarget->GameThread_GetRenderTargetResource();
	TArray<FFloat16Color> FloatColorDepthData;
	RenderTargetResource->ReadFloat16Pixels(FloatColorDepthData);

    ParallelFor(FloatColorDepthData.Num(), [&](int32 i)
    {
        if (i >= 0 && i < FloatColorDepthData.Num() && i < DepthData.Num())
        {
            FFloat16Color& FloatColor = FloatColorDepthData[i];
            DepthData[i] = FloatColor.R;
        }
    });
}
