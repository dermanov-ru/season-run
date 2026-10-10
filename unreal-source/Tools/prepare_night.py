import unreal,json
from pathlib import Path
at=unreal.AssetToolsHelpers.get_asset_tools();lib=unreal.MaterialEditingLibrary
for name,color,unlit,two in [('M_NightSky',(.005,.009,.028),True,True),('M_Stars',(.7,1.5,3),True,False),('M_Meadow',(.065,.15,.035),False,False)]:
 mat=unreal.load_asset('/Game/Runner/'+name) or at.create_asset(name,'/Game/Runner',unreal.Material,unreal.MaterialFactoryNew())
 lib.delete_all_material_expressions(mat);mat.set_editor_property('two_sided',two)
 if unlit:mat.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
 c=lib.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector,-100,0);c.set_editor_property('constant',unreal.LinearColor(*color,1))
 assert lib.connect_material_property(c,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR if unlit else unreal.MaterialProperty.MP_BASE_COLOR)
 lib.set_material_usage(mat,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES);lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
task=unreal.AssetImportTask();task.filename=str(Path(__file__).resolve().parents[1] / 'Audio/NightFlight.wav');task.destination_path='/Game/Runner/Audio';task.destination_name='NightFlight';task.automated=True;task.save=True;task.replace_existing=True
at.import_asset_tasks([task]);sound=unreal.load_asset('/Game/Runner/Audio/NightFlight');assert sound
sound.set_editor_property('looping',True);unreal.EditorAssetLibrary.save_loaded_asset(sound)
Path(str(Path(__file__).resolve().parents[1] / 'night-import.json')).write_text(json.dumps({'music':sound.get_path_name(),'duration':sound.get_editor_property('duration'),'materials':3},indent=2))
