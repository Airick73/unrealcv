// Weichao Qiu @ 2017
#include "Sensors/CVBaseCameraSensor.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/Engine/CollisionProfile.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "TextureResource.h"
#include "CVCameraLog.h"

DECLARE_CYCLE_STAT(TEXT("ReadBuffer"), STAT_ReadBuffer, STATGROUP_CVCamera);

UCVBaseCameraSensor::UCVBaseCameraSensor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CineCameraModel = CreateDefaultSubobject<UCineCameraComponent>(TEXT("CineCameraModel"));
	CineCameraModel->SetupAttachment(this);
	FCameraLensSettings CineLens = CineCameraModel->LensSettings;
	CineLens.MinFocalLength = 1.0f;
	CineLens.MaxFocalLength = 1000.0f;
	CineLens.MinFStop = 0.1f;
	CineLens.MaxFStop = 64.0f;
	CineCameraModel->SetLensSettings(CineLens);
	// static ConstructorHelpers::FObjectFinder<UStaticMesh> EditorCameraMesh(TEXT("/Engine/EditorMeshes/MatineeCam_SM"));
	// Another choice is "StaticMesh'/Engine/EditorMeshes/Camera/SM_CineCam.SM_CineCam'"
	this->ShowFlags.SetPostProcessing(true);
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
	PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	bUseRayTracingIfEnabled = true;
	bAlwaysPersistRenderingState = true;

	// Defaults. The owning UCVFusionCamSensor pushes its own values in BeginPlay.
	FilmWidth = 640;
	FilmHeight = 480;
	FOVAngle = 90;
	CineCameraModel->SetFieldOfView(FOVAngle);
	// Avoid calling virtual function in a constructor
}

float UCVBaseCameraSensor::GetFOV() const
{
	return bCineCameraEnabled && IsValid(CineCameraModel)
		? CineCameraModel->GetHorizontalFieldOfView()
		: FOVAngle;
}

void UCVBaseCameraSensor::SetFOV(float FOV)
{
	const float SafeFOV = FMath::Clamp(FOV, 1.0f, 179.0f);
	if (bCineCameraEnabled && IsValid(CineCameraModel))
	{
		CineCameraModel->SetFieldOfView(SafeFOV);
		UpdateCineCameraView();
		return;
	}
	FOVAngle = SafeFOV;
	if (IsValid(CineCameraModel))
	{
		CineCameraModel->SetFieldOfView(SafeFOV);
	}
}

void UCVBaseCameraSensor::SetCineCameraEnabled(bool bEnabled)
{
	if (bEnabled == bCineCameraEnabled)
	{
		if (bEnabled)
		{
			UpdateCineCameraView();
		}
		return;
	}

	if (bEnabled)
	{
		LegacyPostProcessSettings = PostProcessSettings;
		LegacyPostProcessBlendWeight = PostProcessBlendWeight;
		bHasLegacyPostProcessState = true;
		bCineCameraEnabled = true;
		UpdateCineCameraView();
	}
	else
	{
		bCineCameraEnabled = false;
		bUseCustomProjectionMatrix = false;
		if (bHasLegacyPostProcessState)
		{
			PostProcessSettings = LegacyPostProcessSettings;
			PostProcessBlendWeight = LegacyPostProcessBlendWeight;
			bHasLegacyPostProcessState = false;
		}
	}
}

void UCVBaseCameraSensor::UpdateCineCameraView(float DeltaTime)
{
	if (!bCineCameraEnabled || !IsValid(CineCameraModel))
	{
		bUseCustomProjectionMatrix = false;
		return;
	}

	CineCameraModel->SetWorldLocationAndRotation(GetComponentLocation(), GetComponentRotation());
	const FPostProcessSettings CineOverrides = CineCameraModel->PostProcessSettings;
	CineCameraModel->PostProcessSettings = bHasLegacyPostProcessState
		? LegacyPostProcessSettings
		: PostProcessSettings;
	if (CineOverrides.bOverride_CameraISO)
	{
		CineCameraModel->PostProcessSettings.bOverride_CameraISO = true;
		CineCameraModel->PostProcessSettings.CameraISO = CineOverrides.CameraISO;
	}
	if (CineOverrides.bOverride_CameraShutterSpeed)
	{
		CineCameraModel->PostProcessSettings.bOverride_CameraShutterSpeed = true;
		CineCameraModel->PostProcessSettings.CameraShutterSpeed = CineOverrides.CameraShutterSpeed;
	}
	if (CineOverrides.bOverride_AutoExposureApplyPhysicalCameraExposure)
	{
		CineCameraModel->PostProcessSettings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
		CineCameraModel->PostProcessSettings.AutoExposureApplyPhysicalCameraExposure =
			CineOverrides.AutoExposureApplyPhysicalCameraExposure;
	}
	CineCameraModel->PostProcessBlendWeight = PostProcessBlendWeight;

	FMinimalViewInfo ViewInfo;
	CineCameraModel->GetCameraView(DeltaTime, ViewInfo);
	const int32 Width = FMath::Max(GetFilmWidth(), 1);
	const int32 Height = FMath::Max(GetFilmHeight(), 1);
	ViewInfo.AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
	ViewInfo.bConstrainAspectRatio = false;
	CustomProjectionMatrix = ViewInfo.CalculateProjectionMatrix();
	bUseCustomProjectionMatrix = true;
	PostProcessSettings = ViewInfo.PostProcessSettings;
	PostProcessBlendWeight = ViewInfo.PostProcessBlendWeight;
}

// Explicitly make a request to render frames
// This is needed if we want to disable bCaptureEveryFrame
// https://answers.unrealengine.com/questions/723947/scene-capture-with-post-process-mat-works-only-wit.html?sort=oldest

// if (GetOwner()) // Check whether this is a template project
// if (!IsTemplate())

// NOTE: Avoid creating TextureTarget in the CTOR, this will make CamSensor not savable in a BP actor
// TextureTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("CamSensorRenderTarget"));

void UCVBaseCameraSensor::InitTextureTarget(int filmWidth, int filmHeight)
{
	// bool bUseLinearGamma = false;
	EPixelFormat PixelFormat = EPixelFormat::PF_B8G8R8A8;
	bool bUseLinearGamma = false;
	TextureTarget = NewObject<UTextureRenderTarget2D>(this);
	TextureTarget->InitCustomFormat(filmWidth, filmHeight, PixelFormat, bUseLinearGamma);
}

void UCVBaseCameraSensor::SetFilmSize(int Width, int Height)
{
	this->FilmWidth = Width;
	this->FilmHeight = Height;
	if (!IsValid(TextureTarget))
	{
		TextureTarget = NewObject<UTextureRenderTarget2D>(this);
		// TextureTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("CamSensorRenderTarget"));
	}

	if (TextureTarget->SizeX != Width || TextureTarget->SizeY != Height)
	{
		InitTextureTarget(Width, Height);
	}
	UpdateCineCameraView();
}

int UCVBaseCameraSensor::GetFilmWidth()
{
	if (!IsValid(TextureTarget)) return 0;
	return TextureTarget->SizeX;
}

int UCVBaseCameraSensor::GetFilmHeight()
{
	if (!IsValid(TextureTarget)) return 0;
	return TextureTarget->SizeY;
}


// TODO: Split the logic, move data serialization code outside
// Serialize the data to png and npy, check the speed.

// EPixelFormat::PF_B8G8R8A8
/*
This is defined in FColor
	#ifdef _MSC_VER
	// Win32 x86
	union { struct{ uint8 B,G,R,A; }; uint32 AlignmentDummy; };
#else
	// Linux x86, etc
	uint8 B GCC_ALIGN(4);
	uint8 G,R,A;
*/
bool UCVBaseCameraSensor::CheckTextureTarget()
{
	if (!IsValid(TextureTarget))
	{
		UE_LOG(LogTemp, Warning, TEXT("The TextureTarget was not initialized."));
		return false;
	}
	if (TextureTarget->SizeX == 0 || TextureTarget->SizeY == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("The TextureTarget has invalid size."));
		return false;
	}
	return true;
}

void UCVBaseCameraSensor::Capture(TArray<FColor>& ImageData, int& Width, int& Height)
{
	SCOPE_CYCLE_COUNTER(STAT_ReadBuffer);

	if (!CheckTextureTarget())
	{
		UE_LOG(LogTemp, Error, TEXT("The TextureTarget was not initialized. Capture failed."));
		return;
	}
	UpdateCineCameraView();
	this->CaptureScene();

	Width = TextureTarget->SizeX;
	Height = TextureTarget->SizeY;
	ImageData.Empty();
	ImageData.AddZeroed(Width * Height);

	FTextureRenderTargetResource* RenderTargetResource = TextureTarget->GameThread_GetRenderTargetResource();
	FReadSurfaceDataFlags ReadSurfaceDataFlags;
	ReadSurfaceDataFlags.SetLinearToGamma(false);
	RenderTargetResource->ReadPixels(ImageData, ReadSurfaceDataFlags);
}

void UCVBaseCameraSensor::SetPostProcessMaterial(UMaterial* PostProcessMaterial)
{
	PostProcessSettings.AddBlendable(PostProcessMaterial, 1);
}

void UCVBaseCameraSensor::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	if (bCineCameraEnabled && IsValid(CineCameraModel))
	{
		CineCameraModel->SetWorldLocationAndRotation(GetComponentLocation(), GetComponentRotation());
		CineCameraModel->GetCameraView(DeltaTime, DesiredView);
		return;
	}
	DesiredView.Location = GetComponentLocation();
	DesiredView.Rotation = GetComponentRotation();
	DesiredView.FOV = this->FOVAngle;
	// DesiredView.FOV = FieldOfView;
	// DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
	// DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
	// DesiredView.ProjectionMode = ProjectionMode;
	DesiredView.ProjectionMode = ECameraProjectionMode::Perspective;
	DesiredView.OrthoWidth = OrthoWidth;
	// DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
	// DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;

	// See if the CameraActor wants to override the PostProcess settings used.
	DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
	if (PostProcessBlendWeight > 0.0f)
	{
		DesiredView.PostProcessSettings = PostProcessSettings;
	}

}
