using UnrealBuildTool;
public class SeasonRunTarget : TargetRules {
 public SeasonRunTarget(TargetInfo Target) : base(Target) {
 Type = TargetType.Game; DefaultBuildSettings = BuildSettingsVersion.Latest;
 IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
 ExtraModuleNames.Add("SeasonRun");
 }
}
