// shc @ 2025
#pragma once

#include "Sensors/CVBaseCameraSensor.h"
#include "CVFlowCamSensor.generated.h"

/* Optical flow sensor. ESceneCaptureSource has no optical-flow output, so this sensor renders the
 * scene through the '/CVCamera/OpticalFlowMaterial' post-process material, which encodes motion
 * vectors as colour.
 */

UCLASS()
class CVCAMERA_API UCVFlowCamSensor : public UCVBaseCameraSensor
{
	GENERATED_BODY()

public:
	UCVFlowCamSensor(const FObjectInitializer& ObjectInitializer);

	void SetFilmSize(int Width, int Height);

	// virtual void InitTextureTarget(int FilmWidth, int FilmHeight) override;

	// void CaptureFlow(TArray<FColor>& Image, int& Width, int& Height);

private:
	/** Post process material to extract optical flow data */
	UPROPERTY()
	UMaterial* OpticalFlowPPMaterial;
};
