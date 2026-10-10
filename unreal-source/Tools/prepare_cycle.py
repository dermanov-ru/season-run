import unreal
lib=unreal.MaterialEditingLibrary
for name,param,color in [('M_NightSky','SkyColor',(.005,.009,.028)),('M_Stars','StarColor',(.7,1.5,3))]:
 mat=unreal.load_asset('/Game/Runner/'+name)
 lib.delete_all_material_expressions(mat)
 c=lib.create_material_expression(mat,unreal.MaterialExpressionVectorParameter,-100,0)
 c.set_editor_property('parameter_name',param)
 c.set_editor_property('default_value',unreal.LinearColor(*color,1))
 lib.connect_material_property(c,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
 lib.recompile_material(mat)
 unreal.EditorAssetLibrary.save_loaded_asset(mat)
 print('RUNNER_CYCLE_READY: '+name)
