"""Restore the locally licensed robot assets; never upload them to this repository."""
from pathlib import Path
import argparse,json,shutil
parser=argparse.ArgumentParser(description="Copy required assets from your licensed Stack O Bot project")
parser.add_argument("project",type=Path,help="Stack O Bot project directory or .uproject file")
args=parser.parse_args()
source=args.project.parent if args.project.is_file() else args.project
root=Path(__file__).resolve().parents[1]
assets=json.loads((root/"Tools/stackobot-assets.json").read_text())
missing=[name for name in assets if not (source/"Content"/name).is_file()]
if missing:parser.error("Required assets missing: "+", ".join(missing))
for name in assets:
 target=root/"Content"/name
 target.parent.mkdir(parents=True,exist_ok=True)
 shutil.copy2(source/"Content"/name,target)
print(f"Copied {len(assets)} licensed assets locally. Content/StackOBot is ignored by Git.")
