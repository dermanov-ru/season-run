import unreal,json
from pathlib import Path
at=unreal.AssetToolsHelpers.get_asset_tools();report={}
for name in ['CoinChime','TurboWhoosh']:
 task=unreal.AssetImportTask();task.filename=str(Path(__file__).resolve().parents[1] / 'Audio' / (name+'.wav'));task.destination_path='/Game/Runner/Audio';task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True
 at.import_asset_tasks([task]);sound=unreal.load_asset('/Game/Runner/Audio/'+name);assert sound
 sound.set_editor_property('looping',False);unreal.EditorAssetLibrary.save_loaded_asset(sound)
 report[name]=sound.get_editor_property('duration')
lib=unreal.MaterialEditingLibrary
mat=unreal.load_asset('/Game/Runner/M_Turbo') or at.create_asset('M_Turbo','/Game/Runner',unreal.Material,unreal.MaterialFactoryNew())
lib.delete_all_material_expressions(mat)
color=lib.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector,-100,0);color.set_editor_property('constant',unreal.LinearColor(.02,4,6,1))
assert lib.connect_material_property(color,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
assert lib.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
lib.set_material_usage(mat,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
Path(str(Path(__file__).resolve().parents[1] / 'effects-import.json')).write_text(json.dumps(report,indent=2))
