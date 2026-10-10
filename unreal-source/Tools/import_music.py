import unreal
from pathlib import Path
import json
at=unreal.AssetToolsHelpers.get_asset_tools()
task=unreal.AssetImportTask();task.filename=str(Path(__file__).resolve().parents[1] / 'Audio/WinterPulse.wav');task.destination_path='/Game/Runner/Audio';task.destination_name='WinterPulse';task.automated=True;task.save=True;task.replace_existing=True
at.import_asset_tasks([task])
sound=unreal.load_asset('/Game/Runner/Audio/WinterPulse');assert sound
sound.set_editor_property('looping',True)
unreal.EditorAssetLibrary.save_loaded_asset(sound)
Path(str(Path(__file__).resolve().parents[1] / 'music-import.json')).write_text(json.dumps({'asset':sound.get_path_name(),'looping':sound.get_editor_property('looping'),'duration':sound.get_editor_property('duration')},indent=2))
