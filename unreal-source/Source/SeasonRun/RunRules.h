#pragma once
#include "CoreMinimal.h"
namespace RunRules {
inline bool Contact(FVector Previous,FVector Current,float X,float Y,float HalfWidth,bool Coin,float Height=145.f,float CoinHeight=105.f) {
 if(X<FMath::Min(Previous.X,Current.X)-80 || X>FMath::Max(Previous.X,Current.X)+80) return false;
 float Alpha=FMath::IsNearlyEqual(Previous.X,Current.X)?1.f:FMath::Clamp((X-Previous.X)/(Current.X-Previous.X),0.f,1.f);
 FVector At=FMath::Lerp(Previous,Current,Alpha);
 return FMath::Abs(At.Y-Y)<=HalfWidth && (Coin?FMath::Abs(At.Z-CoinHeight)<=115:At.Z-65<=Height);
}
inline float AddTurbo(float Extra) { return FMath::Max(0.f,Extra)+200.f; }
inline float Speed(float Elapsed,float Extra) { return 1000.f+FMath::Max(0.f,Extra); }
inline bool PitContact(FVector Previous,FVector Current,float Start,float End,float Y,bool Wide=false) {
 float From=FMath::Max(Previous.X,Start+35), To=FMath::Min(Current.X,End-35);
 if(From>To) return false;
 float X=(From+To)*.5f;
 float Alpha=FMath::IsNearlyEqual(Previous.X,Current.X)?1.f:FMath::Clamp((X-Previous.X)/(Current.X-Previous.X),0.f,1.f);
 FVector At=FMath::Lerp(Previous,Current,Alpha);
 return FMath::Abs(At.Y-Y)<(Wide?350:82) && At.Z-65<35;
}
inline int32 Heal(int32 Hearts) { return FMath::Min(3,Hearts+1); }
inline float Daylight(float Time) { float A=.5f+.5f*FMath::Cos(Time*2.f*PI/100.f); return A*A*(3-2*A); }
inline bool Damage(int32& Hearts,float& Protection) {
 if(Protection>0 || Hearts<=0) return false;
 Hearts=FMath::Max(0,Hearts-1); Protection=1.4f; return true;
}
}
