// Extracted from UnrealCV (https://github.com/unrealcv/unrealcv), MIT License.
#include "CVCameraLibrary.h"
#include "CVCameraActor.h"
#include "CVCameraLog.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Modules/ModuleManager.h"

ACVCameraActor* UCVCameraLibrary::SpawnCVCamera(UObject* WorldContextObject, FVector Location, FRotator Rotation,
	int32 Width, int32 Height, float FOV)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACVCameraActor* Camera = World->SpawnActor<ACVCameraActor>(Location, Rotation, Params);
	if (!IsValid(Camera))
	{
		UE_LOG(LogCVCamera, Error, TEXT("Failed to spawn ACVCameraActor"));
		return nullptr;
	}

	// BeginPlay (which runs during SpawnActor in a playing world) applies the defaults; override them here.
	Camera->GetSensor()->SetFilmSize(Width, Height);
	Camera->GetSensor()->SetSensorFOV(FOV);
	return Camera;
}

bool UCVCameraLibrary::SaveColorImageAsPng(const TArray<FColor>& Pixels, int32 Width, int32 Height, const FString& Filename)
{
	if (Width <= 0 || Height <= 0 || Pixels.Num() != Width * Height)
	{
		UE_LOG(LogCVCamera, Warning, TEXT("SaveColorImageAsPng: %d pixels does not match %dx%d"), Pixels.Num(), Width, Height);
		return false;
	}
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> Png = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	if (!Png.IsValid() || !Png->SetRaw(Pixels.GetData(), Pixels.Num() * sizeof(FColor), Width, Height, ERGBFormat::BGRA, 8))
	{
		return false;
	}
	const TArray64<uint8>& Compressed = Png->GetCompressed();
	return FFileHelper::SaveArrayToFile(Compressed, *Filename);
}

bool UCVCameraLibrary::SaveFloatImageAsNpy(const TArray<float>& Pixels, int32 Width, int32 Height, const FString& Filename)
{
	if (Width <= 0 || Height <= 0 || Pixels.Num() != Width * Height)
	{
		UE_LOG(LogCVCamera, Warning, TEXT("SaveFloatImageAsNpy: %d pixels does not match %dx%d"), Pixels.Num(), Width, Height);
		return false;
	}

	// NPY v1.0: magic, version, uint16 header length, then a Python dict literal padded to a multiple of 64 bytes.
	FString Dict = FString::Printf(TEXT("{'descr': '<f4', 'fortran_order': False, 'shape': (%d, %d), }"), Height, Width);
	const int32 Preamble = 10; // 6 magic + 2 version + 2 length
	const int32 TotalHeader = Preamble + Dict.Len() + 1; // + trailing newline
	const int32 Padding = (64 - TotalHeader % 64) % 64;
	Dict += FString::ChrN(Padding, TEXT(' '));
	Dict += TEXT("\n");

	TArray<uint8> Bytes;
	const uint8 Magic[] = { 0x93, 'N', 'U', 'M', 'P', 'Y', 0x01, 0x00 };
	Bytes.Append(Magic, sizeof(Magic));
	const uint16 HeaderLen = static_cast<uint16>(Dict.Len());
	Bytes.Add(HeaderLen & 0xFF);
	Bytes.Add((HeaderLen >> 8) & 0xFF);
	FTCHARToUTF8 DictUtf8(*Dict);
	Bytes.Append(reinterpret_cast<const uint8*>(DictUtf8.Get()), DictUtf8.Length());
	// Little-endian float32 payload (all UE target platforms are little-endian).
	Bytes.Append(reinterpret_cast<const uint8*>(Pixels.GetData()), Pixels.Num() * sizeof(float));

	return FFileHelper::SaveArrayToFile(Bytes, *Filename);
}
