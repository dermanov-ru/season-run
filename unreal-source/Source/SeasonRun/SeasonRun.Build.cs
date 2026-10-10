using UnrealBuildTool;
public class SeasonRun : ModuleRules {
 public SeasonRun(ReadOnlyTargetRules Target) : base(Target) {
 PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
 PublicDependencyModuleNames.AddRange(new string[]{"Core","CoreUObject","Engine","InputCore","RenderCore"});
 }
}
