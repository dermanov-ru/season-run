#include "RunRules.h"
#include "RunGame.h"
#include "Kismet/GameplayStatics.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunContactTest,"SeasonRun.Collision.SweptContact",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FRunContactTest::RunTest(const FString&) {
 TestTrue(TEXT("Slow frame still hits obstacle"),RunRules::Contact(FVector(0,0,65),FVector(600,0,65),300,0,108,false));
 TestFalse(TEXT("Other lane is safe"),RunRules::Contact(FVector(0,220,65),FVector(600,220,65),300,0,108,false));
 TestFalse(TEXT("Jump clears barrier"),RunRules::Contact(FVector(0,0,300),FVector(600,0,300),300,0,108,false));
 TestTrue(TEXT("Lane change sampled at crossing"),RunRules::Contact(FVector(0,0,65),FVector(600,220,65),150,0,108,false));
 TestFalse(TEXT("Distant item cannot hit"),RunRules::Contact(FVector(0,0,65),FVector(600,0,65),2000,0,108,false));
 TestTrue(TEXT("Coin at runner height collected"),RunRules::Contact(FVector(0,0,65),FVector(600,0,65),300,0,100,true));
 TestFalse(TEXT("High jump misses ground coin"),RunRules::Contact(FVector(0,0,350),FVector(600,0,350),300,0,100,true));
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunLivesTest,"SeasonRun.Collision.LivesAndProtection",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FRunLivesTest::RunTest(const FString&) {
 int32 Lives=3; float Protection=0;
 TestTrue(TEXT("First collision damages"),RunRules::Damage(Lives,Protection)); TestEqual(TEXT("One life lost"),Lives,2);
 TestFalse(TEXT("Invulnerability prevents repeated damage"),RunRules::Damage(Lives,Protection)); TestEqual(TEXT("Lives unchanged"),Lives,2);
 Protection=0; RunRules::Damage(Lives,Protection); Protection=0; RunRules::Damage(Lives,Protection);
 TestEqual(TEXT("Third hit reaches zero"),Lives,0); Protection=0; TestFalse(TEXT("No damage after death"),RunRules::Damage(Lives,Protection));
 TestEqual(TEXT("Never negative lives"),Lives,0); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunPitTest,"SeasonRun.Hazards.PitsAndSpeed",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FRunPitTest::RunTest(const FString&) {
 TestTrue(TEXT("Running into pit loses footing"),RunRules::PitContact(FVector(0,0,65),FVector(600,0,65),100,540,0));
 TestFalse(TEXT("Jump clears pit"),RunRules::PitContact(FVector(0,0,210),FVector(600,0,210),100,540,0));
 TestFalse(TEXT("Adjacent lane avoids pit"),RunRules::PitContact(FVector(0,220,65),FVector(600,220,65),100,540,0));
 TestFalse(TEXT("Before pit safe"),RunRules::PitContact(FVector(0,0,65),FVector(50,0,65),100,540,0));
 TestTrue(TEXT("Long frame cannot skip pit"),RunRules::PitContact(FVector(0,0,65),FVector(1800,0,65),100,540,0));
 TestEqual(TEXT("Starting speed"),RunRules::Speed(0,0),1000.f);
 TestEqual(TEXT("Speed only increases from pickups"),RunRules::Speed(50,0),RunRules::Speed(0,0));
 TestEqual(TEXT("Base remains steady"),RunRules::Speed(1000,0),1000.f);
 TestTrue(TEXT("Boost increases speed"),RunRules::Speed(50,3)>RunRules::Speed(50,0));
 TestEqual(TEXT("Negative extra speed is ignored"),RunRules::Speed(50,-1),RunRules::Speed(50,0));
 TestTrue(TEXT("Snowman taller than crate"),RunRules::Contact(FVector(0,0,230),FVector(600,0,230),300,0,108,false,230));
 TestFalse(TEXT("Double jump clears snowman"),RunRules::Contact(FVector(0,0,350),FVector(600,0,350),300,0,108,false,230));
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunTurboTest,"SeasonRun.Hazards.TurboPickups",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FRunTurboTest::RunTest(const FString&) {
 TestTrue(TEXT("Running over arrow collects it"),RunRules::Contact(FVector(0,0,65),FVector(400,0,65),200,0,100,false,40));
 TestFalse(TEXT("Jumping over arrow does not collect it"),RunRules::Contact(FVector(0,0,210),FVector(400,0,210),200,0,100,false,40));
 TestFalse(TEXT("Arrow in another lane not collected"),RunRules::Contact(FVector(0,220,65),FVector(400,220,65),200,0,100,false,40));
 TestTrue(TEXT("Fast frame still collects arrow"),RunRules::Contact(FVector(0,0,65),FVector(3400,0,65),200,0,100,false,40));
 TestEqual(TEXT("One pickup adds a small increment"),RunRules::Speed(0,RunRules::AddTurbo(0)),1200.f);
 TestEqual(TEXT("Old maximum no longer limits speed"),RunRules::Speed(1000,3600),4600.f);
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunAttackTest,"SeasonRun.Hazards.WidePitsAndStacking",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FRunAttackTest::RunTest(const FString&) {
 TestTrue(TEXT("Wide pit affects outside lane"),RunRules::PitContact(FVector(0,220,65),FVector(1000,220,65),100,820,0,true));
 TestFalse(TEXT("Wide pit can be jumped"),RunRules::PitContact(FVector(0,220,250),FVector(1000,220,250),100,820,0,true));
 float Extra=RunRules::AddTurbo(0);TestEqual(TEXT("First turbo adds 200"),Extra,200.f);
 Extra=RunRules::AddTurbo(Extra);TestEqual(TEXT("Second pickup adds more"),Extra,400.f);
 Extra=RunRules::AddTurbo(RunRules::AddTurbo(Extra));TestEqual(TEXT("Stacking remains additive"),Extra,800.f);
 for(int I=0;I<1000;I++) Extra=RunRules::AddTurbo(Extra);
 TestEqual(TEXT("Thousand pickups keep adding without a cap"),Extra,200800.f);
 TestEqual(TEXT("Accumulated speed does not expire with time"),RunRules::Speed(0,Extra),RunRules::Speed(100000,Extra));
 TestTrue(TEXT("Air coin at arc height collected"),RunRules::Contact(FVector(0,0,250),FVector(600,0,250),300,0,100,true,145,280));
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunProgressTest,"SeasonRun.Endless.HealingDaylightAndSave",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FRunProgressTest::RunTest(const FString&) {
 TestEqual(TEXT("Heart heals one life"),RunRules::Heal(1),2);
 TestEqual(TEXT("Healing cannot exceed three"),RunRules::Heal(3),3);
 TestTrue(TEXT("Sunlit day initially"),RunRules::Daylight(0)>.99f);
 TestTrue(TEXT("Night after fifty seconds"),RunRules::Daylight(50)<.01f);
 TestTrue(TEXT("Day returns"),RunRules::Daylight(100)>.99f);
 URunSave* Original=NewObject<URunSave>();Original->BestMeters=4200;Original->BestCoins=123;Original->TotalCoins=456;
 TArray<uint8> Data;TestTrue(TEXT("Progress serializes"),UGameplayStatics::SaveGameToMemory(Original,Data));
 auto* Loaded=Cast<URunSave>(UGameplayStatics::LoadGameFromMemory(Data));
 TestNotNull(TEXT("Saved progress restores"),Loaded);
 if(Loaded) {TestEqual(TEXT("Metres survive"),Loaded->BestMeters,4200);TestEqual(TEXT("Coin record survives"),Loaded->BestCoins,123);TestEqual(TEXT("Coin bank survives"),Loaded->TotalCoins,456);}
 return true;
}
#endif
