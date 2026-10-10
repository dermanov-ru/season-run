#!/bin/zsh
set -e
project_dir="${0:A:h}"
engine_root="${SEASON_RUN_UE_ROOT:-/Users/Shared/Epic Games/UE_5.8}"
editor="$engine_root/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor"
if [[ ! -x "$editor" ]]; then
 print "Unreal Engine 5.8 is required. Set SEASON_RUN_UE_ROOT to its installation directory."
 exit 1
fi
if [[ ! -f "$project_dir/Content/StackOBot/Characters/Bot/Mesh/SKM_Bot.uasset" ]]; then
 print "Restore the locally licensed Stack O Bot character first; see README.md."
 exit 1
fi
"$engine_root/Engine/Build/BatchFiles/Mac/Build.sh" SeasonRunEditor Mac Development "$project_dir/SeasonRun.uproject" -MaxParallelActions=2
exec "$editor" "$project_dir/SeasonRun.uproject" /Engine/Maps/Entry -game -windowed -ResX=1280 -ResY=720 -ExecCmds="t.MaxFPS 60"
