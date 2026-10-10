#include "RunGame.h"
#include "RunRules.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "RenderUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/AnimSequence.h"
#include "Kismet/GameplayStatics.h"
#include "InputCoreTypes.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"

namespace {
FString RunSaveSlot() { return FParse::Param(FCommandLine::Get(),TEXT("RunnerVerify"))?TEXT("SeasonRunVerification"):TEXT("SeasonRunProgress"); }
}
ARunCharacter::ARunCharacter() {
 PrimaryActorTick.bCanEverTick=true;
 GetCapsuleComponent()->InitCapsuleSize(34,65);
 GetCharacterMovement()->MaxWalkSpeed=1000;
 GetCharacterMovement()->JumpZVelocity=650;
 GetCharacterMovement()->GravityScale=1.7;
 GetCharacterMovement()->AirControl=1;
 GetCharacterMovement()->MaxAcceleration=15000;
 GetCharacterMovement()->BrakingDecelerationWalking=8000;
 JumpMaxCount=2;
 bUseControllerRotationYaw=false;
 GetMesh()->SetRelativeLocation(FVector(0,0,-65));
 GetMesh()->SetRelativeRotation(FRotator(0,-90,0));
 GetMesh()->SetSkeletalMesh(LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/StackOBot/Characters/Bot/Mesh/SKM_Bot.SKM_Bot")));
 GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 USpringArmComponent* Arm=CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
 Arm->SetupAttachment(GetRootComponent()); Arm->TargetArmLength=660;
 Arm->SetRelativeLocation(FVector(0,0,155)); Arm->SetRelativeRotation(FRotator(-8,0,0));
 Arm->bDoCollisionTest=false; Arm->bInheritPitch=false; Arm->bInheritYaw=false; Arm->bInheritRoll=false;
 UCameraComponent* Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
 Camera->SetupAttachment(Arm); Camera->FieldOfView=76;
}
void ARunCharacter::BeginPlay() {
 Super::BeginPlay();
 Progress=Cast<URunSave>(UGameplayStatics::LoadGameFromSlot(RunSaveSlot(),0));
 if(!Progress) Progress=Cast<URunSave>(UGameplayStatics::CreateSaveGameObject(URunSave::StaticClass()));
 if(FParse::Param(FCommandLine::Get(),TEXT("RunnerVerify"))) Progress=NewObject<URunSave>(this);
 if (auto* Anim=LoadObject<UAnimSequence>(nullptr,TEXT("/Game/StackOBot/Characters/Bot/Animations/A_Bot_Run.A_Bot_Run"))) GetMesh()->PlayAnimation(Anim,true);
}
void ARunCharacter::SaveProgress() {
 if(!Progress) return;
 Progress->BestMeters=FMath::Max(Progress->BestMeters,FMath::Max(0,FMath::FloorToInt(Distance/100)));
 Progress->BestCoins=FMath::Max(Progress->BestCoins,Coins);
 Progress->TotalCoins+=FMath::Max(0,Coins-SavedRunCoins); SavedRunCoins=Coins;
 if(!UGameplayStatics::SaveGameToSlot(Progress,RunSaveSlot(),0)) UE_LOG(LogTemp,Warning,TEXT("RUNNER_SAVE: could not save progress"));
}
void ARunCharacter::EndPlay(const EEndPlayReason::Type Reason) { SaveProgress(); Super::EndPlay(Reason); }
void ARunCharacter::Tick(float Dt) {
 Super::Tick(Dt);
 auto* PC=Cast<APlayerController>(GetController()); if (!PC) return;
 if (PC->WasInputKeyJustPressed(EKeys::R)) { SaveProgress(); UGameplayStatics::OpenLevel(this,FName("/Engine/Maps/Entry")); return; }
 if (PC->WasInputKeyJustPressed(EKeys::M)) MusicMuted=!MusicMuted;
 if (!Started && PC->WasInputKeyJustPressed(EKeys::Enter)) Started=true;
 if (Started && !Finished && PC->WasInputKeyJustPressed(EKeys::P)) {Paused=!Paused;SaveProgress();}
 if (!Started || Finished || Paused) { GetCharacterMovement()->StopMovementImmediately(); GetMesh()->GlobalAnimRateScale=0; HitFlash=FMath::Max(0.f,HitFlash-Dt); return; }
 Elapsed+=Dt; Boost=FMath::Max(0.f,Boost-Dt);
 Speed=FMath::FInterpConstantTo(Speed,RunRules::Speed(Elapsed,TurboExtra),Dt,120.f);
 GetCharacterMovement()->MaxWalkSpeed=Speed;
 GetMesh()->GlobalAnimRateScale=FMath::Min(2.5f,Speed/1000);
 if(auto* Camera=FindComponentByClass<UCameraComponent>()) Camera->FieldOfView=FMath::FInterpTo(Camera->FieldOfView,FMath::Lerp(76.f,96.f,FMath::Clamp(TurboExtra/1800.f,0.f,1.f)),Dt,3.f);
 if (PC->WasInputKeyJustPressed(EKeys::A)||PC->WasInputKeyJustPressed(EKeys::Left)) Lane=FMath::Max(-1,Lane-1);
 if (PC->WasInputKeyJustPressed(EKeys::D)||PC->WasInputKeyJustPressed(EKeys::Right)) Lane=FMath::Min(1,Lane+1);
 if (PC->WasInputKeyJustPressed(EKeys::SpaceBar)||PC->WasInputKeyJustPressed(EKeys::Up)) {StopJumping(); Jump();}
 float Y=FMath::Clamp((Lane*220.f-GetActorLocation().Y)*0.02f,-1.f,1.f);
 AddMovementInput(FVector(1,Y,0).GetSafeNormal());
 Distance=LoopDistance+GetActorLocation().X;
 SaveTimer+=Dt; if(SaveTimer>=5) {SaveTimer=0;SaveProgress();}
 Invulnerable=FMath::Max(0.f,Invulnerable-Dt); HitFlash=FMath::Max(0.f,HitFlash-Dt);
 GetMesh()->SetVisibility(Invulnerable<=0 || FMath::Fmod(Invulnerable,0.18f)>0.06f);

}
ARunCourse::ARunCourse() { PrimaryActorTick.bCanEverTick=true; RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("Root")); }
UInstancedStaticMeshComponent* ARunCourse::Instances(const TCHAR* Path,UMaterialInterface* Mat) {
 auto* C=NewObject<UInstancedStaticMeshComponent>(this); C->SetupAttachment(RootComponent);
 C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,Path)); if(Mat) C->SetMaterial(0,Mat);
 C->SetCollisionEnabled(ECollisionEnabled::NoCollision); C->RegisterComponent(); AddInstanceComponent(C); return C;
}
void ARunCourse::Shape(const TCHAR* Path,FVector Pos,FVector Scale,UMaterialInterface* Mat,FRotator Rot,bool Collision) {
 auto* C=NewObject<UStaticMeshComponent>(this); C->SetupAttachment(RootComponent);
 C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,Path)); if(Mat) C->SetMaterial(0,Mat);
 C->SetRelativeLocation(Pos); C->SetRelativeRotation(Rot); C->SetRelativeScale3D(Scale);
 C->SetCollisionEnabled(Collision?ECollisionEnabled::QueryAndPhysics:ECollisionEnabled::NoCollision);
 C->SetCollisionProfileName(Collision?TEXT("BlockAll"):TEXT("NoCollision")); C->RegisterComponent(); AddInstanceComponent(C);
}
void ARunCourse::BeginPlay() {
 Super::BeginPlay();
 auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
 auto Make=[&](FLinearColor Color,float Roughness){ auto* M=UMaterialInstanceDynamic::Create(Base,this); M->SetVectorParameterValue(TEXT("Color"),Color); M->SetScalarParameterValue(TEXT("Roughness"),Roughness); return M; };
 Snow=Make(FLinearColor(.8,.88,.95),.8); Dark=Make(FLinearColor(.025,.065,.085),.8); Gold=Make(FLinearColor(1,.5,.035),.24);
 auto* Road=Make(FLinearColor(.045,.075,.11),.28); auto* Ice=Make(FLinearColor(.15,.6,.8),.18);
 const TCHAR* Cube=TEXT("/Engine/BasicShapes/Cube.Cube"), *Sphere=TEXT("/Engine/BasicShapes/Sphere.Sphere"), *Cone=TEXT("/Engine/BasicShapes/Cone.Cone"), *Cylinder=TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
 auto* SnowSurface=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Runner/M_Snow.M_Snow"));
 // Each lane is built as physical segments with actual gaps, not a black overlay.
 for(int I=0;I<8;I++) {float X=9500+I*14000; Pits.Add({X,X+540,((I+2)%3-1)*220.f,false,false});}
 for(int I=0;I<8;I++) {float X=7700+I*14300; Pits.Add({X,X+720,0,false,true});}
 Pits.Sort([](const FPit& A,const FPit& B){return A.Start<B.Start;});
 for(int Side:{-1,1}) Shape(Cube,FVector(60000,Side*3950.f,-50),FVector(1240,71,1),SnowSurface?SnowSurface:Snow,FRotator::ZeroRotator,true);
 for(int Lane=-1;Lane<=1;Lane++) {
  float Start=-1000;
  auto Segment=[&](float End) { float Length=End-Start; if(Length>0) { Shape(Cube,FVector((Start+End)*.5,Lane*220.f,-50),FVector(Length/100,2.2,1),SnowSurface?SnowSurface:Snow,FRotator::ZeroRotator,true); Shape(Cube,FVector((Start+End)*.5,Lane*220.f,1),FVector(Length/100,2.2,.04),Road); }};
  for(const auto& Pit:Pits) if(Pit.Wide || FMath::IsNearlyEqual(Pit.Y,Lane*220.f)) {if(Pit.End<=Start) continue;Segment(Pit.Start);Start=FMath::Max(Start,Pit.End);}
  Segment(123000);
 }
 for(int Side:{-1,1}) Shape(Cube,FVector(60000,Side*365.f,0),FVector(1240,.7,.04),Road);
 for(const auto& Pit:Pits) {
  float Width=Pit.Wide?6.6f:2.2f;
  Shape(Cube,FVector((Pit.Start+Pit.End)*.5,Pit.Y,-260),FVector((Pit.End-Pit.Start)/100,Width,.1),Dark);
  for(float X:{Pit.Start,Pit.End}) {
   Shape(Cube,FVector(X,Pit.Y,-120),FVector(.12,Width,2.4),Ice);
   Shape(Cube,FVector(X,Pit.Y,5),FVector(.15,Width,.05),LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Runner/M_Amber.M_Amber")));
  }
 }
 auto* Lines=Instances(Cube,Ice);
 for(int X=0;X<124000;X+=600) for(int S: {-1,1}) Lines->AddInstance(FTransform(FRotator::ZeroRotator,FVector(X,S*110,4),FVector(2,.06,.025)));
 auto* Amber=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Runner/M_Amber.M_Amber"));
 auto* Posts=Instances(Cylinder,Dark); auto* Lamps=Instances(Sphere,Amber?Amber:Gold);
 for(int X=300;X<124000;X+=1600) for(int S: {-1,1}) { Posts->AddInstance(FTransform(FRotator::ZeroRotator,FVector(X,S*460,65),FVector(.12,.12,1.3))); Lamps->AddInstance(FTransform(FRotator::ZeroRotator,FVector(X,S*460,145),FVector(.35,.35,.35))); }
 auto* Trunks=Instances(Cylinder,Dark); auto* Foliage=Instances(Cone,Dark); auto* Caps=Instances(Cone,Snow); auto* Rocks=Instances(Sphere,Snow);
 auto* Pine=Instances(TEXT("/Game/Runner/Environment/Pine/pine_game/StaticMeshes/pine_game.pine_game"),nullptr);
 auto* Stone=Instances(TEXT("/Game/Runner/Environment/Rock/rock/StaticMeshes/rock.rock"),nullptr);
 FRandomStream Random(137);
 for(int I=0;I<650;I++) {
  float X=Random.FRandRange(-2000,125000), Side=I%2?1:-1, Y=Side*Random.FRandRange(750,5500), Size=Random.FRandRange(.7,2);
  if(Pine->GetStaticMesh()) {
 auto B=Pine->GetStaticMesh()->GetBounds(); float Scale=900*Size/(2*B.BoxExtent.Z);
 Pine->AddInstance(FTransform(FRotator(0,Random.FRandRange(0,360),0),FVector(X,Y,-(B.Origin.Z-B.BoxExtent.Z)*Scale),FVector(Scale))); continue;
 }
 Trunks->AddInstance(FTransform(FRotator::ZeroRotator,FVector(X,Y,170*Size),FVector(.45,.45,3.4)*Size));
  for(int Tier=0;Tier<3;Tier++) {
   float Radius=(4.1f-Tier)*Size, Z=(270+Tier*110)*Size;
   Foliage->AddInstance(FTransform(FRotator::ZeroRotator,FVector(X,Y,Z),FVector(Radius,Radius,3.4*Size)));
   Caps->AddInstance(FTransform(FRotator::ZeroRotator,FVector(X,Y,Z+45*Size),FVector(Radius*.83,Radius*.83,2.8*Size)));
  }
 }
 for(int I=0;I<75;I++) {
  float X=Random.FRandRange(-3000,128000), S=I%2?1:-1;
  FVector MountainScale(Random.FRandRange(30,90),Random.FRandRange(20,60),Random.FRandRange(35,110));
  float MountainRadius=50*FMath::Sqrt(FMath::Square(MountainScale.X)+FMath::Square(MountainScale.Y));
  Rocks->AddInstance(FTransform(FRotator(0,Random.FRandRange(0,360),0),FVector(X,S*(400+MountainRadius+Random.FRandRange(8000,13000)),-1200),MountainScale));
 }
 for(int I=0;I<160 && Stone->GetStaticMesh();I++) {
 auto B=Stone->GetStaticMesh()->GetBounds(); float Scale=Random.FRandRange(120,450)/B.BoxExtent.Z;
 float Radius=(FMath::Sqrt(FMath::Square(B.BoxExtent.X)+FMath::Square(B.BoxExtent.Y))+FVector2D(B.Origin.X,B.Origin.Y).Size())*Scale;
 Stone->AddInstance(FTransform(FRotator(0,Random.FRandRange(0,360),0),FVector(Random.FRandRange(-1000,123000),(I%2?1:-1)*(400+Radius+Random.FRandRange(150,900)),-(B.Origin.Z-B.BoxExtent.Z)*Scale-35),FVector(Scale)));
 }
 int32 UnsafeRocks=0;
 for(auto* Group : {Stone,Rocks}) if(Group->GetStaticMesh()) {
  FBox Local=Group->GetStaticMesh()->GetBoundingBox();
  for(int32 N=0;N<Group->GetInstanceCount();N++) {
   FTransform T; Group->GetInstanceTransform(N,T,true); FBox Bounds=Local.TransformBy(T);
   if(Bounds.Min.Y<400 && Bounds.Max.Y>-400) UnsafeRocks++;
  }
 }
 UE_LOG(LogTemp,Display,TEXT("RUNNER_ROAD_CLEARANCE: %d intruding rocks"),UnsafeRocks);
 ensureAlwaysMsgf(UnsafeRocks==0,TEXT("Scenery overlaps the runner road"));
 Coins=Instances(Cylinder,Gold);
 auto* SnowBody=Instances(Sphere,Snow); auto* Coal=Instances(Sphere,Dark);
 auto* Hat=Instances(Cylinder,Dark); auto* Orange=Instances(Cone,Make(FLinearColor(1,.18,.01),.65));
 auto* Scarf=Instances(Cylinder,Make(FLinearColor(.65,.018,.025),.75));
 auto* MudBody=Instances(Sphere,Make(FLinearColor(.14,.055,.02),.24));
 auto* Eyes=Instances(Sphere,Amber?Amber:Gold); auto* Branches=Instances(Cylinder,Dark);
 auto* TurboMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Runner/M_Turbo.M_Turbo"));
 BoostArrows=Instances(Cube,TurboMaterial?TurboMaterial:Ice);
 for(int I=0;I<64;I++) {
  float X=3000+I*1700; int Lane=I==0?0:Random.RandRange(-1,1);
  bool Conflict=false;for(const auto& Pit:Pits) if((Pit.Wide || FMath::Abs(Pit.Y-Lane*220.f)<1) && X>Pit.Start-1100 && X<Pit.End+1100) Conflict=true;
  if(!Conflict) {
   bool Mud=(I%4==2 || I%4==3); FEnemy Enemy; Enemy.X=X; Enemy.Y=Lane*220.f;Enemy.Mud=Mud;
   auto Part=[&](UInstancedStaticMeshComponent* Group,FVector Offset,FVector Scale,FRotator Rot=FRotator::ZeroRotator) {
    FTransform Pose(Rot,FVector(X,Lane*220.f,0)+Offset,Scale);
    FTransform Hidden=Pose; Hidden.AddToTranslation(FVector(0,0,-310));
    int N=Group->AddInstance(Hidden);Enemy.Parts.Add({Group,N,Pose,Group==Branches});
   };
   auto* Body=Mud?MudBody:SnowBody;
   Part(Body,FVector(0,0,55),FVector(1.25,1.25,1.1));
   Part(Body,FVector(0,0,128),FVector(1,1,1));
   Part(Body,FVector(0,0,197),FVector(.76,.76,.76));
   for(int Side:{-1,1}) {
    Part(Mud?Eyes:Coal,FVector(-34,Side*15.f,209),FVector(.1,.1,.1));
    Part(Branches,FVector(0,Side*85.f,140),FVector(.08,.08,.95),FRotator(0,0,Side*60.f));
    if(Mud) Part(Body,FVector(-4,Side*53.f,145),FVector(.45,.6,.5));
   }
   if(!Mud) {
    Part(Orange,FVector(-49,0,194),FVector(.2,.2,.45),FRotator(-90,0,0));
    Part(Hat,FVector(0,0,239),FVector(.95,.95,.08));
    Part(Hat,FVector(0,0,260),FVector(.6,.6,.4));
    Part(Scarf,FVector(0,0,165),FVector(.87,.87,.12));
    for(int J=0;J<3;J++) Part(Coal,FVector(-47,0,105+J*22.f),FVector(.09,.09,.09));
   } else {
    Part(Body,FVector(-8,14,225),FVector(.32,.44,.48));
    Part(Coal,FVector(-38,0,184),FVector(.08,.34,.12));
   }
   Enemies.Add(MoveTemp(Enemy)); Items.Add({X,Lane*220.f,Mud?EKind::Mudman:EKind::Snowman,false,-1});
  }
  int Safe=Lane==0?1:0;
  for(int J=0;J<5;J++) {
   float CX=X-450+J*200; bool OverPit=false;for(const auto& Pit:Pits) if((Pit.Wide || FMath::Abs(Pit.Y-Safe*220.f)<1) && CX>Pit.Start-250 && CX<Pit.End+250) OverPit=true;
   if(OverPit) continue;
   int N=Coins->AddInstance(FTransform(FRotator(90,0,0),FVector(CX,Safe*220.f,105),FVector(.7,.7,.13))); Items.Add({CX,Safe*220.f,EKind::Coin,false,N});
  }
 }
 int32 BoostCount=0;
 for(int I=0;I<28;I++) {
  float X=1800+I*4200; int Lane=I==0?0:I%3-1;
  // Keep acceleration arrows out of gaps and away from an enemy at the same location.
  for(int Attempt=0;Attempt<3;Attempt++) {
   bool Unsafe=false;
   for(const auto& Pit:Pits) if((Pit.Wide || FMath::IsNearlyEqual(Pit.Y,Lane*220.f)) && X>Pit.Start-600 && X<Pit.End+600) Unsafe=true;
   for(const auto& Item:Items) if((Item.Kind==EKind::Snowman || Item.Kind==EKind::Mudman) && FMath::IsNearlyEqual(Item.Y,Lane*220.f) && FMath::Abs(Item.X-X)<650) Unsafe=true;
   if(!Unsafe) break; Lane=(Lane+2)%3-1;
  }
  float Y=Lane*220.f; int N=BoostArrows->GetInstanceCount();
  // Three forward chevrons. The tips point along +X, the running direction.
  for(int Row=-1;Row<=1;Row++) for(int Side:{-1,1})
   BoostArrows->AddInstance(FTransform(FRotator(0,-Side*45.f,0),FVector(X+Row*140-25,Y+Side*35.f,7),FVector(1.1,.15,.035)));
  Items.Add({X,Y,EKind::Boost,false,N});BoostCount++;
 }
 HeartLobes=Instances(Sphere,Make(FLinearColor(1,.025,.12),.2));
 HeartTips=Instances(Cone,Make(FLinearColor(1,.025,.12),.2));
 int HeartCount=0;
 for(int I=0;I<12;I++) {
  float X=6200+I*9300;int Lane=(I+1)%3-1;
  bool Safe=false;
  for(int Try=0;Try<3;Try++) {
   Safe=true;
   for(const auto& Pit:Pits) if((Pit.Wide || FMath::Abs(Pit.Y-Lane*220.f)<1) && X>Pit.Start-700 && X<Pit.End+700) Safe=false;
   for(const auto& Enemy:Enemies) if(FMath::Abs(Enemy.Y-Lane*220.f)<1 && FMath::Abs(Enemy.X-X)<750) Safe=false;
   if(Safe) break;Lane=(Lane+2)%3-1;
  }
  if(!Safe) continue;
  int N=HeartLobes->GetInstanceCount();
  for(int Side:{-1,1}) HeartLobes->AddInstance(FTransform(FRotator::ZeroRotator,FVector(X,Lane*220.f+Side*20,140),FVector(.48,.48,.48)));
  HeartTips->AddInstance(FTransform(FRotator(180,0,0),FVector(X,Lane*220.f,115),FVector(.75,.75,.65)));
  Items.Add({X,Lane*220.f,EKind::Heart,false,N,130});HeartCount++;
 }
 UE_LOG(LogTemp,Display,TEXT("RUNNER_ENDLESS: loop 1200m, %d healing hearts, autosave every 5s"),HeartCount);
 CoinChime=LoadObject<USoundWave>(nullptr,TEXT("/Game/Runner/Audio/CoinChime.CoinChime"));
 TurboWhoosh=LoadObject<USoundWave>(nullptr,TEXT("/Game/Runner/Audio/TurboWhoosh.TurboWhoosh"));
 UE_LOG(LogTemp,Display,TEXT("RUNNER_EFFECTS: %d turbo arrows; coin sound %s; turbo sound %s"),BoostCount,CoinChime?TEXT("loaded"):TEXT("MISSING"),TurboWhoosh?TEXT("loaded"):TEXT("MISSING"));
 Music=NewObject<UAudioComponent>(this); Music->SetupAttachment(RootComponent);Music->bAutoActivate=false;
 Music->bIsUISound=true; Music->bAllowSpatialization=false;Music->SetVolumeMultiplier(.24);
 Music->SetSound(LoadObject<USoundWave>(nullptr,TEXT("/Game/Runner/Audio/NightFlight.NightFlight")));
 Music->RegisterComponent(); AddInstanceComponent(Music);
 UE_LOG(LogTemp,Display,TEXT("RUNNER_HAZARDS: %d enemies, %d physical pits"),Enemies.Num(),Pits.Num());
 for(const auto& Pit:Pits) if(Pit.Wide) for(int J=0;J<7;J++) {
 float X=Pit.Start-160+J*175, Z=110+FMath::Sin(J/6.f*PI)*190;
 int N=Coins->AddInstance(FTransform(FRotator(90,0,0),FVector(X,0,Z),FVector(.7,.7,.13)));Items.Add({X,0,EKind::Coin,false,N,Z});
 }
 SunLight=GetWorld()->SpawnActor<ADirectionalLight>(FVector(0,0,1000),FRotator(-28,-35,0));
 Cast<UDirectionalLightComponent>(SunLight->GetLightComponent())->SetAtmosphereSunLight(true); SunLight->GetLightComponent()->SetIntensity(4.5); SunLight->GetLightComponent()->SetLightColor(FLinearColor(.42,.65,1));
 auto* AtmosActor=GetWorld()->SpawnActor<AActor>();
 auto* Atmos=NewObject<USkyAtmosphereComponent>(AtmosActor); AtmosActor->SetRootComponent(Atmos); Atmos->RegisterComponent(); AtmosActor->AddInstanceComponent(Atmos);
 AmbientLight=GetWorld()->SpawnActor<ASkyLight>(); AmbientLight->GetLightComponent()->SetIntensity(.9); AmbientLight->GetLightComponent()->SetRealTimeCapture(true);
 CycleFog=GetWorld()->SpawnActor<AExponentialHeightFog>(); CycleFog->GetComponent()->SetFogDensity(.004); CycleFog->GetComponent()->SetFogHeightFalloff(.18); CycleFog->GetComponent()->SetVolumetricFog(true); CycleFog->GetComponent()->SetVolumetricFogDistance(18000);
 PostProcess=GetWorld()->SpawnActor<APostProcessVolume>(); auto* PP=PostProcess; PP->bUnbound=true;
 PP->Settings.bOverride_AutoExposureMethod=true; PP->Settings.AutoExposureMethod=AEM_Manual;
 PP->Settings.bOverride_AutoExposureApplyPhysicalCameraExposure=true; PP->Settings.AutoExposureApplyPhysicalCameraExposure=false;
 PP->Settings.bOverride_AutoExposureBias=true; PP->Settings.AutoExposureBias=0;
 PP->Settings.bOverride_BloomIntensity=true; PP->Settings.BloomIntensity=.45;
 PP->Settings.bOverride_MotionBlurAmount=true; PP->Settings.bOverride_MotionBlurMax=true;PP->Settings.MotionBlurMax=7;
 PP->Settings.bOverride_MotionBlurTargetFPS=true; PP->Settings.MotionBlurTargetFPS=60;
 MakeNightVillage();
 // Cache collectible poses once; recycling the course keeps memory bounded.
 for(auto Pair : {TPair<UInstancedStaticMeshComponent*,TArray<FTransform>*>(Coins,&CoinPoses),{BoostArrows,&ArrowPoses},{HeartLobes,&HeartLobePoses},{HeartTips,&HeartTipPoses}})
  for(int I=0;I<Pair.Key->GetInstanceCount();I++) {FTransform T;Pair.Key->GetInstanceTransform(I,T);Pair.Value->Add(T);}
 UpdateDaylight(0);

}
void ARunCourse::Tick(float Dt) {
 Super::Tick(Dt);
 auto* Runner=Cast<ARunCharacter>(UGameplayStatics::GetPlayerPawn(this,0)); if(!Runner) return;
 if(Music) {
  Music->SetVolumeMultiplier(Runner->MusicMuted?0.f:.24f);
  if(Runner->Started && !Music->IsPlaying() && !Runner->Finished && !Runner->Paused) Music->Play();
  Music->SetPaused(Runner->Paused || Runner->Finished);
 }
#if WITH_DEV_AUTOMATION_TESTS
 // Opt-in integration check runs the actual course and collectible logic in an isolated save slot.
 if(FParse::Param(FCommandLine::Get(),TEXT("RunnerVerify"))) {
  if(VerifyStage==0) {
   for(int I=0;I<Items.Num();I++) if(Items[I].Kind==EKind::Heart) { VerifyHeart=I;break; }
   check(VerifyHeart>=0);
   const auto& Heart=Items[VerifyHeart];FVector Position(Heart.X-300,Heart.Y,85);
   Runner->Started=true;Runner->Hearts=2;Runner->Invulnerable=1000;
   Runner->Lane=FMath::RoundToInt(Heart.Y/220);Runner->SetActorLocation(Position,false,nullptr,ETeleportType::TeleportPhysics);
   PreviousRunnerPosition=Position;VerifyStage=1;
  } else if(VerifyStage==1 && Runner->GetActorLocation().X>Items[VerifyHeart].X+120) {
   checkf(Runner->Hearts==3 && Items[VerifyHeart].Done,TEXT("Healing pickup integration failed"));
   UE_LOG(LogTemp,Display,TEXT("RUNNER_VERIFY: healing pickup PASS"));
   FVector Position(119980,0,85);Runner->Lane=0;Runner->SetActorLocation(Position,false,nullptr,ETeleportType::TeleportPhysics);PreviousRunnerPosition=Position;VerifyStage=2;
  } else if(VerifyStage==2 && Runner->LoopDistance>=ARunCharacter::LoopLength) {
   checkf(!Items[VerifyHeart].Done,TEXT("Heart was not restored for endless loop"));
   for(const auto& Pit:Pits) check(!Pit.Done);
   Runner->Coins=7;Runner->SaveProgress();Runner->SaveProgress();
   auto* Loaded=Cast<URunSave>(UGameplayStatics::LoadGameFromSlot(RunSaveSlot(),0));
   checkf(Loaded && Loaded->TotalCoins==7 && Loaded->BestCoins==7 && Loaded->BestMeters>=1200,TEXT("Saved progress integration failed"));
   UE_LOG(LogTemp,Display,TEXT("RUNNER_VERIFY: endless recycling and save/load without duplicate coins PASS"));
   VerifyStage=3;FGenericPlatformMisc::RequestExit(false);return;
  }
 }
#endif
 UpdateDaylight(Runner->Elapsed);
 if(!Runner->Started || Runner->Paused || Runner->Finished) return;
 if(Runner->GetActorLocation().X>=ARunCharacter::LoopLength) {
  FVector Position=Runner->GetActorLocation();Position.X-=ARunCharacter::LoopLength;
  Runner->LoopDistance+=ARunCharacter::LoopLength;
  Runner->SetActorLocation(Position,false,nullptr,ETeleportType::TeleportPhysics);
  ResetLoop();PreviousRunnerPosition=Position;
  UE_LOG(LogTemp,Display,TEXT("RUNNER_LOOP: recycled at %.0f metres"),Runner->LoopDistance/100);
 }
 FVector P=Runner->GetActorLocation(); FVector Previous=PreviousRunnerPosition; PreviousRunnerPosition=P;
 if(PostProcess) PostProcess->Settings.MotionBlurAmount=FMath::Lerp(.1f,.65f,FMath::Clamp(Runner->TurboExtra/1800.f,0.f,1.f));
 for(auto& Enemy:Enemies) {
  float Distance=Enemy.X-P.X;
  float RevealRange=3900.f;
  if(Distance>RevealRange+850 || Distance<-900) continue;
  float Alpha=FMath::Clamp((RevealRange-Distance)/850,0.,1.);float Rise=Alpha*Alpha*(3-2*Alpha);
  Enemy.LastRise=Rise;
  if(Enemy.BreakAge>=0) Enemy.BreakAge+=Dt;
  float Time=Runner->Elapsed;int N=0;
  for(auto& Part:Enemy.Parts) {
   FTransform T=Part.Pose;FVector Local=Part.Pose.GetLocation()-FVector(Enemy.X,Enemy.Y,0);
   FRotator Sway(0,FMath::Sin(Time*3+Enemy.X)*7,0);
   T.SetLocation(FVector(Enemy.X,Enemy.Y,-310*(1-Rise))+Sway.RotateVector(Local)+FVector(0,0,FMath::Abs(FMath::Sin(Time*4+Enemy.X))*10));
   if(Enemy.BreakAge>=0) {
    float Age=Enemy.BreakAge;T.AddToTranslation(FVector(FMath::Sin(N*2.4)*Age*340,FMath::Cos(N*2.4)*Age*340,Age*430-Age*Age*500));T.SetScale3D(T.GetScale3D()*FMath::Max(0.f,1-Age/.7f));
   }
   Part.Mesh->UpdateInstanceTransform(Part.Index,T,false,true);N++;
  }
 }
 auto FinishDamage=[&]() {Runner->HitFlash=.3; if(Runner->Hearts<=0) {Runner->Finished=true;Runner->Won=false;Runner->GetMesh()->SetVisibility(true);Runner->SaveProgress();}};
 for(auto& Pit:Pits) {
  if(Pit.Done || !RunRules::PitContact(Previous,P,Pit.Start,Pit.End,Pit.Y,Pit.Wide)) continue;
  Pit.Done=true; if(RunRules::Damage(Runner->Hearts,Runner->Invulnerable)) FinishDamage();
  // Rescue after the fall so a surviving player can continue rather than get stuck below the road.
  Runner->SetActorLocation(FVector(Pit.End+100,Runner->Lane*220.f,85));
  Runner->GetCharacterMovement()->Velocity=FVector(Runner->Speed,0,300);
  P=Runner->GetActorLocation(); PreviousRunnerPosition=P;Previous=P;break;
 }
 for(auto& Item:Items) {
  if(Runner->Finished) break;
  bool Coin=Item.Kind==EKind::Coin, Boost=Item.Kind==EKind::Boost, Heart=Item.Kind==EKind::Heart;
  if(Item.Done || !RunRules::Contact(Previous,P,Item.X,Item.Y,Coin||Boost||Heart?100.f:108.f,Coin||Heart,Boost?40.f:230.f,Item.Z)) continue;
  if(Coin) {
   Item.Done=true; Runner->Coins++; if(CoinChime) UGameplayStatics::PlaySound2D(this,CoinChime,.38f,.94f+(Runner->Coins%5)*.035f); Coins->UpdateInstanceTransform(Item.Index,FTransform(FVector(0,0,-10000)),false,true);
  } else if(Heart) {
   Item.Done=true; Runner->Hearts=RunRules::Heal(Runner->Hearts);
   if(CoinChime) UGameplayStatics::PlaySound2D(this,CoinChime,.5f,.7f);
   for(int N=0;N<2;N++) HeartLobes->UpdateInstanceTransform(Item.Index+N,FTransform(FVector(0,0,-10000)),false,true);
   HeartTips->UpdateInstanceTransform(Item.Index/2,FTransform(FVector(0,0,-10000)),false,true);
  } else if(Boost) {
   Item.Done=true;Runner->Boost=8;Runner->TurboExtra=RunRules::AddTurbo(Runner->TurboExtra);
   if(TurboWhoosh) UGameplayStatics::PlaySound2D(this,TurboWhoosh,.55f);
   for(int N=0;N<6;N++) BoostArrows->UpdateInstanceTransform(Item.Index+N,FTransform(FVector(0,0,-10000)),false,true);
  }
  else {
   if(!RunRules::Damage(Runner->Hearts,Runner->Invulnerable)) continue;
   Item.Done=true; FinishDamage();
   for(auto& Enemy:Enemies) if(FMath::Abs(Enemy.X-Item.X)<1 && FMath::Abs(Enemy.Y-Item.Y)<1) Enemy.BreakAge=0;
  }
 }
}
void ARunHUD::DrawHUD() {
 Super::DrawHUD(); if(!Canvas) return;
 auto* R=Cast<ARunCharacter>(GetOwningPawn()); if(!R) return;
 float W=Canvas->SizeX,H=Canvas->SizeY;
 // Vector icons remain crisp at every window size, with a compact right-hand header.
 auto Triangle=[&](FVector2D A,FVector2D B,FVector2D C,FLinearColor Color) {
  FCanvasTriangleItem Item(A,B,C,GWhiteTexture);Item.SetColor(Color);Item.BlendMode=SE_BLEND_Translucent;Canvas->DrawItem(Item);
 };
 auto Disc=[&](float X,float Y,float Radius,FLinearColor Color) {
  for(int I=0;I<32;I++) {float A=I*2*PI/32,B=(I+1)*2*PI/32;Triangle({X,Y},{X+Radius*FMath::Cos(A),Y+Radius*FMath::Sin(A)},{X+Radius*FMath::Cos(B),Y+Radius*FMath::Sin(B)},Color);}
 };
 auto Heart=[&](float X,float Y,float Size,FLinearColor Color) {
  auto Point=[&](float T) {return FVector2D(X+Size*FMath::Pow(FMath::Sin(T),3),Y-Size*(13*FMath::Cos(T)-5*FMath::Cos(2*T)-2*FMath::Cos(3*T)-FMath::Cos(4*T))/16);};
  for(int I=0;I<48;I++) Triangle({X,Y},Point(I*2*PI/48),Point((I+1)*2*PI/48),Color);
 };
 auto Card=[&](float X,float Width,FLinearColor Accent) {
  DrawRect(FLinearColor(.005,.016,.032,.85),X+10,24,Width-20,82);
  DrawRect(FLinearColor(.005,.016,.032,.85),X,34,Width,62);
  for(float DX:{10.f,Width-10}) for(float Y:{34.f,96.f}) Disc(X+DX,Y,10,FLinearColor(.005,.016,.032,.85));
  DrawRect(Accent,X+16,103,Width-32,2);
 };
 float Header=W-452;
 Card(Header,144,FLinearColor(.2,.85,1));Card(Header+154,124,FLinearColor(1,.75,.2));Card(Header+288,142,FLinearColor(1,.2,.4));
 float CX=Header+29,CY=59;
 for(int I=0;I<20;I++) {float A=PI+I*PI/20,B=PI+(I+1)*PI/20;DrawLine(CX+19*FMath::Cos(A),CY+19*FMath::Sin(A),CX+19*FMath::Cos(B),CY+19*FMath::Sin(B),FLinearColor(.2,.85,1),3);}
 float Needle=PI+PI*R->Speed/(R->Speed+1600.f);
 DrawLine(CX,CY,CX+15*FMath::Cos(Needle),CY+15*FMath::Sin(Needle),FLinearColor::White,2);Disc(CX,CY,3,FLinearColor::White);
 DrawText(FString::Printf(TEXT("%.0f"),R->Speed*.036f),FLinearColor::White,Header+58,36,nullptr,2.15);
 DrawText(TEXT("km/h"),FLinearColor(.45,.7,.85),Header+59,76,nullptr,.95);
 Disc(Header+181,59,17,FLinearColor(.55,.27,.025));Disc(Header+181,57,15,FLinearColor(1,.72,.12));Disc(Header+181,57,11,FLinearColor(.9,.48,.035));
 DrawLine(Header+181,48,Header+181,66,FLinearColor(1,.86,.4),3);
 DrawText(FString::FromInt(R->Coins),FLinearColor(1,.85,.4),Header+207,39,nullptr,2.1);
 DrawText(TEXT("coins"),FLinearColor(.7,.61,.4),Header+207,76,nullptr,.95);
 for(int I=0;I<3;I++) {Heart(Header+311+I*39,57,16,FLinearColor(.2,.1,.17));if(I<R->Hearts) Heart(Header+311+I*39,55,15,FLinearColor(1,.14,.32));}
 DrawText(TEXT("health"),FLinearColor(.8,.45,.55),Header+334,79,nullptr,.9);
 DrawText(TEXT("SEASON RUN"),FLinearColor(.5,.85,1),28,27,nullptr,1.3);
 DrawText(FString::Printf(TEXT("%s  /  ENDLESS"),RunRules::Daylight(R->Elapsed)>.5?TEXT("DAY"):TEXT("NIGHT")),FLinearColor(.65,.75,.82),28,55,nullptr,.95);
 DrawText(FString::Printf(TEXT("%d m"),FMath::Max(0,int(R->Distance/100))),FLinearColor::White,28,80,nullptr,2.1);
 if(R->Progress) {
  DrawText(FString::Printf(TEXT("BEST  %d m   /   %d coins"),FMath::Max(R->Progress->BestMeters,int(R->Distance/100)),FMath::Max(R->Progress->BestCoins,R->Coins)),FLinearColor(.85,.73,.4),28,120,nullptr,1);
  DrawText(FString::Printf(TEXT("BANK  %d coins"),R->Progress->TotalCoins+R->Coins-R->SavedRunCoins),FLinearColor(.6,.7,.8),28,143,nullptr,.95);
 }
 DrawText(TEXT("A/D: lanes   SPACE: double jump   P: pause   R: retry   M: music"),FLinearColor::White,30,H-32,nullptr,1);
 if(R->TurboExtra>50 && R->Started && !R->Paused && !R->Finished) {
  // Peripheral streaks leave the road and obstacles unobscured.
  float T=GetWorld()->GetTimeSeconds();
  for(int N=0;N<40;N++) {
   float Angle=N*2.39996f, R0=.48f+.8f*FMath::Fmod(T*1.9f+N*.047f,1.f);
   FVector2D D(FMath::Cos(Angle)*W*.62,FMath::Sin(Angle)*H*.72),Center(W*.5,H*.48);
   FVector2D A=Center+D*R0,B=Center+D*(R0+.16);
   DrawLine(A.X,A.Y,B.X,B.Y,FLinearColor(.2,.8,1,.1f+.35f*FMath::Clamp(R->TurboExtra/1800.f,0.f,1.f)),1.4);
  }
  DrawText(FString::Printf(TEXT("TURBO  +%.0f km/h"),R->TurboExtra*.036f),FLinearColor(.2,1,1),W*.38,H-65,nullptr,1.5);
 }
 if(R->Started && !R->Paused && !R->Finished) {
  // A full-width chasm cannot be avoided by switching lanes.
  for(int I=0;I<8;I++) { float X=7700+I*14300, Gap=X-R->GetActorLocation().X;
   if(Gap>0 && Gap<FMath::Max(1100.f,R->Speed*.85f)) {DrawText(TEXT("JUMP!"),FLinearColor(1,.7,.2),W*.45,H*.22,nullptr,2.1);break;}
  }
 }
 if(R->HitFlash>0) {DrawRect(FLinearColor(1,.05,.015,R->HitFlash*.5),0,0,W,H); DrawText(TEXT("HIT  -1 LIFE"),FLinearColor(1,.3,.1),W*.42,H*.3,nullptr,1.8);}
 if(!R->Started || R->Paused || R->Finished) {
  DrawRect(FLinearColor(.01,.025,.05,.65),W*.2,H*.32,W*.6,190);
  FString Title=!R->Started?TEXT("ENDLESS RUN"):R->Paused?TEXT("PAUSED"):R->Won?TEXT("PASS COMPLETE"):TEXT("RUN ENDED");
  FString Sub=!R->Started?TEXT("ENTER to run  /  hearts heal  /  day & night"):R->Paused?TEXT("P to resume"):FString::Printf(TEXT("%d m  /  %d coins  /  record saved  /  R to retry"),int(R->Distance/100),R->Coins);
  DrawText(Title,FLinearColor::White,W*.28,H*.37,nullptr,2.4); DrawText(Sub,FLinearColor(.5,.85,1),W*.28,H*.47,nullptr,1.2);
 }
}
ARunGameMode::ARunGameMode() {DefaultPawnClass=ARunCharacter::StaticClass(); HUDClass=ARunHUD::StaticClass();}
void ARunGameMode::BeginPlay() {
 Super::BeginPlay(); GetWorld()->SpawnActor<ARunCourse>();
 if(auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0)) Pawn->SetActorLocation(FVector(0,0,80));
 if(auto* PC=UGameplayStatics::GetPlayerController(this,0)) {PC->SetInputMode(FInputModeGameOnly());PC->bShowMouseCursor=false;}
}

void ARunCourse::MakeNightVillage() {
 const TCHAR* Cube=TEXT("/Engine/BasicShapes/Cube.Cube"),*Sphere=TEXT("/Engine/BasicShapes/Sphere.Sphere"),*Cone=TEXT("/Engine/BasicShapes/Cone.Cone");
 auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
 auto Make=[&](FLinearColor C){auto* M=UMaterialInstanceDynamic::Create(Base,this);M->SetVectorParameterValue(TEXT("Color"),C);M->SetScalarParameterValue(TEXT("Roughness"),.85);return M;};
 auto* SkyMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Runner/M_NightSky.M_NightSky"));
 auto* StarsMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Runner/M_Stars.M_Stars"));
 auto* SkyMesh=NewObject<UStaticMeshComponent>(this);SkyMesh->SetupAttachment(RootComponent);
 CycleSky=UMaterialInstanceDynamic::Create(SkyMaterial,this);
 SkyMesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,Sphere));SkyMesh->SetMaterial(0,CycleSky);
 SkyMesh->SetRelativeLocation(FVector(60000,0,0));SkyMesh->SetRelativeScale3D(FVector(10000));SkyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);SkyMesh->SetCastShadow(false);SkyMesh->RegisterComponent();AddInstanceComponent(SkyMesh);
 CycleStars=UMaterialInstanceDynamic::Create(StarsMaterial,this);
 auto* Stars=Instances(Sphere,CycleStars); auto* Links=Instances(Cube,CycleStars);
 NightStars=Stars;NightLinks=Links;
 Stars->SetCastShadow(false); Links->SetCastShadow(false);
 FRandomStream R(391);
 for(int I=0;I<430;I++) {
  FVector Pos(R.FRandRange(-50000,240000),R.FRandRange(-170000,170000),R.FRandRange(40000,180000));
  float Scale=R.FRandRange(2.2,5);Stars->AddInstance(FTransform(FRotator::ZeroRotator,Pos,FVector(Scale)));
 }
 // Repeated distant seven-star constellations remain visible along the route.
 for(int C=0;C<5;C++) {
  FVector Origin(60000+C*38000,(C%2?1:-1)*21000,6500+C%2*1500);
  TArray<FVector> Points={Origin+FVector(0,0,0),Origin+FVector(900,3500,1100),Origin+FVector(1600,7000,1500),Origin+FVector(2400,10000,2300),Origin+FVector(2000,14500,2400),Origin+FVector(1400,14000,5500),Origin+FVector(2200,10000,5700)};
  for(auto P:Points) Stars->AddInstance(FTransform(FRotator::ZeroRotator,P,FVector(5)));
  for(int I=0;I<6;I++) {FVector D=Points[I+1]-Points[I];Links->AddInstance(FTransform(FRotationMatrix::MakeFromX(D).Rotator(),(Points[I]+Points[I+1])*.5,FVector(D.Size()/100,.13,.13)));}
  FVector D=Points[3]-Points[6];Links->AddInstance(FTransform(FRotationMatrix::MakeFromX(D).Rotator(),(Points[3]+Points[6])*.5,FVector(D.Size()/100,.13,.13)));
 }
 auto* Wood=Make(FLinearColor(.12,.055,.025)); auto* Roof=Make(FLinearColor(.015,.04,.065));
 auto* Warm=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Runner/M_Amber.M_Amber"));
 auto* Houses=Instances(Cube,Wood),*Roofs=Instances(Cube,Roof),*RoofSnow=Instances(Cube,Snow),*Windows=Instances(Cube,Warm),*Doors=Instances(Cube,Dark);
 auto* MeadowMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Runner/M_Meadow.M_Meadow"));
 auto* Meadows=Instances(Sphere,MeadowMaterial);auto* Grass=Instances(Cone,MeadowMaterial);
 for(int I=0;I<24;I++) {
  float X=5500+I*4600,Side=I%2?1:-1,Y=Side*5400;
  Meadows->AddInstance(FTransform(FRotator::ZeroRotator,FVector(X,Y,2),FVector(55,27,.04)));
  for(int House=0;House<2;House++) {
   FVector Origin(X+House*1050,Y+House*Side*800,0);
   Houses->AddInstance(FTransform(FRotator::ZeroRotator,Origin+FVector(0,0,220),FVector(8,6,4.4)));
   for(int S:{-1,1}) {
    FRotator Rot(0,0,-S*35.f); FVector Pos=Origin+FVector(0,S*170.f,520);
    Roofs->AddInstance(FTransform(Rot,Pos,FVector(9,4,.3)));RoofSnow->AddInstance(FTransform(Rot,Pos+FVector(0,0,12),FVector(9,4,.04)));
   }
   for(int S:{-1,1}) Windows->AddInstance(FTransform(FRotator::ZeroRotator,Origin+FVector(S*230.f,-Side*303.f,270),FVector(1.3,.05,1.5)));
   Doors->AddInstance(FTransform(FRotator::ZeroRotator,Origin+FVector(0,-Side*305.f,130),FVector(.9,.06,2.6)));
   Doors->AddInstance(FTransform(FRotator::ZeroRotator,Origin+FVector(240,90,610),FVector(.7,.7,2.3)));
  }
  for(int J=0;J<70;J++) Grass->AddInstance(FTransform(FRotator::ZeroRotator,FVector(X+R.FRandRange(-2200,2200),Y+R.FRandRange(-1000,1000),23),FVector(.045,.045,R.FRandRange(.2,.5))));
 }
 UE_LOG(LogTemp,Display,TEXT("RUNNER_WORLD: 48 cabins, 24 meadows, 5 constellations"));
}

void ARunCourse::ResetLoop() {
 for(auto& Item:Items) Item.Done=false;
 for(auto& Pit:Pits) Pit.Done=false;
 for(auto& Enemy:Enemies) { Enemy.BreakAge=-1; Enemy.LastRise=-1;
  for(auto& Part:Enemy.Parts) { FTransform T=Part.Pose;T.AddToTranslation(FVector(0,0,-310));Part.Mesh->UpdateInstanceTransform(Part.Index,T,false,true); }
 }
 for(auto Pair : {TPair<UInstancedStaticMeshComponent*,TArray<FTransform>*>(Coins,&CoinPoses),{BoostArrows,&ArrowPoses},{HeartLobes,&HeartLobePoses},{HeartTips,&HeartTipPoses}})
  for(int I=0;I<Pair.Value->Num();I++) Pair.Key->UpdateInstanceTransform(I,(*Pair.Value)[I],false,I==Pair.Value->Num()-1);
}
void ARunCourse::UpdateDaylight(float Time) {
 float Day=RunRules::Daylight(Time);
 if(NightStars) NightStars->SetVisibility(Day<.65f);
 if(NightLinks) NightLinks->SetVisibility(Day<.65f);
 if(CycleSky) CycleSky->SetVectorParameterValue(TEXT("SkyColor"),FMath::Lerp(FLinearColor(.005,.009,.028),FLinearColor(.22,.5,.95),Day));
 if(CycleStars) CycleStars->SetVectorParameterValue(TEXT("StarColor"),FLinearColor(.7,1.5,3)*FMath::Pow(1-Day,3.f));
 if(SunLight) { SunLight->SetActorRotation(FRotator(FMath::Lerp(-22.f,-52.f,Day),-35+Day*35,0));
  SunLight->GetLightComponent()->SetIntensity(FMath::Lerp(4.5f,9.f,Day));
  SunLight->GetLightComponent()->SetLightColor(FMath::Lerp(FLinearColor(.42,.65,1),FLinearColor(1,.93,.78),Day)); }
 if(AmbientLight) AmbientLight->GetLightComponent()->SetIntensity(FMath::Lerp(.9f,1.5f,Day));
 if(CycleFog) {CycleFog->GetComponent()->SetFogDensity(FMath::Lerp(.004f,.0012f,Day));CycleFog->GetComponent()->SetFogInscatteringColor(FMath::Lerp(FLinearColor(.035,.06,.13),FLinearColor(.5,.7,1),Day));}
}
