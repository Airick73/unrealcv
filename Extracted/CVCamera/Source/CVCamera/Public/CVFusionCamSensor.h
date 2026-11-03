// Weichao Qiu @ 2017
#pragma once

#include "Runtime/Engine/Classes/Components/PrimitiveComponent.h"
#include "Runtime/Engine/Classes/Camera/CameraTypes.h"
#include "CVFusionCamSensor.generated.h"

class UCineCameraComponent;

UENUM(BlueprintType)
enum class ECVLitMode : uint8
{
	Lit,
	Slow
};

UENUM(BlueprintType)
enum class ECVDepthMode : uint8
{
	PlaneDepth,
	DistToCamCenter
};

UENUM(BlueprintType)
enum class ECVSegMode : uint8
{
	AnnotationComponent,
	VertexColor,
	CustomStencil
};

UENUM(BlueprintType)
enum class ECVPresetFilmSize : uint8
{
	F640x480,
	F720p,
	F1080p
};

UCLASS(meta = (BlueprintSpawnableComponent))
class CVCAMERA_API UCVFusionCamSensor : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UCVFusionCamSensor(const FObjectInitializer& ObjectInitializer);

	void checkFusionSensors();

	// virtual void OnRegister() override;
	virtual bool GetEditorPreviewInfo(float DeltaTime, FMinimalViewInfo& ViewOut);

	/** Get rgb data */
	UFUNCTION(BlueprintPure, Category = "CVCamera")
	void GetLit(TArray<FColor>& LitData, int& InOutWidth, int& InOutHeight, ECVLitMode LitMode = ECVLitMode::Lit);

	/** Get depth data */
	UFUNCTION(BlueprintPure, Category = "CVCamera")
	void GetDepth(TArray<float>& DepthData, int& InOutWidth, int& InOutHeight, ECVDepthMode DepthMode = ECVDepthMode::PlaneDepth);

	/** Get surface normal data */
	UFUNCTION(BlueprintPure, Category = "CVCamera")
	void GetNormal(TArray<FColor>& NormalData, int& Width, int& Height);

	/** Get optical flow data */
	UFUNCTION(BlueprintPure, Category = "CVCamera")
	void GetFlow(TArray<FColor>& FlowData, int& Width, int& Height);

	/** Get object mask data, the annotation color can be extracted from FObjectAnnotator */
	UFUNCTION(BlueprintPure, Category = "CVCamera")
	void GetSeg(TArray<FColor>& ObjMaskData, int& Width, int& Height, ECVSegMode SegMode = ECVSegMode::AnnotationComponent);

	UFUNCTION(BlueprintPure, Category = "CVCamera")
	FVector GetSensorLocation();

	UFUNCTION(BlueprintPure, Category = "CVCamera")
	FRotator GetSensorRotation();

	UFUNCTION(BlueprintCallable, Category = "CVCamera")
	void SetSensorLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "CVCamera")
	void SetSensorRotation(FRotator Rotator);

	UFUNCTION(BlueprintCallable, Category = "CVCamera")
	void SetFilmSize(int Width, int Height);

	UFUNCTION(BlueprintCallable, Category = "CVCamera")
	float GetFilmWidth() { return FilmWidth; }

	UFUNCTION(BlueprintCallable, Category = "CVCamera")
	float GetFilmHeight() { return FilmHeight; }

	UFUNCTION(BlueprintCallable, Category = "CVCamera")
	float GetSensorFOV();

	UFUNCTION(BlueprintCallable, Category = "CVCamera")
	void SetSensorFOV(float FOV);

	/** Pinhole intrinsics (pixels) for the current film size and FOV. Ignores cine lens offsets. */
	UFUNCTION(BlueprintPure, Category = "CVCamera")
	void GetIntrinsics(float& Fx, float& Fy, float& Cx, float& Cy);

	UFUNCTION(BlueprintCallable, Category = "CVCamera|cine camera")
	void SetCineCameraEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "CVCamera|cine camera")
	bool IsCineCameraEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "CVCamera|cine camera")
	void SetCineFilmback(float SensorWidth, float SensorHeight, float HorizontalOffset = 0.0f, float VerticalOffset = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "CVCamera|cine camera")
	void SetCineLens(float FocalLength, float Aperture);

	UFUNCTION(BlueprintCallable, Category = "CVCamera|cine camera")
	void SetCineLensSettings(float MinFocalLength, float MaxFocalLength, float MinFStop, float MaxFStop,
		float MinimumFocusDistance, float SqueezeFactor, int32 DiaphragmBladeCount);

	UFUNCTION(BlueprintCallable, Category = "CVCamera|cine camera")
	void SetCineFocusDistance(float FocusDistance);

	bool SetCineFocusMode(const FString& FocusMode, bool bSmoothFocusChanges, float SmoothingSpeed, float FocusOffset);
	bool SetCineTrackingFocus(AActor* ActorToTrack, const FVector& RelativeOffset);

	UFUNCTION(BlueprintCallable, Category = "CVCamera|cine camera")
	void SetCineCrop(float CropAspectRatio, float Overscan, bool bCropOverscan, bool bScaleResolutionWithOverscan);

	UFUNCTION(BlueprintCallable, Category = "CVCamera|cine camera")
	void SetCineNearClip(bool bEnabled, float NearClipDistance);

	UFUNCTION(BlueprintCallable, Category = "CVCamera|cine camera")
	void SetCineExposure(float ISO, float ShutterSpeed, bool bApplyPhysicalExposure);

	UCineCameraComponent* GetCineCameraComponent() const;

	UFUNCTION(BlueprintCallable, Category = "CVCamera")
	void SetProjectionType(ECameraProjectionMode::Type ProjectionType);

	UFUNCTION(BlueprintCallable, Category = "CVCamera")
	void SetOrthoWidth(float OrthoWidth);

	UFUNCTION(BlueprintCallable, Category = "CVCamera")
	void SetLitCaptureSource(ESceneCaptureSource CaptureSource);

    UFUNCTION(BlueprintCallable, Category = "CVCamera")
    void SetReflectionMethod(EReflectionMethod::Type Method);

    UFUNCTION(BlueprintCallable, Category = "CVCamera")
    void SetGlobalIlluminationMethod(EDynamicGlobalIlluminationMethod::Type Method);

    UFUNCTION(BlueprintCallable, Category = "CVCamera")
	void SetExposureMethod(EAutoExposureMethod ExposureMethod);

	UFUNCTION(BlueprintCallable, Category = "CVCamera")
	void SetExposureBias(float ExposureBias);

	UFUNCTION(BlueprintCallable, Category = "CVCamera")
	void SetAutoExposureSpeed(float ExposureSpeedDown, float ExposureSpeedUp);

	UFUNCTION(BlueprintCallable, Category = "CVCamera")
	void SetAutoExposureBrightness(float MinBrightness, float MaxBrightness);

	UFUNCTION(BlueprintCallable, Category = "CVCamera")
	void SetApplyPhysicalCameraExposure(int ApplyPhysicalCameraExposure);

	UFUNCTION(BlueprintCallable, Category = "CVCamera")
	void SetMotionBlurParams(float MotionBlurAmount, float MotionBlurMax, float MotionBlurPerObjectSize, int MotionBlurTargetFPS);

    UFUNCTION(BlueprintCallable, Category = "CVCamera")
    void SetFocalParams(float FocalDistance, float FocalRegion);
	// UFUNCTION(BlueprintPure, Category = "CVCamera")
	// float GetFilmHeight();

	// UFUNCTION(BlueprintPure, Category = "CVCamera")
	// float GetFilmWidth();

	// void SetFilmSize(int Width, int Height);

	UPROPERTY(meta = (AllowPrivateAccess= "true"))
	class UCameraComponent* PreviewCamera;

	virtual void BeginPlay() override;


	static TArray<UCVFusionCamSensor*> GetComponents(AActor* Actor);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;
#endif

private:
	UPROPERTY(EditInstanceOnly, meta=(AllowPrivateAccess = "true"), Category = "CVCamera")
	ECVPresetFilmSize PresetFilmSize;

	UPROPERTY(EditInstanceOnly, meta=(AllowPrivateAccess = "true"), Category = "CVCamera")
	int FilmWidth;

	UPROPERTY(EditInstanceOnly, meta=(AllowPrivateAccess = "true"), Category = "CVCamera")
	int FilmHeight;

	UPROPERTY(EditInstanceOnly, meta=(AllowPrivateAccess = "true"), Category = "CVCamera")
	float FOV;

protected:
	TArray<UCineCameraComponent*> GetCineCameraComponents() const;

	UPROPERTY()
	TArray<class UCVBaseCameraSensor*> FusionSensors;

	UPROPERTY(EditDefaultsOnly, Category = "CVCamera")
	class UCVDepthCamSensor* DepthCamSensor;

	UPROPERTY(EditDefaultsOnly, Category = "CVCamera")
	class UCVNormalCamSensor* NormalCamSensor;

	UPROPERTY(EditDefaultsOnly, Category = "CVCamera")
	class UCVAnnotationCamSensor* AnnotationCamSensor;

	UPROPERTY(EditDefaultsOnly, Category = "CVCamera")
	class UCVLitCamSensor* LitCamSensor;

	/* LogOutputDevice: Error: Ensure condition failed: false  [File:D:\build\++UE5\Sync\Engine\Source\Runtime\Engine\Private\Components\SceneComponent.cpp] [Line: 2104] 
	 * LogOutputDevice: Error: Template Mismatch during attachment. Attaching instanced component to template component. Parent 'FusionCamSensor_GEN_VARIABLE' (Owner 'None') Self 'FusionCamSensor_GEN_VARIABLE_FlowCamSensor' (Owner 'BP_Drone01_C_1').
	 * 'FusionCamSensor_1_AnnotationCamSensor': FusionCamSensor /Game/SuburbNeighborhood_Day_dooropen.SuburbNeighborhood_Day_dooropen:PersistentLevel.BP_Drone01_C_1.FusionCamSensor
	 * 'FusionCamSensor_1_PreviewCamera': FusionCamSensor /Game/SuburbNeighborhood_Day_dooropen.SuburbNeighborhood_Day_dooropen:PersistentLevel.BP_Drone01_C_1.FusionCamSensor
	 * 'FusionCamSensor_1_DepthCamSensor': FusionCamSensor /Game/SuburbNeighborhood_Day_dooropen.SuburbNeighborhood_Day_dooropen:PersistentLevel.BP_Drone01_C_1.FusionCamSensor
	 * 'FusionCamSensor_1_LitCamSensor': FusionCamSensor /Game/SuburbNeighborhood_Day_dooropen.SuburbNeighborhood_Day_dooropen:PersistentLevel.BP_Drone01_C_1.FusionCamSensor
	 * 'FusionCamSensor_1_NormalCamSensor': FusionCamSensor /Game/SuburbNeighborhood_Day_dooropen.SuburbNeighborhood_Day_dooropen:PersistentLevel.BP_Drone01_C_1.FusionCamSensor
	 */
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CVCamera")
	UPROPERTY(EditDefaultsOnly, Category = "CVCamera")
	class UCVFlowCamSensor* FlowCamSensor;


	/** This preview camera is used for UE version < 4.17 which only support UCameraComponent PIP preview
	See the difference between
	https://github.com/EpicGames/UnrealEngine/blob/4.17/Engine/Source/Editor/LevelEditor/Private/SLevelViewport.cpp#L3927
	and
	https://github.com/EpicGames/UnrealEngine/blob/4.16/Engine/Source/Editor/LevelEditor/Private/SLevelViewport.cpp#L3908
	*/


};
