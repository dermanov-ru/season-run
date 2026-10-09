extends Node3D
var hero: Node3D
var camera: Camera3D
var environment: Environment
var trees: Array[Node3D]=[]
var chunks: Array[Node3D]=[]
var obstacles: Array[Node3D]=[]
var coins: Array[Node3D]=[]
var legs: Array[Node3D]=[]
var arms: Array[Node3D]=[]
var label: Label
var info: Label
var lane:=0
var distance:=0.0
var speed:=12.0
var y:=0.0
var vy:=0.0
var jumps:=0
var hearts:=3
var score:=0
var time:=0.0
var inv:=0.0
var paused:=true
var finish:=false
var mode := "level"
var current_level := 0
var levels: Array = []
var stars: Dictionary = {}
var armor := 0
var bonus := 0
var effect := ""
var effect_time := 0.0
var boost := 0.0
var season := 0.0
var hour := 9.0
var lost := 0
var smashed := 0
var jumped := 0
var boxes := 0
var powerups: Array[Node3D] = []
var menu: PanelContainer
var result: PanelContainer
var snowflakes: GPUParticles3D
var sun_light: DirectionalLight3D
var runner_anim: AnimationPlayer
var jump_buffer := 0.0
var snow: StandardMaterial3D
var dark: StandardMaterial3D
var wood: StandardMaterial3D
var gold: StandardMaterial3D
var coat: StandardMaterial3D
var rng:=RandomNumberGenerator.new()
func mat(c: Color,metal: float=0.0,emission: float=0.0)->StandardMaterial3D:
 var m:=StandardMaterial3D.new()
 m.albedo_color=c
 m.metallic=metal
 m.roughness=0.65
 if emission>0:
  m.emission_enabled=true
  m.emission=c
  m.emission_energy_multiplier=emission
 return m
func mesh(g: Mesh,p: Vector3,m: Material,parent: Node3D=self)->MeshInstance3D:
 var o:=MeshInstance3D.new()
 o.mesh=g
 o.material_override=m
 parent.add_child(o)
 o.position=p
 return o
func box(p: Vector3,s: Vector3,m: Material,parent: Node3D=self)->MeshInstance3D:
 var b:=BoxMesh.new()
 b.size=s
 return mesh(b,p,m,parent)
func sphere(p: Vector3,s: Vector3,m: Material,parent: Node3D=self)->MeshInstance3D:
 var b:=SphereMesh.new()
 b.radius=0.5
 b.height=1.0
 var o:=mesh(b,p,m,parent)
 o.scale=s
 return o
func cylinder(p: Vector3,r: float,h: float,m: Material,parent: Node3D=self)->MeshInstance3D:
 var b:=CylinderMesh.new()
 b.top_radius=r
 b.bottom_radius=r
 b.height=h
 return mesh(b,p,m,parent)
func _ready():
 rng.seed=45
 levels=JSON.parse_string(FileAccess.get_file_as_string("res://levels.json"))
 mode=get_tree().get_meta("run_mode","level")
 current_level=int(get_tree().get_meta("run_level",0))
 if FileAccess.file_exists("user://progress.json"):
  var saved=JSON.parse_string(FileAccess.get_file_as_string("user://progress.json"))
  if saved is Dictionary:stars=saved
 season=float(levels[current_level].season) if mode=="level" else 0.0
 hour=float(levels[current_level].hour) if mode=="level" else 9.0
 snow=mat(Color("e7edf3"))
 snow.albedo_texture=load("res://assets/snow_diff.jpg")
 snow.normal_enabled=true
 snow.normal_texture=load("res://assets/snow_normal.jpg")
 snow.normal_scale=0.65
 snow.uv1_scale=Vector3(2,2,2)
 dark=mat(Color("252d36"))
 wood=mat(Color("4a3327"))
 coat=mat(Color("d35035"))
 gold=mat(Color("ffc15b"),0.7,0.25)
 var worldenv:=WorldEnvironment.new()
 environment=Environment.new()
 var sky:=Sky.new()
 var sky_mat:=ProceduralSkyMaterial.new()
 sky_mat.sky_top_color=Color("203655")
 sky_mat.sky_horizon_color=Color("efd3b4")
 sky_mat.ground_horizon_color=Color("b6c4d4")
 sky_mat.ground_bottom_color=Color("5b7187")
 sky_mat.sun_angle_max=6.0
 sky.sky_material=sky_mat
 environment.background_mode=Environment.BG_SKY
 environment.sky=sky
 environment.ambient_light_source=Environment.AMBIENT_SOURCE_SKY
 environment.ambient_light_energy=0.4
 environment.tonemap_mode=Environment.TONE_MAPPER_FILMIC
 environment.tonemap_exposure=.85
 environment.fog_enabled=true
 environment.fog_density=0.003
 environment.ssao_enabled=true
 environment.glow_enabled=true
 environment.glow_intensity=.35
 environment.volumetric_fog_enabled=true
 environment.volumetric_fog_density=.004
 environment.volumetric_fog_albedo=Color("afc7e5")
 environment.fog_light_color=Color("9badc4")
 environment.fog_sun_scatter=0.8
 worldenv.environment=environment
 add_child(worldenv)
 var sun:=DirectionalLight3D.new()
 sun.rotation_degrees=Vector3(-17,-28,0)
 sun.light_color=Color("fff0dd")
 sun.light_energy=2.2
 sun.shadow_enabled=true
 sun.directional_shadow_max_distance=100
 add_child(sun)
 sun_light=sun
 box(Vector3(0,-.8,-100),Vector3(600,.5,600),snow)
 make_mountains()
 var pine: PackedScene=load("res://assets/pine_game.glb")
 for i in range(44):
  var t:=pine.instantiate() as Node3D
  add_child(t)
  t.position=Vector3((-1 if i%2 else 1)*rng.randf_range(8,25),-.5,-i*4.0+8)
  t.rotation.y=rng.randf_range(0,TAU)
  t.scale=Vector3.ONE*rng.randf_range(.65,1.2)
  trees.append(t)
 for i in range(12):
  var c:=Node3D.new()
  add_child(c)
  c.position.z=-i*16
  box(Vector3(0,-.18,0),Vector3(9,.35,16),snow,c)
  for side in [-1,1]:
   sphere(Vector3(side*7,-.25,0),Vector3(8,2,17),snow,c)
   for z in [-6,-2,2,6]:
    box(Vector3(side*4.5,.65,z),Vector3(.16,1.3,.16),wood,c)
   box(Vector3(side*4.5,.9,0),Vector3(.12,.15,16),wood,c)
   cylinder(Vector3(side*4,2.3,0),.055,4.6,dark,c)
   cylinder(Vector3(side*4,4.15,0),.28,.07,dark,c)
   sphere(Vector3(side*4,4.38,0),Vector3(.25,.38,.25),gold,c)
   cylinder(Vector3(side*4,4.62,0),.28,.08,dark,c)
   for xx in [-.2,.2]:
    for zz in [-.2,.2]:box(Vector3(side*4+xx,4.4,zz),Vector3(.025,.48,.025),dark,c)
   var lamp:=OmniLight3D.new()
   lamp.position=Vector3(side*4,4.4,0)
   lamp.light_color=Color("ffb263")
   lamp.light_energy=2
   lamp.omni_range=7
   c.add_child(lamp)
  # Twin faint wheel ruts.
  var rut:=mat(Color("a8bacb"))
  for x in [-2.2,2.2]:box(Vector3(x,.002,0),Vector3(.08,.009,16),rut,c)
  chunks.append(c)
 var rock_scene: PackedScene=load("res://assets/rock/rock.gltf")
 for i in range(24):
  var rock:=rock_scene.instantiate() as Node3D
  add_child(rock)
  rock.position=Vector3((-1 if i%2 else 1)*rng.randf_range(9,34),-.4,-rng.randf_range(0,180))
  rock.rotation.y=rng.randf_range(0,TAU)
  rock.scale=Vector3.ONE*rng.randf_range(1.3,3.8)
 # Winter explorer: articulated limbs and layered clothing.
 hero=Node3D.new()
 add_child(hero)
 sphere(Vector3(0,1.18,0),Vector3(.85,1.05,.6),coat,hero)
 sphere(Vector3(0,1.98,0),Vector3(.52,.55,.52),mat(Color("deb294")),hero)
 sphere(Vector3(0,2.13,.02),Vector3(.63,.42,.63),dark,hero)
 cylinder(Vector3(0,2.05,0),.31,.08,mat(Color("dde3e5")),hero)
 box(Vector3(0,1.25,.36),Vector3(.55,.62,.22),wood,hero)
 box(Vector3(0,1.75,.12),Vector3(.73,.15,.53),gold,hero)
 for side in [-1,1]:
  var leg:=Node3D.new()
  hero.add_child(leg)
  leg.position=Vector3(side*.22,.85,0)
  sphere(Vector3(0,-.35,0),Vector3(.28,.78,.28),dark,leg)
  box(Vector3(0,-.78,-.06),Vector3(.32,.18,.5),dark,leg)
  legs.append(leg)
  var arm:=Node3D.new()
  hero.add_child(arm)
  arm.position=Vector3(side*.48,1.55,0)
  sphere(Vector3(0,-.25,0),Vector3(.24,.75,.25),coat,arm)
  sphere(Vector3(0,-.6,0),Vector3(.25,.25,.25),dark,arm)
  arms.append(arm)
 for child in hero.get_children():child.hide()
 var runner_scene: PackedScene=load("res://assets/winter_runner.glb")
 var runner:=runner_scene.instantiate() as Node3D
 hero.add_child(runner)
 runner.rotation.y=PI
 runner.scale=Vector3.ONE*1.6
 var animations=runner.find_children("*","AnimationPlayer",true,false)
 if not animations.is_empty():
  runner_anim=animations[0]
  var names=runner_anim.get_animation_list()
  for animation_name in names:
   if animation_name!="RESET":
    runner_anim.get_animation(animation_name).loop_mode=Animation.LOOP_LINEAR
    runner_anim.play(animation_name)
    break
 for i in range(18):spawn_row(35+i*9)
 for i in range(14):
  spawn_power(["armor","mystery","heart","pad"][i%4],Vector3((i%3-1)*2.2,.8,-24-i*12))
 # A stone portal destination in the fog.
 var gate:=Node3D.new()
 add_child(gate)
 gate.position.z=-float(levels[current_level].length)
 box(Vector3(-4,3,0),Vector3(1.4,6,2),dark,gate)
 box(Vector3(4,3,0),Vector3(1.4,6,2),dark,gate)
 box(Vector3(0,6,0),Vector3(9.4,1,2),snow,gate)
 gate.set_meta("destination",true)
 camera=Camera3D.new()
 add_child(camera)
 camera.position=Vector3(0,3.6,7.4)
 camera.look_at(Vector3(0,1.9,-14))
 camera.fov=65
 camera.current=true
 # Fine snow particles.
 var flakes:=GPUParticles3D.new()
 add_child(flakes)
 snowflakes=flakes
 flakes.amount=550
 flakes.lifetime=5
 flakes.position=Vector3(0,8,-20)
 var process:=ParticleProcessMaterial.new()
 process.emission_shape=ParticleProcessMaterial.EMISSION_SHAPE_BOX
 process.emission_box_extents=Vector3(14,5,35)
 process.direction=Vector3(.2,-1,.1)
 process.gravity=Vector3(.2,-.7,0)
 process.initial_velocity_min=1
 process.initial_velocity_max=2
 process.scale_min=.02
 process.scale_max=.05
 flakes.process_material=process
 var flake_mesh:=SphereMesh.new()
 flake_mesh.radius=.5
 flake_mesh.height=1
 flake_mesh.material=mat(Color("edf6ff"),0,.2)
 flakes.draw_pass_1=flake_mesh
 var canvas:=CanvasLayer.new()
 add_child(canvas)
 var panel:=PanelContainer.new()
 panel.position=Vector2(28,28)
 canvas.add_child(panel)
 label=Label.new()
 label.add_theme_font_size_override("font_size",24)
 panel.add_child(label)
 info=Label.new()
 info.position=Vector2(28,95)
 info.add_theme_font_size_override("font_size",20)
 canvas.add_child(info)
 build_menu(canvas)
 if "--capture" in OS.get_cmdline_user_args():capture_preview.call_deferred()
 apply_season()
 print("WINTER_RUN_READY: 44 detailed pines, 3 lanes, jump, coins, 400m course")
func spawn_row(dist: float):
 var obstacle_lane:=rng.randi_range(-1,1)
 var root:=Node3D.new()
 add_child(root)
 root.position=Vector3(obstacle_lane*2.2,0,-dist)
 root.set_meta("lane",obstacle_lane)
 root.set_meta("distance",dist)
 root.set_meta("hit",false)
 # Snowman with scarf, coal, hat and carrot.
 sphere(Vector3(0,.65,0),Vector3(1.4,1.3,1.4),snow,root)
 sphere(Vector3(0,1.55,0),Vector3(1.02,1.05,1.02),snow,root)
 sphere(Vector3(0,2.25,0),Vector3(.74,.74,.74),snow,root)
 cylinder(Vector3(0,2.65,0),.46,.07,dark,root)
 cylinder(Vector3(0,2.85,0),.29,.4,dark,root)
 box(Vector3(0,1.91,0),Vector3(1.05,.16,.92),coat,root)
 for x in [-.15,.15]:sphere(Vector3(x,2.35,.34),Vector3(.08,.08,.06),dark,root)
 sphere(Vector3(0,2.22,.43),Vector3(.12,.13,.45),coat,root)
 for yy in [.5,.85,1.5]:sphere(Vector3(0,yy,.66),Vector3(.09,.09,.08),dark,root)
 obstacles.append(root)
 for j in range(3):
  var c:=Node3D.new()
  add_child(c)
  c.position=Vector3(((obstacle_lane+2)%3-1)*2.2,1.1,-dist+3+j*1.2)
  c.set_meta("distance",dist-3-j*1.2)
  var shape:=CylinderMesh.new()
  shape.top_radius=.23
  shape.bottom_radius=.23
  shape.height=.07
  var coin:=mesh(shape,Vector3.ZERO,gold,c)
  coin.rotation.x=PI/2
  coins.append(c)
func _unhandled_input(event):
 if event is InputEventKey and event.pressed and not event.echo:
  match event.physical_keycode:
   KEY_ENTER:
    if finish:restart_run(mode,current_level)
    elif menu.visible:menu.hide();paused=false
    else:paused=false
   KEY_P,KEY_ESCAPE:
    paused=not paused
    if not finish:menu.visible=paused
   KEY_R:restart_run(mode,current_level)
   KEY_A,KEY_LEFT:lane=max(-1,lane+1) if effect=="reverse" else max(-1,lane-1)
   KEY_D,KEY_RIGHT:lane=min(1,lane-1) if effect=="reverse" else min(1,lane+1)
   KEY_SPACE,KEY_UP:
    if not paused:
     if jumps<2:do_jump()
     else:jump_buffer=.14
func _process(dt):
 if not is_instance_valid(hero):return
 if paused or finish:
  if is_instance_valid(runner_anim):runner_anim.speed_scale=0
  if finish:info.text="Enter / R — ещё раз"
  else:info.text="ENTER — старт   |   ← → / A D — дорожка   |   Пробел — двойной прыжок\nP — пауза   •   R — заново   •   Зимний перевал, 400 м"
  label.text=hud_text()
  return
 info.text="← → — дорожка    •    Пробел × 2 — двойной прыжок    •    P — пауза"
 time+=dt
 if is_instance_valid(runner_anim):runner_anim.speed_scale=1.5 if y<.1 else .3
 speed=min(26,11+current_level*.9+time*.05) if mode=="level" else min(26,12+time*.12)
 if boost>0:speed*=1.6
 effect_time=max(0,effect_time-dt)
 boost=max(0,boost-dt)
 jump_buffer=max(0,jump_buffer-dt)
 if effect_time<=0:effect=""
 if mode=="endless":season=fmod(season+dt/35,4.0)
 hour=fmod(hour+dt*24/240,24.0)
 apply_season()
 hero.scale=Vector3.ONE*(1.8 if effect=="giant" else 1.0)
 var step: float=speed*dt
 distance+=step
 inv=max(0,inv-dt)
 hero.position.x=lerp(hero.position.x,lane*2.2,min(1,dt*(2.2 if effect=="ice" else 12)))
 if effect=="rocket":
  y=lerp(y,2.6,min(1,dt*4));vy=0;jumps=2
 elif y>0 or vy>0:
  vy-=20.4*dt
  y+=vy*dt
  if y<=0:
   y=0;vy=0;jumps=0
   if jump_buffer>0:do_jump();jump_buffer=0
 hero.position.y=y
 hero.visible=inv<=0 or int(inv*12)%2==0
 for i in range(2):
  legs[i].rotation.x=sin(time*speed*.75+i*PI)*.7 if y<.1 else -.4
  arms[i].rotation.x=-sin(time*speed*.75+i*PI)*.55
 for t in trees:
  t.position.z+=step
  if t.position.z>20:t.position.z-=180
 for c in chunks:
  c.position.z+=step
  if c.position.z>16:c.position.z-=192
 for o in obstacles:
  o.position.z+=step
  if not o.get_meta("hit") and o.position.z>=-.65 and o.position.z<1 and abs(o.position.x-hero.position.x)<.8 and y<2.65:
   o.set_meta("hit",true)
   if armor>0 or boost>0 or effect=="giant":
    if armor>0:armor-=1
    smashed+=1;bonus+=20;o.visible=false
   elif inv<=0:
    hearts-=1;lost+=1;inv=1.6;o.visible=false
    if hearts<=0:end_run(false)
  if not o.get_meta("hit") and o.position.z>=0 and abs(o.position.x-hero.position.x)<.8 and y>=2.65:
   jumped+=1;bonus+=15;o.set_meta("hit",true)
  if o.position.z>12:
   o.position.z-=180
   o.set_meta("hit",false)
   o.visible=true
 for c in coins:
  c.position.z+=step
  c.rotation.y+=dt*2
  if effect=="magnet" and c.position.z>-9 and c.visible:c.position.x=lerp(c.position.x,hero.position.x,dt*6)
  if c.visible and abs(c.position.z)<.8 and abs(c.position.x-hero.position.x)<.7 and y<1.9:
   c.visible=false
   score+=1
  if c.position.z>12:
   c.position.z-=180
   c.visible=true
 for n in get_children():
  if n.has_meta("destination"):n.position.z+=step
 camera.position.x=lerp(camera.position.x,hero.position.x*.28,dt*4)
 update_powerups(step)
 if mode=="level" and distance>=float(levels[current_level].length):end_run(true)
 label.text=hud_text()

func do_jump():
 vy=7.14 if jumps==0 else 7.14*.88
 jumps+=1
func hud_text()->String:
 var seasons=["Зима","Весна","Лето","Осень"]
 var title: String=levels[current_level].name if mode=="level" else "Бесконечный забег"
 var goal: String=(" / %d"%int(levels[current_level].length)) if mode=="level" else ""
 return "%s   •   %s
Монеты %d    Жизни %d    Броня %d    •    %d%s м%s"%[title,seasons[int(round(season))%4],score,hearts,armor,distance,goal,("   •   "+effect) if effect!="" else ""]
func restart_run(new_mode: String, index: int):
 get_tree().set_meta("run_mode",new_mode)
 get_tree().set_meta("run_level",index)
 get_tree().reload_current_scene()
func build_menu(canvas: CanvasLayer):
 menu=PanelContainer.new()
 menu.position=Vector2(760,90)
 menu.custom_minimum_size=Vector2(485,610)
 canvas.add_child(menu)
 var margin:=MarginContainer.new()
 for name in ["margin_left","margin_right","margin_top","margin_bottom"]:margin.add_theme_constant_override(name,20)
 menu.add_child(margin)
 var column:=VBoxContainer.new()
 column.add_theme_constant_override("separation",8)
 margin.add_child(column)
 var title:=Label.new()
 title.text="БЕГ СКВОЗЬ СЕЗОНЫ"
 title.add_theme_font_size_override("font_size",27)
 column.add_child(title)
 var resume:=Button.new()
 resume.text="▶  Начать / продолжить — Enter"
 resume.pressed.connect(func():menu.hide();paused=false)
 column.add_child(resume)
 var endless:=Button.new()
 endless.text="∞  Бесконечный забег со сменой сезонов"
 endless.pressed.connect(func():restart_run("endless",0))
 column.add_child(endless)
 var scroll:=ScrollContainer.new()
 scroll.custom_minimum_size=Vector2(440,430)
 column.add_child(scroll)
 var rows:=VBoxContainer.new()
 rows.size_flags_horizontal=Control.SIZE_EXPAND_FILL
 scroll.add_child(rows)
 for i in range(levels.size()):
  var b:=Button.new()
  var l: Dictionary=levels[i]
  var n: int=int(stars.get(str(i),0))
  b.text="%d. %s   %s"%[i+1,l.name,"★".repeat(n)]
  b.tooltip_text="%s • %d м • Цель: %s %d"%[l.region,l.length,l.goal,l.n]
  b.alignment=HORIZONTAL_ALIGNMENT_LEFT
  b.disabled=i>0 and int(stars.get(str(i-1),0))==0
  b.pressed.connect(restart_run.bind("level",i))
  rows.add_child(b)
 result=PanelContainer.new()
 result.position=Vector2(320,240)
 result.custom_minimum_size=Vector2(640,260)
 canvas.add_child(result)
 result.hide()
func end_run(success: bool):
 if finish:return
 finish=true
 var earned:=0
 if success:
  var l: Dictionary=levels[current_level]
  var value: int=score if l.goal=="coins" else jumped if l.goal=="jump" else smashed if l.goal=="smash" else boxes if l.goal=="boxes" else 0
  var done: bool=lost==0 if l.goal=="nohit" else value>=int(l.n)
  earned=1+int(done)+int(score>=30 if l.goal=="nohit" else lost==0)
  stars[str(current_level)]=max(int(stars.get(str(current_level),0)),earned)
  var file:=FileAccess.open("user://progress.json",FileAccess.WRITE)
  file.store_string(JSON.stringify(stars))
 var column:=VBoxContainer.new()
 result.add_child(column)
 result.show()
 var msg:=Label.new()
 msg.add_theme_font_size_override("font_size",28)
 msg.horizontal_alignment=HORIZONTAL_ALIGNMENT_CENTER
 msg.text=("Уровень пройден!  "+"★".repeat(earned)) if success else "Забег окончен"
 column.add_child(msg)
 var stats:=Label.new()
 stats.horizontal_alignment=HORIZONTAL_ALIGNMENT_CENTER
 stats.text="%d м • %d монет • %d очков\nПрыжки: %d • Разрушения: %d • Сюрпризы: %d"%[distance,score,int(distance)+score*10+bonus,jumped,smashed,boxes]
 column.add_child(stats)
 var retry:=Button.new()
 retry.text="Ещё раз"
 retry.pressed.connect(restart_run.bind(mode,current_level))
 column.add_child(retry)
 if success and current_level<11:
  var next:=Button.new()
  next.text="Следующий уровень"
  next.pressed.connect(restart_run.bind("level",current_level+1))
  column.add_child(next)
 var select:=Button.new()
 select.text="Выбор уровней"
 select.pressed.connect(func():result.hide();menu.show())
 column.add_child(select)
func apply_season():
 var colors=[Color("e7edf3"),Color("86a178"),Color("739572"),Color("b69b72")]
 var k:=int(floor(season))%4
 var blend: float=season-floor(season)
 snow.albedo_color=colors[k].lerp(colors[(k+1)%4],blend)
 var day: float=max(.12,sin((hour-6)*PI/12))
 if is_instance_valid(sun_light):sun_light.light_energy=day*1.15
 if is_instance_valid(snowflakes):snowflakes.emitting=int(round(season))%4==0
func spawn_power(kind: String,position_: Vector3):
 var root:=Node3D.new()
 add_child(root)
 root.position=position_
 root.set_meta("kind",kind)
 var color: Color=Color("75caff") if kind=="armor" else Color("ee77e0") if kind=="mystery" else Color("ff6262") if kind=="heart" else Color("52efe0")
 var m:=mat(color,.3,.6)
 if kind=="pad":box(Vector3(0,-.75,0),Vector3(1.4,.04,1.6),m,root)
 else:
  sphere(Vector3.ZERO,Vector3(.55,.55,.55),m,root)
  var text:=Label3D.new()
  text.text="◈" if kind=="armor" else "?" if kind=="mystery" else "+"
  text.font_size=64
  text.pixel_size=.01
  text.position.y=.55
  text.billboard=BaseMaterial3D.BILLBOARD_ENABLED
  root.add_child(text)
 powerups.append(root)
func update_powerups(step: float):
 for p in powerups:
  p.position.z+=step
  if p.visible and abs(p.position.z)<.9 and abs(p.position.x-hero.position.x)<.75 and y<1.7:
   p.hide()
   var kind: String=p.get_meta("kind")
   match kind:
    "armor":armor=3
    "heart":hearts=min(5,hearts+1)
    "pad":boost=5
    "mystery":
     boxes+=1
     var effects=["magnet","giant","rocket","coinrain","reverse","ice"]
     effect=effects[rng.randi_range(0,5)]
     effect_time={"magnet":7.0,"giant":6.0,"rocket":4.5,"coinrain":3.0,"reverse":6.0,"ice":6.0}[effect]
     if effect=="coinrain":score+=20
  if p.position.z>12:p.position.z-=180;p.show()

func capture_preview():
 menu.hide()
 await get_tree().create_timer(3).timeout
 await RenderingServer.frame_post_draw
 var image: Image=get_viewport().get_texture().get_image()
 image.save_png("res://WinterPass-preview.png")
 print("PREVIEW_CAPTURED")
 get_tree().quit()

func make_mountains():
 var surface:=SurfaceTool.new()
 surface.begin(Mesh.PRIMITIVE_TRIANGLES)
 var noise:=FastNoiseLite.new()
 noise.seed=884
 noise.frequency=.012
 var step_size:=6.0
 for ix in range(70):
  for iz in range(50):
   var a:=Vector2((ix-35)*step_size,-30-iz*step_size)
   for offset in [Vector2(0,0),Vector2(0,step_size),Vector2(step_size,0),Vector2(step_size,0),Vector2(0,step_size),Vector2(step_size,step_size)]:
    var p: Vector2=a+offset
    var mask: float=clamp((abs(p.x)-40)/55,0.0,1.0)
    var height: float=pow(abs(noise.get_noise_2d(p.x,p.y)),.65)*95*mask-1
    surface.set_uv(p/16)
    surface.add_vertex(Vector3(p.x,height,p.y))
 surface.generate_normals()
 var terrain:=surface.commit()
 mesh(terrain,Vector3.ZERO,snow)
