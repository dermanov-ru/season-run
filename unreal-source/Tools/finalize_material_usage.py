import unreal
for name in ['M_Amber','M_Turbo','M_NightSky','M_Stars','M_Meadow','M_Snow']:
 mat=unreal.load_asset('/Game/Runner/'+name);assert mat
 for usage in [unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES,unreal.MaterialUsage.MATUSAGE_NANITE]:
  unreal.MaterialEditingLibrary.set_material_usage(mat,usage)
 unreal.MaterialEditingLibrary.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
