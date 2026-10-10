#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/HUD.h"
#include "GameFramework/SaveGame.h"
#include "RunGame.generated.h"

UCLASS()
class SEASONRUN_API URunSave : public USaveGame {
 GENERATED_BODY()
public:
 UPROPERTY() int32 BestMeters=0;
 UPROPERTY() int32 BestCoins=0;
 UPROPERTY() int32 TotalCoins=0;
};

UCLASS()
class SEASONRUN_API ARunCharacter : public ACharacter {
 GENERATED_BODY()
public:
 ARunCharacter();
 virtual void BeginPlay() override;
 virtual void Tick(float Delta) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 void SaveProgress();
 UPROPERTY() URunSave* Progress;
 float SaveTimer=0, LoopDistance=0;
 int32 SavedRunCoins=0;
 int32 Lane=0, Hearts=3, Coins=0;
 bool Started=false, Paused=false, Finished=false, Won=false, MusicMuted=false;
 float Distance=0, Invulnerable=0, HitFlash=0, Elapsed=0, Speed=1000, Boost=0, TurboExtra=0;
 static constexpr float LoopLength=120000.f;
};

UCLASS()
class SEASONRUN_API ARunCourse : public AActor {
 GENERATED_BODY()
public:
 ARunCourse();
 virtual void BeginPlay() override;
 virtual void Tick(float Delta) override;
private:
 int32 VerifyStage=0, VerifyHeart=-1;
 int32 TrailerPit=-1;
 float TrailerJumpAt=-1;
 enum class EKind { Coin, Snowman, Mudman, Boost, Heart };
 struct FItem { float X,Y; EKind Kind; bool Done=false; int32 Index; float Z=105; };
 struct FPit { float Start,End,Y; bool Done=false,Wide=false; };
 struct FPart { class UInstancedStaticMeshComponent* Mesh; int32 Index; FTransform Pose; bool Arm=false; };
 struct FEnemy { float X,Y; bool Mud=false; float LastRise=-1,BreakAge=-1; TArray<FPart> Parts; };
 UPROPERTY() class APostProcessVolume* PostProcess;
 void MakeNightVillage();
 void ResetLoop();
 void UpdateDaylight(float Time);
 UPROPERTY() class ADirectionalLight* SunLight;
 UPROPERTY() class ASkyLight* AmbientLight;
 UPROPERTY() class AExponentialHeightFog* CycleFog;
 UPROPERTY() class UMaterialInstanceDynamic* CycleSky;
 UPROPERTY() class UMaterialInstanceDynamic* CycleStars;
 UPROPERTY() class UInstancedStaticMeshComponent* NightStars;
 UPROPERTY() class UInstancedStaticMeshComponent* NightLinks;
 UPROPERTY() class UInstancedStaticMeshComponent* HeartLobes;
 UPROPERTY() class UInstancedStaticMeshComponent* HeartTips;
 TArray<FTransform> CoinPoses,ArrowPoses,HeartLobePoses,HeartTipPoses;

 TArray<FPit> Pits;
 TArray<FEnemy> Enemies;
 UPROPERTY() class UAudioComponent* Music;
 UPROPERTY() class USoundWave* CoinChime;
 UPROPERTY() class USoundWave* TurboWhoosh;
 UPROPERTY() class UInstancedStaticMeshComponent* BoostArrows;
 TArray<FItem> Items;
 FVector PreviousRunnerPosition=FVector(0,0,80);
 UPROPERTY() class UInstancedStaticMeshComponent* Obstacles;
 UPROPERTY() class UInstancedStaticMeshComponent* Coins;
 UPROPERTY() class UMaterialInstanceDynamic* Snow;
 UPROPERTY() class UMaterialInstanceDynamic* Dark;
 UPROPERTY() class UMaterialInstanceDynamic* Gold;
 class UInstancedStaticMeshComponent* Instances(const TCHAR* Path, class UMaterialInterface* Mat);
 void Shape(const TCHAR* Path,FVector Position,FVector Scale,class UMaterialInterface* Mat,FRotator Rotation=FRotator::ZeroRotator,bool Collision=false);
};

UCLASS()
class SEASONRUN_API ARunHUD : public AHUD {
 GENERATED_BODY()
public: virtual void DrawHUD() override;
};

UCLASS()
class SEASONRUN_API ARunGameMode : public AGameModeBase {
 GENERATED_BODY()
public:
 ARunGameMode();
 virtual void BeginPlay() override;
};
