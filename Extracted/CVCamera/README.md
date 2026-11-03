# CVCamera: UnrealCV's cameras, without the server

A standalone Unreal Engine plugin containing only UnrealCV's camera sensors.
Spawn cameras from your own C++ or Blueprints, capture frames and get the pixel
data back directly. There is no TCP server, command protocol, world controller or
Python client.

| Output | Function | Pixel type | Notes |
|---|---|---|---|
| RGB | `GetLit` | `TArray<FColor>` (BGRA8) | Final tonemapped colour |
| Depth | `GetDepth` | `TArray<float>` | Plane depth (distance along the view axis) in **cm**, stored as float16 internally |
| Surface normal | `GetNormal` | `TArray<FColor>` | World-space normal encoded as colour |
| Optical flow | `GetFlow` | `TArray<FColor>` | Motion vectors encoded as colour |
| Segmentation | `GetSeg` | `TArray<FColor>` | One unique colour per actor (see *Segmentation*) |
| Intrinsics | `GetIntrinsics` | `fx, fy, cx, cy` | Pinhole model in pixels |
| Pose | `GetSensorLocation` / `GetSensorRotation` | `FVector` / `FRotator` | World space, UE units (cm) |

Images are row-major, `Width * Height` pixels, with the top-left pixel first.

Targets UE 5.4 to 5.8, the same range as the UnrealCV source this was extracted from.

## Install

1. Copy this `CVCamera` folder to `YourProject/Plugins/CVCamera`.
2. If your project has C++ code, add the module to `YourProject.Build.cs`:
   ```csharp
   PublicDependencyModuleNames.Add("CVCamera");
   ```
3. Regenerate project files and build.

**Do not enable this alongside the full UnrealCV plugin.** The class names don't clash,
but the two plugins would each annotate the world.

The materials in `Content/` (`WorldNormal`, `OpticalFlowMaterial`, `AnnotationColor`) were
copied from UnrealCV's `/UnrealCV/` mount point and are loaded from `/CVCamera/`. If the
editor warns about a package name mismatch the first time, open each one and save it.

## Usage (C++)

```cpp
#include "CVCameraActor.h"
#include "CVCameraLibrary.h"

// Spawn
ACVCameraActor* Cam = UCVCameraLibrary::SpawnCVCamera(this, FVector(0, 0, 200), FRotator(-10, 0, 0),
                                                      /*Width*/ 1280, /*Height*/ 720, /*FOV*/ 90.f);
UCVFusionCamSensor* Sensor = Cam->GetSensor();

// Move it whenever you like
Sensor->SetSensorLocation(FVector(100, 0, 200));
Sensor->SetSensorRotation(FRotator(0, 45, 0));

// Capture (synchronous: each call renders one frame and reads it back)
int32 W = 0, H = 0;
TArray<FColor> Rgb;    Sensor->GetLit(Rgb, W, H);
TArray<float>  Depth;  Sensor->GetDepth(Depth, W, H);
TArray<FColor> Normal; Sensor->GetNormal(Normal, W, H);
TArray<FColor> Flow;   Sensor->GetFlow(Flow, W, H);
TArray<FColor> Seg;    Sensor->GetSeg(Seg, W, H);

float Fx, Fy, Cx, Cy;
Sensor->GetIntrinsics(Fx, Fy, Cx, Cy);

// Optional: write to disk
UCVCameraLibrary::SaveColorImageAsPng(Rgb, W, H, FPaths::ProjectSavedDir() / TEXT("rgb.png"));
UCVCameraLibrary::SaveFloatImageAsNpy(Depth, W, H, FPaths::ProjectSavedDir() / TEXT("depth.npy"));
```

You can also skip the actor and add a `UCVFusionCamSensor` component to any actor of
your own, for example a drone or vehicle Blueprint. It appears in the *Add Component* menu.

All of the above is Blueprint-callable under the **CVCamera** category.

### Camera settings

`UCVFusionCamSensor` keeps UnrealCV's full set of controls: `SetFilmSize`, `SetSensorFOV`,
`SetProjectionType`/`SetOrthoWidth`, exposure (`SetExposureMethod`, `SetExposureBias`, ...),
motion blur, depth of field, and a physical cine-camera model (`SetCineFilmback`,
`SetCineLens`, `SetCineFocusDistance`, `SetCineExposure`, ...).

## Segmentation

`UCVAnnotationSubsystem` (a world subsystem) gives every actor that has a mesh a
unique colour when play begins. That colour is what appears in `GetSeg`.

```cpp
UCVAnnotationSubsystem* Ann = GetWorld()->GetSubsystem<UCVAnnotationSubsystem>();

TMap<FString, FColor> Colors = Ann->GetAnnotationColors();   // actor name -> colour
AActor* Hit = Ann->FindActorByColor(Seg[Y * W + X]);         // pixel -> actor
Ann->SetAnnotationColor(SomeActor, FColor(255, 0, 0));       // choose your own colours / classes

// Actors spawned after BeginPlay are not annotated automatically:
Ann->AnnotateActor(NewlySpawnedActor);
```

To disable automatic annotation, add this to `Config/DefaultGame.ini`:

```ini
[/Script/CVCamera.CVAnnotationSubsystem]
bAutoAnnotateOnBeginPlay=False
```

By default, depth is rendered from the same annotation proxies, so transparent surfaces
(glass, water) still have depth. If the world has not been annotated, depth falls back to
the normal scene. Set `bIgnoreTransparentObjects` on the depth sensor to always use the
normal scene.

## Performance

Every `Get*` call renders the scene once more and waits for the GPU (`ReadPixels`). Each
modality is a separate render, so capturing all five costs five renders. Only request the
outputs you need. Keep captures on the game thread, for example in `Tick` or a timer.

## What was removed or changed compared to UnrealCV

- Removed the TCP server, command dispatcher, `vset`/`vget` commands, world controller,
  player view modes and Python client.
- Camera defaults (640x480, FOV 90) are hard-coded instead of read from `unrealcv.ini`.
  Change them per instance in the Details panel or call `SetFilmSize` / `SetSensorFOV`.
- `FObjectAnnotator` became `UCVAnnotationSubsystem`.
- Depth no longer returns an empty frame when the world has not been annotated.
- Fixed a small static-array overflow in the colour palette generator. The colours it
  produces are unchanged.
- Every class has a `CV` prefix (`ACVCameraActor`, `UCVFusionCamSensor`, ...), and the log
  category is `LogCVCamera`.

## License

MIT, same as UnrealCV (see `LICENSE`). `OpticalFlowMaterial` has its own license; see
`Content/OpticalFlowMaterial.license.txt`.
