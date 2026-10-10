import unreal, json
from pathlib import Path
assets=unreal.AssetToolsHelpers.get_asset_tools()
report={}
for name,file in [('Pine',str(Path(__file__).resolve().parents[2] / 'godot-source/assets/pine_game.glb')),('Rock',str(Path(__file__).resolve().parents[2] / 'godot-source/assets/rock/rock.gltf')),('Snow',str(Path(__file__).resolve().parents[2] / 'godot-source/assets/snow_diff.jpg'))]:
 task=unreal.AssetImportTask()
 task.filename=file
 task.destination_path='/Game/Runner/Environment/'+name
 task.automated=True
 task.replace_existing=True
 task.save=True
 assets.import_asset_tasks([task])
 report[name]=list(task.imported_object_paths)
 unreal.log('RUNNER_IMPORT '+name+': '+str(report[name]))
unreal.EditorAssetLibrary.save_directory('/Game/Runner',only_if_is_dirty=False,recursive=True)
Path(str(Path(__file__).resolve().parents[1] / 'art-import.json')).write_text(json.dumps(report,indent=2))
