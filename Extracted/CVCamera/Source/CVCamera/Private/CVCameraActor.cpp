// Extracted from UnrealCV (https://github.com/unrealcv/unrealcv), MIT License.
#include "CVCameraActor.h"

ACVCameraActor::ACVCameraActor()
{
	PrimaryActorTick.bCanEverTick = false;

	FusionCamSensor = CreateDefaultSubobject<UCVFusionCamSensor>(TEXT("FusionCamSensor"));
	RootComponent = FusionCamSensor;
}
