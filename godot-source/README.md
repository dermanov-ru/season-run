# Season Run — Godot graphics prototype

Separate graphics prototype based on the local original ../index.html. Original repository is untouched.

## Play
Open project.godot in Godot 4.7+, F5. Enter starts, arrows/A/D switch three lanes, Space/Up jumps twice, P/Esc pauses, R restarts. The menu includes the original 12 level definitions and an endless mode. Stars unlock later levels and save to user://progress.json.

## Implemented
Original gravity (20.4), jump impulses (7.14 and 88%), lane interpolation, speed progression, snowmen, coins, lives, armor, speed pads, six surprise types, objectives/stars, pause/restart, seasonal palette and day/night light. Modern Metal Forward+ lighting, volumetric fog, SSAO, snow normal map, detailed optimized trees/rocks, animated skinned runner.

## Remaining differences from the original
This is a winter environment for visual comparison, not the completed port. Village/meadow/river/sea biomes, pits, cows, throwing and moving snowmen, full seasonal obstacle transformation, exact surprise effects/coin showers, and mobile swipe controls have not been ported. Current coin-rain awards coins immediately; level goals and definitions are present but all levels currently share the winter environment.

## Credits
- Game design and source: dermanov-ru/season-run.
- Poly Haven pine_tree_01, rock_07, snow_02: CC0, https://polyhaven.com/a/pine_tree_01 , https://polyhaven.com/a/rock_07 , https://polyhaven.com/a/snow_02 . Pine mesh reduced to about 243k polygons; source remains in assets/pine, excluded from Godot import.
- CesiumMan animated character, ©2017 Cesium, CC BY 4.0; https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/CesiumMan . See assets/runner-license.md. The character uses the source Cesium logo.

Verification: verify.gd checks jump impulses, lane movement, armor, heart pickup, world recycling and 12 level definitions.

Character update: modified CesiumMan into a winter explorer with differentiated fabric/skin materials, quilted parka, knitted hat, scarf, boots and canvas backpack. winter_runner.glb retains the running rig and animation. Original license/attribution remains applicable.
