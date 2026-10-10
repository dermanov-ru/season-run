import unreal,json
from pathlib import Path
at=unreal.AssetToolsHelpers.get_asset_tools()
lib=unreal.MaterialEditingLibrary
mat=unreal.load_asset('/Game/Runner/M_Snow') or at.create_asset('M_Snow','/Game/Runner',unreal.Material,unreal.MaterialFactoryNew())
lib.delete_all_material_expressions(mat)
def node(cls,x,y): return lib.create_material_expression(mat,cls,x,y)
world=node(unreal.MaterialExpressionWorldPosition,-600,0)
mask=node(unreal.MaterialExpressionComponentMask,-450,0); mask.set_editor_property('r',True);mask.set_editor_property('g',True);mask.set_editor_property('b',False);mask.set_editor_property('a',False)
assert lib.connect_material_expressions(world,'',mask,'')
scale=node(unreal.MaterialExpressionMultiply,-300,0);scale.set_editor_property('const_b',.003)
lib.connect_material_expressions(mask,'',scale,'A')
tex=node(unreal.MaterialExpressionTextureSample,-100,0);tex.set_editor_property('texture',unreal.load_asset('/Game/Runner/Environment/Snow/snow_diff'))
lib.connect_material_expressions(scale,'',tex,'UVs')
desat=node(unreal.MaterialExpressionDesaturation,80,0);assert lib.connect_material_expressions(tex,'RGB',desat,'')
white=node(unreal.MaterialExpressionLinearInterpolate,250,0);white.set_editor_property('const_a',.65);white.set_editor_property('const_b',.95);lib.connect_material_expressions(desat,'',white,'Alpha');lib.connect_material_property(white,'',unreal.MaterialProperty.MP_BASE_COLOR)
rough=node(unreal.MaterialExpressionConstant,-100,180);rough.set_editor_property('r',.78)
lib.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
glow=unreal.load_asset('/Game/Runner/M_Amber') or at.create_asset('M_Amber','/Game/Runner',unreal.Material,unreal.MaterialFactoryNew())
lib.delete_all_material_expressions(glow)
color=lib.create_material_expression(glow,unreal.MaterialExpressionConstant3Vector,-100,0);color.set_editor_property('constant',unreal.LinearColor(3,1,.12,1))
lib.connect_material_property(color,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR);lib.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
lib.recompile_material(glow);unreal.EditorAssetLibrary.save_loaded_asset(glow)
report={}
for n,path in [('pine','/Game/Runner/Environment/Pine/pine_game/StaticMeshes/pine_game'),('rock','/Game/Runner/Environment/Rock/rock/StaticMeshes/rock')]:
 m=unreal.load_asset(path);b=m.get_bounds();report[n]={'origin':str(b.origin),'extent':str(b.box_extent)}
Path(str(Path(__file__).resolve().parents[1] / 'art-bounds.json')).write_text(json.dumps(report,indent=2))

for path in unreal.EditorAssetLibrary.list_assets('/Game/Runner/Environment',recursive=True):
 material=unreal.load_asset(path)
 if isinstance(material,unreal.MaterialInstanceConstant):
  for usage in [unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES,unreal.MaterialUsage.MATUSAGE_NANITE]:
   lib.set_material_usage_override(material,usage,True,True)
  unreal.EditorAssetLibrary.save_loaded_asset(material)
