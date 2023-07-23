#include <juce_graphics/juce_graphics.h>
#include <minecraft-file.hpp>

#include "Palette.hpp"

using namespace juce;
using namespace mcfile::blocks::minecraft;

namespace mcview {
namespace {
static Colour const kColorPotter(135, 75, 58);
static Colour const kColorPlanksBirch(244, 230, 161);
static Colour const kColorPlanksDarkOak(101, 75, 50);
static Colour const kColorPlanksOak(127, 85, 48);
static Colour const kColorPlanksJungle(149, 108, 76);
static Colour const kColorPlanksSpruce(122, 89, 51);
static Colour const kColorPlanksCrimson(125, 57, 85);
static Colour const kColorPlanksWarped(56, 129, 128);
static Colour const kColorPlanksManvrove(137, 76, 57);
static Colour const kColorPlanksAcacia(184, 98, 55);
static Colour const kColorPlanksBamboo(224, 202, 105);
static Colour const kColorPlanksCherry(228, 200, 195);
static Colour const kColorLogOak(141, 118, 71);
static Colour const kColorLogSpruce(58, 39, 19);
static Colour const kColorLogCherry(83, 52, 66);
static Colour const kColorLogCherryStripped(214, 146, 151);
static Colour const kColorLeavesCherry(243, 159, 209);
static Colour const kColorBricks(175, 98, 76);
static Colour const kColorAnvil(73, 73, 73);
static Colour const kColorDeadCoral(115, 105, 102);
static Colour const kColorRail(154, 154, 154);
static Colour const kColorStoneDiorite(252, 249, 242);
static Colour const kColorStoneGranite(149, 108, 76);
static Colour const kColorStoneAndesite(165, 168, 151);
static Colour const kColorStone(111, 111, 111);
static Colour const kColorStoneBlack(48, 43, 53);
static Colour const kColorStonePolishedBlack(59, 56, 70);
static Colour const kColorNetherBricks(33, 17, 20);
static Colour const kColorFurnace(131, 131, 131);
static Colour const kColorEndStoneBricks(233, 248, 173);
static Colour const kColorPolishedBlackStoneBricks(32, 28, 23);
static Colour const kColorRedSandstone(184, 102, 33);
static Colour const kColorSand(201, 192, 154);
static Colour const kColorDragonHead(22, 22, 22);
static Colour const kColorQuartz(235, 227, 219);
static Colour const kColorMossyStone(115, 131, 82);
static Colour const kColorPistonHead(186, 150, 97);
static Colour const kColorPurPur(170, 122, 170);
static Colour const kColorPrismarine(75, 125, 151);
static Colour const kColorRedNetherBricks(89, 0, 0);
static Colour const kColorCreaperHead(96, 202, 77);
static Colour const kColorPlayerHead(46, 31, 14);
static Colour const kColorSkeltonSkull(186, 186, 186);
static Colour const kColorPiglinHead(239, 184, 132);
static Colour const kColorChest(141, 118, 71);
static Colour const kColorWitherSkeltonSkull(31, 31, 31);
static Colour const kColorZombieHead(61, 104, 45);
static Colour const kColorDeepslate(104, 104, 104);
static Colour const kColorCopper(224, 128, 107);
static Colour const kColorExposedCopper(150, 138, 104);
static Colour const kColorWeatheredCopper(99, 158, 118);
static Colour const kColorOxidizedCopper(75, 146, 130);
static Colour const kColorMudBrick(147, 112, 79);
static Colour const kColorSculkSensor(7, 71, 86);
static Colour const kColorSculk(5, 41, 49);
static Colour const kColorBookshelf(192, 155, 97);
static Colour const kColorTorchflower(243, 183, 39);
static Colour const kColorPitcher(188, 154, 252);

static Colour const kColorGravel = kColorStone;

std::unordered_map<mcfile::blocks::BlockId, juce::Colour> *CreateTable() {
  auto ret = new std::unordered_map<mcfile::blocks::BlockId, Colour>();

#define C(id, r, g, b) (*ret)[(id)] = Colour((r), (g), (b));
#define K(id, n) (*ret)[(id)] = (n);

  K(stone, kColorStone)
  K(granite, kColorStoneGranite)
  K(diorite, kColorStoneDiorite)
  K(andesite, kColorStoneAndesite)
  K(chest, kColorChest)
  C(clay, 162, 166, 182)
  C(coal_ore, 111, 111, 111)
  C(cobblestone, 111, 111, 111)
  C(dirt, 149, 108, 76)
  C(brown_mushroom, 0, 123, 0)
  C(grass_block, 130, 148, 58)
  C(iron_ore, 111, 111, 111)
  K(sand, kColorSand)
  C(oak_leaves, 56, 95, 31)
  C(jungle_leaves, 56, 95, 31)
  C(birch_leaves, 67, 124, 37)
  C(red_mushroom, 0, 123, 0)
  K(mossy_cobblestone, kColorMossyStone)
  K(oak_stairs, kColorPlanksOak)
  K(gravel, kColorGravel)
  K(oak_log, kColorLogOak)
  K(oak_planks, kColorPlanksOak)
  C(farmland, 149, 108, 76)
  K(oak_fence, kColorPlanksOak)
  C(cobblestone_stairs, 111, 111, 111)
  C(dirt_path, 204, 204, 204)
  K(birch_fence, kColorPlanksBirch)
  K(birch_planks, kColorPlanksBirch)
  K(birch_stairs, kColorPlanksBirch)
  K(dark_oak_fence, kColorPlanksDarkOak)
  C(dark_oak_log, 101, 75, 50)
  K(dark_oak_planks, kColorPlanksDarkOak)
  K(dark_oak_slab, kColorPlanksDarkOak)
  K(dark_oak_stairs, kColorPlanksDarkOak)
  C(dark_oak_trapdoor, 141, 118, 71)
  C(diamond_ore, 111, 111, 111)
  C(gold_ore, 111, 111, 111)
  C(ice, 158, 158, 252)
  K(jungle_fence, kColorPlanksJungle)
  C(jungle_log, 149, 108, 76)
  K(jungle_planks, kColorPlanksJungle)
  K(jungle_slab, kColorPlanksJungle)
  K(jungle_stairs, kColorPlanksJungle)
  K(jungle_button, kColorPlanksJungle)
  K(jungle_door, kColorPlanksJungle)
  C(jungle_trapdoor, 141, 118, 71)
  C(lapis_ore, 111, 111, 111)
  C(lava, 179, 71, 3)
  C(oak_door, 141, 118, 71)
  K(oak_slab, kColorPlanksOak)
  C(oak_trapdoor, 141, 118, 71)
  C(obsidian, 29, 14, 52)
  C(packed_ice, 158, 158, 252)
  K(polished_granite, kColorStoneGranite)
  K(prismarine, kColorPrismarine)
  C(prismarine_bricks, 91, 216, 210)
  K(rail, kColorRail)
  C(redstone_ore, 111, 111, 111)
  K(sandstone, kColorSand)
  C(sea_lantern, 252, 249, 242)
  C(snow, 229, 229, 229)
  C(snow_block, 252, 252, 252)
  C(powder_snow, 252, 252, 252)
  K(spruce_door, kColorPlanksSpruce)
  K(spruce_fence, kColorPlanksSpruce)
  C(spruce_leaves, 56, 95, 31)
  C(stone_brick_stairs, 111, 111, 111)
  C(stone_bricks, 111, 111, 111)
  C(stone_slab, 111, 111, 111)
  K(spruce_log, kColorLogSpruce)
  K(spruce_planks, kColorPlanksSpruce)
  K(spruce_slab, kColorPlanksSpruce)
  K(spruce_stairs, kColorPlanksSpruce)
  K(spruce_trapdoor, kColorPlanksSpruce)
  K(mossy_stone_bricks, kColorMossyStone)
  K(chiseled_stone_bricks, kColorStone)
  K(cracked_stone_bricks, kColorStone)
  C(infested_stone, 111, 111, 111)
  C(cobweb, 255, 255, 255)
  C(blue_ice, 102, 151, 246)
  C(magma_block, 181, 64, 9)
  C(end_stone, 219, 219, 172)
  C(end_portal, 4, 18, 24)
  C(end_portal_frame, 65, 114, 102)
  C(bedrock, 111, 111, 111)
  C(mycelium, 114, 96, 97)
  C(white_terracotta, 209, 180, 161)
  C(orange_terracotta, 165, 82, 40)
  C(magenta_terracotta, 147, 87, 108)
  C(light_blue_terracotta, 110, 106, 135)
  C(yellow_terracotta, 184, 129, 33)
  C(lime_terracotta, 102, 116, 52)
  C(pink_terracotta, 160, 77, 78)
  C(gray_terracotta, 57, 41, 36)
  C(light_gray_terracotta, 137, 106, 99)
  C(cyan_terracotta, 90, 94, 93)
  C(purple_terracotta, 117, 69, 86)
  C(blue_terracotta, 73, 58, 90)
  C(brown_terracotta, 76, 50, 36)
  C(green_terracotta, 75, 82, 41)
  C(red_terracotta, 139, 58, 45)
  C(black_terracotta, 37, 22, 15)
  C(terracotta, 153, 95, 68)
  C(red_sand, 201, 109, 36)
  K(purpur_slab, kColorPurPur)
  K(purpur_block, kColorPurPur)
  K(purpur_pillar, kColorPurPur)
  C(ender_chest, 39, 54, 56)
  C(tnt, 216, 46, 26)
  K(prismarine_slab, kColorPrismarine)
  K(prismarine_stairs, kColorPrismarine)
  C(prismarine_bricks, 89, 173, 162)
  C(prismarine_brick_slab, 89, 173, 162)
  C(prismarine_brick_stairs, 89, 173, 162)
  C(dark_prismarine, 55, 97, 80)
  C(dark_prismarine_slab, 55, 97, 80)
  C(dark_prismarine_stairs, 55, 97, 80)
  C(netherrack, 86, 32, 31)
  K(nether_bricks, kColorNetherBricks)
  K(nether_brick_slab, kColorNetherBricks)
  K(nether_brick_wall, kColorNetherBricks)
  K(red_nether_bricks, kColorRedNetherBricks)
  K(red_nether_brick_slab, kColorRedNetherBricks)
  K(red_nether_brick_wall, kColorRedNetherBricks)
  C(glowstone, 248, 215, 115)
  C(nether_quartz_ore, 170, 112, 105)
  C(soul_sand, 72, 54, 43)
  C(white_wool, 247, 247, 247)
  C(orange_wool, 244, 122, 25)
  C(magenta_wool, 193, 73, 183)
  C(light_blue_wool, 65, 186, 220)
  C(yellow_wool, 249, 206, 47)
  C(lime_wool, 123, 193, 27)
  C(pink_wool, 241, 160, 186)
  C(gray_wool, 70, 78, 81)
  C(light_gray_wool, 151, 151, 145)
  C(cyan_wool, 22, 153, 154)
  C(purple_wool, 132, 47, 179)
  C(blue_wool, 57, 63, 164)
  C(brown_wool, 125, 79, 46)
  C(green_wool, 91, 119, 22)
  C(red_wool, 170, 42, 36)
  C(black_wool, 28, 28, 32)
  C(white_carpet, 247, 247, 247)
  C(orange_carpet, 244, 122, 25)
  C(magenta_carpet, 193, 73, 183)
  C(light_blue_carpet, 65, 186, 220)
  C(yellow_carpet, 249, 206, 47)
  C(lime_carpet, 123, 193, 27)
  C(pink_carpet, 241, 160, 186)
  C(gray_carpet, 70, 78, 81)
  C(light_gray_carpet, 151, 151, 145)
  C(cyan_carpet, 22, 153, 154)
  C(purple_carpet, 132, 47, 179)
  C(blue_carpet, 57, 63, 164)
  C(brown_carpet, 125, 79, 46)
  C(green_carpet, 91, 119, 22)
  C(red_carpet, 170, 42, 36)
  C(black_carpet, 28, 28, 32)
  C(white_bed, 247, 247, 247)
  C(orange_bed, 244, 122, 25)
  C(magenta_bed, 193, 73, 183)
  C(light_blue_bed, 65, 186, 220)
  C(yellow_bed, 249, 206, 47)
  C(lime_bed, 123, 193, 27)
  C(pink_bed, 241, 160, 186)
  C(gray_bed, 70, 78, 81)
  C(light_gray_bed, 151, 151, 145)
  C(cyan_bed, 22, 153, 154)
  C(purple_bed, 132, 47, 179)
  C(blue_bed, 57, 63, 164)
  C(brown_bed, 125, 79, 46)
  C(green_bed, 91, 119, 22)
  C(red_bed, 170, 42, 36)
  C(black_bed, 28, 28, 32)
  C(dried_kelp_block, 43, 55, 32)
  C(beacon, 72, 210, 202)
  C(shulker_box, 149, 101, 149)
  C(white_shulker_box, 225, 230, 230)
  C(orange_shulker_box, 225, 230, 230)
  C(magenta_shulker_box, 183, 61, 172)
  C(light_blue_shulker_box, 57, 177, 215)
  C(yellow_shulker_box, 249, 194, 34)
  C(lime_shulker_box, 108, 183, 24)
  C(pink_shulker_box, 239, 135, 166)
  C(gray_shulker_box, 59, 63, 67)
  C(light_gray_shulker_box, 135, 135, 126)
  C(cyan_shulker_box, 22, 133, 144)
  C(purple_shulker_box, 115, 38, 167)
  C(blue_shulker_box, 49, 52, 152)
  C(brown_shulker_box, 111, 69, 39)
  C(green_shulker_box, 83, 107, 29)
  C(red_shulker_box, 152, 35, 33)
  C(black_shulker_box, 31, 31, 34)
  K(bricks, kColorBricks)
  K(cut_sandstone, kColorSand)
  K(sandstone_stairs, kColorSand)
  K(chiseled_sandstone, kColorSand)
  K(sandstone_slab, kColorSand)
  K(dark_oak_door, kColorPlanksDarkOak)
  K(polished_diorite, kColorStoneDiorite)
  C(coarse_dirt, 96, 67, 45)
  C(acacia_log, 104, 97, 88)
  K(oak_pressure_plate, kColorPlanksOak)
  C(fire, 202, 115, 3)
  C(cobblestone_wall, 111, 111, 111)
  C(cobblestone_slab, 111, 111, 111)
  C(podzol, 105, 67, 23)
  C(sticky_piston, 131, 131, 131)
  K(piston_head, kColorPistonHead)
  C(piston, 131, 131, 131)
  C(lever, 134, 133, 134)
  C(observer, 100, 100, 100)
  C(slime_block, 112, 187, 94)
  K(activator_rail, kColorRail)
  K(oak_fence_gate, kColorPlanksOak)
  K(dark_oak_fence_gate, kColorPlanksDarkOak)
  K(birch_button, kColorPlanksBirch)
  K(birch_slab, kColorPlanksBirch)
  K(acacia_stairs, kColorPlanksAcacia)
  K(acacia_pressure_plate, kColorPlanksAcacia)
  C(redstone_wire, 75, 1, 0)
  C(redstone_block, 162, 24, 8)
  C(redstone_lamp, 173, 104, 58)
  C(hopper, 70, 70, 70)
  C(crafting_table, 156, 88, 49)
  C(lectern, 164, 128, 73)
  K(acacia_planks, kColorPlanksAcacia)
  K(acacia_wood, kColorPlanksAcacia)
  K(acacia_slab, kColorPlanksAcacia)
  K(acacia_fence, kColorPlanksAcacia)
  K(acacia_fence_gate, kColorPlanksAcacia)
  K(acacia_door, kColorPlanksAcacia)
  K(stripped_acacia_log, kColorPlanksAcacia)
  C(brain_coral, 225, 125, 183)
  C(brain_coral_block, 225, 125, 183)
  C(brain_coral_fan, 225, 125, 183)
  C(brain_coral_wall_fan, 225, 125, 183)
  C(bubble_coral, 198, 25, 184)
  C(bubble_coral_block, 198, 25, 184)
  C(bubble_coral_fan, 198, 25, 184)
  C(bubble_coral_wall_fan, 198, 25, 184)
  C(horn_coral, 234, 233, 76)
  C(horn_coral_block, 234, 233, 76)
  C(horn_coral_fan, 234, 233, 76)
  C(horn_coral_wall_fan, 234, 233, 76)
  C(tube_coral, 48, 78, 218)
  C(tube_coral_block, 48, 78, 218)
  C(tube_coral_fan, 48, 78, 218)
  C(tube_coral_wall_fan, 48, 78, 218)
  C(fire_coral, 196, 42, 54)
  C(fire_coral_block, 196, 42, 54)
  C(fire_coral_fan, 196, 42, 54)
  C(fire_coral_wall_fan, 196, 42, 54)
  K(smooth_sandstone, kColorSand)
  K(smooth_sandstone_slab, kColorSand)
  K(smooth_sandstone_stairs, kColorSand)
  K(sandstone_wall, kColorSand)
  K(polished_andesite, kColorStoneAndesite)
  C(carved_pumpkin, 213, 125, 50)
  C(stripped_oak_wood, 127, 85, 48)
  C(stonecutter, 131, 131, 131)
  C(smoker, 131, 131, 131)
  C(hay_block, 203, 176, 7)
  C(birch_log, 252, 252, 252)
  C(iron_trapdoor, 227, 227, 227)
  C(bell, 250, 211, 56)
  C(white_glazed_terracotta, 246, 252, 251)
  C(orange_glazed_terracotta, 26, 196, 197)
  C(magenta_glazed_terracotta, 201, 87, 192)
  C(light_blue_glazed_terracotta, 86, 187, 220)
  C(yellow_glazed_terracotta, 251, 219, 93)
  C(lime_glazed_terracotta, 137, 214, 35)
  C(pink_glazed_terracotta, 241, 179, 201)
  C(gray_glazed_terracotta, 94, 114, 118)
  C(light_gray_glazed_terracotta, 199, 203, 207)
  C(cyan_glazed_terracotta, 20, 159, 160)
  C(purple_glazed_terracotta, 146, 53, 198)
  C(blue_glazed_terracotta, 59, 66, 167)
  C(brown_glazed_terracotta, 167, 120, 79)
  C(green_glazed_terracotta, 111, 151, 36)
  K(furnace, kColorFurnace)
  C(composter, 139, 91, 49)
  C(campfire, 199, 107, 3)
  C(cartography_table, 86, 53, 24)
  C(brewing_stand, 47, 47, 47)
  C(grindstone, 141, 141, 141)
  C(fletching_table, 212, 191, 131)
  C(iron_bars, 154, 154, 154)
  K(bookshelf, kColorBookshelf)
  C(acacia_sapling, 125, 150, 33)
  K(potted_dead_bush, kColorPotter)
  K(potted_cactus, kColorPotter)
  C(jack_o_lantern, 213, 125, 50)
  K(acacia_button, kColorPlanksAcacia)
  K(acacia_sign, kColorPlanksAcacia)
  K(acacia_trapdoor, kColorPlanksAcacia)
  K(acacia_wall_sign, kColorPlanksAcacia)
  K(andesite_slab, kColorStoneAndesite)
  K(andesite_stairs, kColorStoneAndesite)
  K(andesite_wall, kColorStoneAndesite)
  K(anvil, kColorAnvil)
  C(attached_melon_stem, 203, 196, 187)
  C(bamboo_sapling, 67, 103, 8)
  C(barrel, 137, 102, 60)
  K(birch_door, kColorPlanksBirch)
  K(birch_fence_gate, kColorPlanksBirch)
  K(birch_pressure_plate, kColorPlanksBirch)
  C(birch_sapling, 107, 156, 55)
  K(birch_sign, kColorPlanksBirch)
  K(birch_trapdoor, kColorPlanksBirch)
  K(birch_wall_sign, kColorPlanksBirch)
  C(birch_wood, 252, 252, 252)
  C(black_concrete, 8, 10, 15)
  C(black_concrete_powder, 22, 24, 29)
  C(black_glazed_terracotta, 24, 24, 27)
  C(blast_furnace, 131, 131, 131)
  C(coal_block, 13, 13, 13)
  C(diamond_block, 100, 242, 224)
  C(emerald_block, 62, 240, 130)
  C(gold_block, 251, 221, 72)
  C(iron_block, 227, 227, 227)
  C(iron_door, 227, 227, 227)
  K(potted_acacia_sapling, kColorPotter)
  K(potted_allium, kColorPotter)
  K(potted_azure_bluet, kColorPotter)
  K(potted_bamboo, kColorPotter)
  K(potted_birch_sapling, kColorPotter)
  K(potted_blue_orchid, kColorPotter)
  K(potted_brown_mushroom, kColorPotter)
  K(potted_cornflower, kColorPotter)
  K(potted_dandelion, kColorPotter)
  K(potted_dark_oak_sapling, kColorPotter)
  K(potted_fern, kColorPotter)
  K(potted_jungle_sapling, kColorPotter)
  K(potted_lily_of_the_valley, kColorPotter)
  K(potted_oak_sapling, kColorPotter)
  K(potted_orange_tulip, kColorPotter)
  K(potted_oxeye_daisy, kColorPotter)
  K(potted_pink_tulip, kColorPotter)
  K(potted_poppy, kColorPotter)
  K(potted_red_mushroom, kColorPotter)
  K(potted_red_tulip, kColorPotter)
  K(potted_spruce_sapling, kColorPotter)
  K(potted_white_tulip, kColorPotter)
  K(potted_wither_rose, kColorPotter)
  K(dark_oak_button, kColorPlanksDarkOak)
  K(dark_oak_pressure_plate, kColorPlanksDarkOak)
  K(dark_oak_sign, kColorPlanksDarkOak)
  K(dark_oak_wall_sign, kColorPlanksDarkOak)
  K(oak_button, kColorPlanksOak)
  K(oak_sign, kColorPlanksOak)
  K(oak_wall_sign, kColorPlanksOak)
  K(brick_slab, kColorBricks)
  K(brick_stairs, kColorBricks)
  K(brick_wall, kColorBricks)
  K(chipped_anvil, kColorAnvil)
  K(damaged_anvil, kColorAnvil)
  C(daylight_detector, 188, 168, 140)
  K(dead_brain_coral, kColorDeadCoral)
  K(dead_brain_coral_block, kColorDeadCoral)
  K(dead_brain_coral_fan, kColorDeadCoral)
  K(dead_brain_coral_wall_fan, kColorDeadCoral)
  K(dead_bubble_coral, kColorDeadCoral)
  K(dead_bubble_coral_block, kColorDeadCoral)
  K(dead_bubble_coral_fan, kColorDeadCoral)
  K(dead_bubble_coral_wall_fan, kColorDeadCoral)
  K(dead_fire_coral, kColorDeadCoral)
  K(dead_fire_coral_block, kColorDeadCoral)
  K(dead_fire_coral_fan, kColorDeadCoral)
  K(dead_fire_coral_wall_fan, kColorDeadCoral)
  K(dead_horn_coral, kColorDeadCoral)
  K(dead_horn_coral_block, kColorDeadCoral)
  K(dead_horn_coral_fan, kColorDeadCoral)
  K(dead_horn_coral_wall_fan, kColorDeadCoral)
  K(dead_tube_coral, kColorDeadCoral)
  K(dead_tube_coral_block, kColorDeadCoral)
  K(dead_tube_coral_fan, kColorDeadCoral)
  K(dead_tube_coral_wall_fan, kColorDeadCoral)
  K(detector_rail, kColorRail)
  K(powered_rail, kColorRail)
  K(diorite_slab, kColorStoneDiorite)
  K(diorite_stairs, kColorStoneDiorite)
  K(diorite_wall, kColorStoneDiorite)
  K(polished_diorite_slab, kColorStoneDiorite)
  K(polished_diorite_stairs, kColorStoneDiorite)
  K(granite_slab, kColorStoneGranite)
  K(granite_stairs, kColorStoneGranite)
  K(granite_wall, kColorStoneGranite)
  K(polished_granite_slab, kColorStoneGranite)
  K(polished_granite_stairs, kColorStoneGranite)
  K(jungle_fence_gate, kColorPlanksJungle)
  K(jungle_pressure_plate, kColorPlanksJungle)
  K(jungle_sign, kColorPlanksJungle)
  K(jungle_wall_sign, kColorPlanksJungle)
  K(nether_brick_fence, kColorNetherBricks)
  K(nether_brick_stairs, kColorNetherBricks)
  K(stone_button, kColorStone)
  K(stone_pressure_plate, kColorStone)
  K(stone_stairs, kColorStone)
  K(spruce_button, kColorPlanksSpruce)
  K(spruce_fence_gate, kColorPlanksSpruce)
  K(spruce_pressure_plate, kColorPlanksSpruce)
  K(spruce_sign, kColorPlanksSpruce)
  K(spruce_wall_sign, kColorPlanksSpruce)
  K(dispenser, kColorFurnace)
  K(dropper, kColorFurnace)
  K(quartz_block, kColorQuartz)
  C(blue_concrete_powder, 72, 75, 175)
  C(brown_concrete_powder, 120, 81, 50)
  C(cyan_concrete_powder, 37, 154, 160)
  C(gray_concrete_powder, 75, 79, 82)
  C(green_concrete_powder, 103, 126, 37)
  C(light_blue_concrete_powder, 91, 194, 216)
  C(light_gray_concrete_powder, 154, 154, 148)
  C(lime_concrete_powder, 138, 197, 45)
  C(magenta_concrete_powder, 200, 93, 193)
  C(orange_concrete_powder, 230, 128, 20)
  C(pink_concrete_powder, 236, 172, 195)
  C(purple_concrete_powder, 138, 58, 186)
  C(red_concrete_powder, 180, 58, 55)
  C(white_concrete_powder, 222, 223, 224)
  C(yellow_concrete_powder, 235, 209, 64)
  K(end_stone_brick_slab, kColorEndStoneBricks)
  K(end_stone_brick_stairs, kColorEndStoneBricks)
  K(end_stone_brick_wall, kColorEndStoneBricks)
  K(end_stone_bricks, kColorEndStoneBricks)
  C(blue_concrete, 44, 46, 142)
  C(bone_block, 199, 195, 165)
  C(brown_concrete, 95, 58, 31)
  C(cake, 238, 229, 203)
  C(candle_cake, 238, 229, 203)
  C(white_candle_cake, 238, 229, 203)
  C(orange_candle_cake, 238, 229, 203)
  C(magenta_candle_cake, 238, 229, 203)
  C(light_blue_candle_cake, 238, 229, 203)
  C(yellow_candle_cake, 238, 229, 203)
  C(lime_candle_cake, 238, 229, 203)
  C(pink_candle_cake, 238, 229, 203)
  C(gray_candle_cake, 238, 229, 203)
  C(light_gray_candle_cake, 238, 229, 203)
  C(cyan_candle_cake, 238, 229, 203)
  C(purple_candle_cake, 238, 229, 203)
  C(blue_candle_cake, 238, 229, 203)
  C(brown_candle_cake, 238, 229, 203)
  C(green_candle_cake, 238, 229, 203)
  C(red_candle_cake, 238, 229, 203)
  C(black_candle_cake, 238, 229, 203)
  C(chain_command_block, 159, 193, 178)
  K(chiseled_quartz_block, kColorQuartz)
  K(chiseled_red_sandstone, kColorRedSandstone)
  C(command_block, 196, 125, 78)
  C(conduit, 126, 113, 81)
  K(cut_red_sandstone, kColorRedSandstone)
  K(cut_red_sandstone_slab, kColorRedSandstone)
  K(red_sandstone, kColorRedSandstone)
  K(red_sandstone_slab, kColorRedSandstone)
  K(red_sandstone_stairs, kColorRedSandstone)
  K(red_sandstone_wall, kColorRedSandstone)
  K(smooth_red_sandstone, kColorRedSandstone)
  K(smooth_red_sandstone_slab, kColorRedSandstone)
  K(smooth_red_sandstone_stairs, kColorRedSandstone)
  K(cut_sandstone_slab, kColorSand)
  C(cyan_concrete, 21, 118, 134)
  C(dark_oak_sapling, 31, 100, 25)
  C(dark_oak_wood, 62, 48, 29)
  C(dragon_egg, 9, 9, 9)
  K(dragon_head, kColorDragonHead)
  K(dragon_wall_head, kColorDragonHead)
  K(quartz_pillar, kColorQuartz)
  K(quartz_slab, kColorQuartz)
  K(quartz_stairs, kColorQuartz)
  K(emerald_ore, kColorStone)
  K(polished_andesite_slab, kColorStoneAndesite)
  K(polished_andesite_stairs, kColorStoneAndesite)
  K(mossy_cobblestone_slab, kColorMossyStone)
  K(mossy_cobblestone_stairs, kColorMossyStone)
  K(mossy_cobblestone_wall, kColorMossyStone)
  K(infested_cobblestone, kColorStone)
  K(infested_mossy_stone_bricks, kColorMossyStone)
  K(mossy_stone_brick_slab, kColorMossyStone)
  K(mossy_stone_brick_stairs, kColorMossyStone)
  K(mossy_stone_brick_wall, kColorMossyStone)
  K(infested_chiseled_stone_bricks, kColorStone)
  K(infested_cracked_stone_bricks, kColorStone)
  K(infested_stone_bricks, kColorStone)
  K(moving_piston, kColorPistonHead)
  K(smooth_quartz, kColorQuartz)
  K(smooth_quartz_slab, kColorQuartz)
  K(smooth_quartz_stairs, kColorQuartz)
  K(stone_brick_slab, kColorStone)
  K(stone_brick_wall, kColorStone)
  K(purpur_stairs, kColorPurPur)
  K(prismarine_wall, kColorPrismarine)
  K(red_nether_brick_stairs, kColorRedNetherBricks)
  K(creeper_head, kColorCreaperHead)
  K(creeper_wall_head, kColorCreaperHead)
  C(enchanting_table, 73, 234, 207)
  C(end_gateway, 3, 13, 20)
  C(gray_concrete, 53, 57, 61)
  C(green_concrete, 72, 90, 35)
  C(heavy_weighted_pressure_plate, 182, 182, 182)
  C(jigsaw, 147, 120, 148)
  C(jukebox, 122, 79, 56)
  C(jungle_sapling, 41, 73, 12)
  C(jungle_wood, 88, 69, 26)
  C(lantern, 72, 79, 100)
  C(lapis_block, 24, 59, 115)
  C(light_blue_concrete, 37, 136, 198)
  C(light_gray_concrete, 125, 125, 115)
  C(light_weighted_pressure_plate, 202, 171, 50)
  C(lime_concrete, 93, 167, 24)
  C(loom, 200, 164, 112)
  C(magenta_concrete, 168, 49, 158)
  C(nether_wart_block, 122, 1, 0)
  C(note_block, 146, 92, 64)
  C(oak_sapling, 63, 141, 46)
  K(oak_wood, kColorLogOak)
  C(orange_concrete, 222, 97, 0)
  K(petrified_oak_slab, kColorPlanksOak)
  C(pink_concrete, 210, 100, 141)
  K(player_head, kColorPlayerHead)
  K(player_wall_head, kColorPlayerHead)
  C(purple_concrete, 99, 32, 154)
  C(red_concrete, 138, 32, 32)
  C(red_glazed_terracotta, 202, 65, 57)
  C(comparator, 185, 185, 185)
  C(repeater, 185, 185, 185)
  C(repeating_command_block, 105, 78, 197)
  C(scaffolding, 225, 196, 115)
  K(skeleton_skull, kColorSkeltonSkull)
  K(skeleton_wall_skull, kColorSkeltonSkull)
  C(smithing_table, 63, 65, 82)
  C(spawner, 24, 43, 56)
  C(sponge, 203, 204, 73)
  C(spruce_sapling, 34, 52, 34)
  K(spruce_wood, kColorLogSpruce)
  C(stripped_acacia_wood, 185, 94, 61)
  C(stripped_birch_log, 205, 186, 126)
  C(stripped_birch_wood, 205, 186, 126)
  C(stripped_dark_oak_log, 107, 83, 51)
  C(stripped_dark_oak_wood, 107, 83, 51)
  C(stripped_jungle_log, 173, 126, 82)
  C(stripped_jungle_wood, 173, 126, 82)
  C(stripped_oak_log, 148, 115, 64)
  C(stripped_spruce_log, 120, 90, 54)
  C(stripped_spruce_wood, 120, 90, 54)
  C(structure_block, 147, 120, 148)
  K(trapped_chest, kColorChest)
  C(tripwire_hook, 135, 135, 135)
  C(turtle_egg, 224, 219, 197)
  C(wet_sponge, 174, 189, 74)
  C(white_concrete, 204, 209, 210)
  C(wither_rose, 23, 18, 16)
  K(wither_skeleton_skull, kColorWitherSkeltonSkull)
  K(wither_skeleton_wall_skull, kColorWitherSkeltonSkull)
  C(yellow_concrete, 239, 175, 22)
  K(zombie_head, kColorZombieHead)
  K(zombie_wall_head, kColorZombieHead)
  C(end_rod, 202, 202, 202)
  K(flower_pot, kColorPotter)
  C(frosted_ice, 109, 146, 193)
  C(nether_portal, 78, 30, 135)

  // plants
  C(lily_pad, 0, 123, 0)
  C(wheat, 0, 123, 0)
  C(melon, 125, 202, 25)
  C(pumpkin, 213, 125, 50)
  C(grass, 109, 141, 35)
  C(tall_grass, 109, 141, 35)
  C(dandelion, 245, 238, 50)
  C(poppy, 229, 31, 29)
  C(peony, 232, 143, 213)
  C(pink_tulip, 234, 182, 209)
  C(orange_tulip, 242, 118, 33)
  C(lilac, 212, 119, 197)
  C(sunflower, 245, 238, 50)
  C(allium, 200, 109, 241)
  C(red_tulip, 229, 31, 29)
  C(white_tulip, 255, 255, 255)
  C(rose_bush, 136, 40, 27)
  C(blue_orchid, 47, 181, 199)
  C(oxeye_daisy, 236, 246, 247)
  C(sugar_cane, 165, 214, 90)
  C(chorus_plant, 90, 51, 90)
  C(chorus_flower, 159, 119, 159)
  C(dark_oak_leaves, 58, 82, 23)
  C(red_mushroom_block, 199, 42, 41)
  C(mushroom_stem, 203, 196, 187)
  C(brown_mushroom_block, 149, 113, 80)
  C(acacia_leaves, 63, 89, 25)
  C(dead_bush, 146, 99, 40)
  C(cactus, 90, 138, 42)
  C(sweet_berry_bush, 40, 97, 63)
  C(cornflower, 69, 105, 232)
  C(pumpkin_stem, 72, 65, 9)
  C(nether_wart, 163, 35, 41)
  C(attached_pumpkin_stem, 72, 65, 9)
  C(lily_of_the_valley, 252, 252, 252)
  C(melon_stem, 72, 65, 9)
  C(smooth_stone, 111, 111, 111)
  C(smooth_stone_slab, 111, 111, 111)
  C(bamboo, 67, 103, 8)
  C(sea_pickle, 106, 113, 42)
  C(cocoa, 109, 112, 52)

  // 1.15

  C(bee_nest, 198, 132, 67)
  C(beehive, 182, 146, 94)
  C(honey_block, 233, 145, 38)
  C(honeycomb_block, 229, 138, 8)

  // 1.16

  C(crimson_nylium, 146, 24, 24)
  C(warped_nylium, 22, 125, 132)
  K(crimson_planks, kColorPlanksCrimson)
  K(warped_planks, kColorPlanksWarped)
  C(nether_gold_ore, 245, 173, 42)
  K(crimson_stem, kColorPlanksCrimson)
  K(warped_stem, kColorPlanksWarped)
  C(stripped_crimson_stem, 148, 61, 97)
  C(stripped_warped_stem, 67, 159, 157)
  C(crimson_hyphae, 148, 21, 21)
  C(warped_hyphae, 22, 96, 90)
  K(crimson_slab, kColorPlanksCrimson)
  K(warped_slab, kColorPlanksWarped)
  K(cracked_nether_bricks, kColorNetherBricks)
  K(chiseled_nether_bricks, kColorNetherBricks)
  K(crimson_stairs, kColorPlanksCrimson)
  K(warped_stairs, kColorPlanksWarped)
  C(netherite_block, 76, 72, 76)
  C(soul_soil, 90, 68, 55)
  C(basalt, 91, 91, 91)
  C(polished_basalt, 115, 115, 115)
  C(smooth_basalt, 91, 91, 91)
  C(ancient_debris, 125, 95, 88)
  C(crying_obsidian, 42, 1, 119)
  K(blackstone, kColorStoneBlack)
  K(blackstone_slab, kColorStoneBlack)
  K(blackstone_stairs, kColorStoneBlack)
  C(gilded_blackstone, 125, 68, 14)
  K(polished_blackstone, kColorStonePolishedBlack)
  K(polished_blackstone_slab, kColorStonePolishedBlack)
  K(polished_blackstone_stairs, kColorStonePolishedBlack)
  K(chiseled_polished_blackstone, kColorStonePolishedBlack)
  K(polished_blackstone_bricks, kColorPolishedBlackStoneBricks)
  K(polished_blackstone_brick_slab, kColorPolishedBlackStoneBricks)
  K(polished_blackstone_brick_stairs, kColorPolishedBlackStoneBricks)
  K(cracked_polished_blackstone_bricks, kColorPolishedBlackStoneBricks)
  C(crimson_fungus, 162, 36, 40)
  C(warped_fungus, 20, 178, 131)
  C(crimson_roots, 171, 16, 28)
  C(warped_roots, 20, 178, 131)
  C(nether_sprouts, 20, 178, 131)
  C(weeping_vines, 171, 16, 28)
  C(weeping_vines_plant, 171, 16, 28)
  C(twisting_vines, 20, 178, 131)
  K(crimson_fence, kColorPlanksCrimson)
  K(warped_fence, kColorPlanksWarped)
  C(soul_torch, 123, 239, 242)
  C(chain, 60, 65, 80)
  K(blackstone_wall, kColorStoneBlack)
  K(polished_blackstone_wall, kColorStonePolishedBlack)
  K(polished_blackstone_brick_wall, kColorPolishedBlackStoneBricks)
  C(soul_lantern, 123, 239, 242)
  C(soul_campfire, 123, 239, 242)
  C(soul_fire, 123, 239, 242)
  C(soul_wall_torch, 123, 239, 242)
  C(shroomlight, 251, 170, 108)
  C(lodestone, 160, 162, 170)
  C(respawn_anchor, 129, 8, 225)
  K(crimson_pressure_plate, kColorPlanksCrimson)
  K(warped_pressure_plate, kColorPlanksWarped)
  K(crimson_trapdoor, kColorPlanksCrimson)
  K(warped_trapdoor, kColorPlanksWarped)
  K(crimson_fence_gate, kColorPlanksCrimson)
  K(warped_fence_gate, kColorPlanksWarped)
  K(crimson_button, kColorPlanksCrimson)
  K(warped_button, kColorPlanksWarped)
  K(crimson_door, kColorPlanksCrimson)
  K(warped_door, kColorPlanksWarped)
  C(target, 183, 49, 49)

  // bugfix for 1.16
  C(twisting_vines_plant, 17, 153, 131)
  C(warped_wart_block, 17, 153, 131)
  K(quartz_bricks, kColorQuartz)
  C(stripped_crimson_hyphae, 148, 61, 97)
  C(stripped_warped_hyphae, 67, 159, 157)
  K(crimson_sign, kColorPlanksCrimson)
  K(warped_sign, kColorPlanksWarped)
  K(polished_blackstone_pressure_plate, kColorStonePolishedBlack)
  K(polished_blackstone_button, kColorStonePolishedBlack)

  K(crimson_wall_sign, kColorPlanksCrimson)
  K(warped_wall_sign, kColorPlanksWarped)

  K(deepslate, kColorDeepslate)
  K(cobbled_deepslate, kColorDeepslate)
  K(polished_deepslate, kColorDeepslate)
  K(deepslate_coal_ore, kColorDeepslate)
  K(deepslate_iron_ore, kColorDeepslate)
  K(deepslate_copper_ore, kColorDeepslate)
  K(deepslate_gold_ore, kColorDeepslate)
  K(deepslate_redstone_ore, kColorDeepslate)
  K(deepslate_emerald_ore, kColorDeepslate)
  K(deepslate_lapis_ore, kColorDeepslate)
  K(deepslate_diamond_ore, kColorDeepslate)
  K(infested_deepslate, kColorDeepslate)
  K(deepslate_bricks, kColorDeepslate)
  K(cracked_deepslate_bricks, kColorDeepslate)
  K(deepslate_tiles, kColorDeepslate)
  K(cracked_deepslate_tiles, kColorDeepslate)
  K(chiseled_deepslate, kColorDeepslate)
  K(cobbled_deepslate_stairs, kColorDeepslate)
  K(polished_deepslate_stairs, kColorDeepslate)
  K(deepslate_brick_stairs, kColorDeepslate)
  K(deepslate_tile_stairs, kColorDeepslate)
  K(cobbled_deepslate_slab, kColorDeepslate)
  K(polished_deepslate_slab, kColorDeepslate)
  K(deepslate_brick_slab, kColorDeepslate)
  K(deepslate_tile_slab, kColorDeepslate)
  K(cobbled_deepslate_wall, kColorDeepslate)
  K(polished_deepslate_wall, kColorDeepslate)
  K(deepslate_brick_wall, kColorDeepslate)
  K(deepslate_tile_wall, kColorDeepslate)

  K(copper_ore, kColorStone)

  K(calcite, kColorStoneDiorite)
  K(tuff, kColorStone)
  C(dripstone_block, 140, 116, 97)
  C(pointed_dripstone, 140, 116, 97)
  C(raw_iron_block, 109, 89, 64)
  C(raw_copper_block, 145, 83, 62)
  C(raw_gold_block, 173, 137, 34)
  C(amethyst_block, 145, 104, 174)
  C(budding_amethyst, 145, 104, 174)

  K(copper_block, kColorCopper)
  K(cut_copper, kColorCopper)
  K(waxed_copper_block, kColorCopper)
  K(waxed_cut_copper, kColorCopper)
  K(cut_copper_stairs, kColorCopper)
  K(waxed_cut_copper_stairs, kColorCopper)
  K(cut_copper_slab, kColorCopper)
  K(waxed_cut_copper_slab, kColorCopper)

  K(exposed_copper, kColorExposedCopper)
  K(exposed_cut_copper, kColorExposedCopper)
  K(waxed_exposed_copper, kColorExposedCopper)
  K(waxed_exposed_cut_copper, kColorExposedCopper)
  K(exposed_cut_copper_stairs, kColorExposedCopper)
  K(waxed_exposed_cut_copper_stairs, kColorExposedCopper)
  K(exposed_cut_copper_slab, kColorExposedCopper)
  K(waxed_exposed_cut_copper_slab, kColorExposedCopper)

  K(weathered_copper, kColorWeatheredCopper)
  K(weathered_cut_copper, kColorWeatheredCopper)
  K(waxed_weathered_copper, kColorWeatheredCopper)
  K(waxed_weathered_cut_copper, kColorWeatheredCopper)
  K(weathered_cut_copper_stairs, kColorWeatheredCopper)
  K(waxed_weathered_cut_copper_stairs, kColorWeatheredCopper)
  K(weathered_cut_copper_slab, kColorWeatheredCopper)
  K(waxed_weathered_cut_copper_slab, kColorWeatheredCopper)

  K(oxidized_copper, kColorOxidizedCopper)
  K(oxidized_cut_copper, kColorOxidizedCopper)
  K(waxed_oxidized_copper, kColorOxidizedCopper)
  K(waxed_oxidized_cut_copper, kColorOxidizedCopper)
  K(oxidized_cut_copper_stairs, kColorOxidizedCopper)
  K(waxed_oxidized_cut_copper_stairs, kColorOxidizedCopper)
  K(oxidized_cut_copper_slab, kColorOxidizedCopper)
  K(waxed_oxidized_cut_copper_slab, kColorOxidizedCopper)

  C(azalea_leaves, 111, 144, 44)
  C(azalea, 111, 144, 44)
  C(flowering_azalea, 184, 97, 204)
  C(spore_blossom, 184, 97, 204)
  C(moss_carpet, 111, 144, 44)
  C(moss_block, 111, 144, 44)

  K(potted_azalea_bush, kColorPotter)
  K(potted_flowering_azalea_bush, kColorPotter)
  C(cauldron, 53, 52, 52)
  C(water_cauldron, 53, 52, 52)
  C(lava_cauldron, 53, 52, 52)
  C(powder_snow_cauldron, 53, 52, 52)
  C(rooted_dirt, 149, 108, 76)
  C(flowering_azalea_leaves, 184, 97, 204)
  C(small_amethyst_bud, 145, 104, 174)
  C(medium_amethyst_bud, 145, 104, 174)
  C(large_amethyst_bud, 145, 104, 174)
  C(amethyst_cluster, 145, 104, 174)
  C(cave_vines, 106, 126, 48)
  C(cave_vines_plant, 106, 126, 48)
  K(potted_crimson_fungus, kColorPotter)
  K(potted_warped_fungus, kColorPotter)
  K(potted_crimson_roots, kColorPotter)
  K(potted_warped_roots, kColorPotter)
  K(sculk_sensor, kColorSculkSensor)
  K(reinforced_deepslate, kColorDeepslate)
  K(sculk, kColorSculk)
  K(sculk_catalyst, kColorSculk)
  K(sculk_shrieker, kColorSculk)
  K(mangrove_planks, kColorPlanksManvrove)
  C(mangrove_roots, 89, 71, 43)
  C(muddy_mangrove_roots, 57, 55, 60)
  C(mangrove_log, 89, 71, 43)
  K(stripped_mangrove_log, kColorPlanksManvrove)
  K(stripped_mangrove_wood, kColorPlanksManvrove)
  C(mangrove_wood, 89, 71, 43)
  K(mangrove_slab, kColorPlanksManvrove)
  K(mud_brick_slab, kColorMudBrick)
  K(packed_mud, kColorMudBrick)
  K(mud_bricks, kColorMudBrick)
  K(mangrove_fence, kColorPlanksManvrove)
  K(mangrove_stairs, kColorPlanksManvrove)
  K(mangrove_fence_gate, kColorPlanksManvrove)
  K(mud_brick_wall, kColorMudBrick)
  K(mangrove_sign, kColorPlanksManvrove)
  K(mangrove_wall_sign, kColorPlanksManvrove)
  C(ochre_froglight, 252, 249, 242)
  C(verdant_froglight, 252, 249, 242)
  C(pearlescent_froglight, 252, 249, 242)
  C(mangrove_leaves, 59, 73, 16)
  K(mangrove_button, kColorPlanksManvrove)
  K(mangrove_pressure_plate, kColorPlanksManvrove)
  K(mangrove_door, kColorPlanksManvrove)
  K(mangrove_trapdoor, kColorPlanksManvrove)
  K(potted_mangrove_propagule, kColorPotter)
  C(mud, 57, 55, 60)

  // 1.20 and bugfix
  K(mud_brick_stairs, kColorMudBrick)
  K(acacia_hanging_sign, kColorPlanksAcacia)
  K(acacia_wall_hanging_sign, kColorPlanksAcacia)
  C(bamboo_block, 127, 132, 56)
  K(bamboo_button, kColorPlanksBamboo)
  K(bamboo_door, kColorPlanksBamboo)
  K(bamboo_fence, kColorPlanksBamboo)
  K(bamboo_fence_gate, kColorPlanksBamboo)
  K(bamboo_hanging_sign, kColorPlanksBamboo)
  K(bamboo_mosaic, kColorPlanksBamboo)
  K(bamboo_mosaic_slab, kColorPlanksBamboo)
  K(bamboo_mosaic_stairs, kColorPlanksBamboo)
  K(bamboo_planks, kColorPlanksBamboo)
  K(bamboo_pressure_plate, kColorPlanksBamboo)
  K(bamboo_sign, kColorPlanksBamboo)
  K(bamboo_slab, kColorPlanksBamboo)
  K(bamboo_stairs, kColorPlanksBamboo)
  K(bamboo_trapdoor, kColorPlanksBamboo)
  K(bamboo_wall_hanging_sign, kColorPlanksBamboo)
  K(bamboo_wall_sign, kColorPlanksBamboo)
  K(birch_hanging_sign, kColorPlanksBirch)
  K(birch_wall_hanging_sign, kColorPlanksBirch)
  K(chiseled_bookshelf, kColorBookshelf)
  K(crimson_hanging_sign, kColorPlanksCrimson)
  K(crimson_wall_hanging_sign, kColorPlanksCrimson)
  K(dark_oak_hanging_sign, kColorPlanksDarkOak)
  K(dark_oak_wall_hanging_sign, kColorPlanksDarkOak)
  K(jungle_hanging_sign, kColorPlanksJungle)
  K(jungle_wall_hanging_sign, kColorPlanksJungle)
  K(mangrove_hanging_sign, kColorPlanksManvrove)
  K(mangrove_wall_hanging_sign, kColorPlanksManvrove)
  K(oak_hanging_sign, kColorPlanksOak)
  K(oak_wall_hanging_sign, kColorPlanksOak)
  K(piglin_head, kColorPiglinHead)
  K(piglin_wall_head, kColorPiglinHead)
  K(spruce_hanging_sign, kColorPlanksSpruce)
  K(spruce_wall_hanging_sign, kColorPlanksSpruce)
  K(stripped_bamboo_block, kColorPlanksBamboo)
  K(warped_hanging_sign, kColorPlanksWarped)
  K(warped_wall_hanging_sign, kColorPlanksWarped)
  K(cherry_button, kColorPlanksCherry)
  K(cherry_door, kColorPlanksCherry)
  K(cherry_fence, kColorPlanksCherry)
  K(cherry_fence_gate, kColorPlanksCherry)
  K(cherry_hanging_sign, kColorPlanksCherry)
  K(cherry_leaves, kColorLeavesCherry)
  K(cherry_log, kColorLogCherry)
  K(cherry_planks, kColorPlanksCherry)
  K(cherry_pressure_plate, kColorPlanksCherry)
  K(cherry_sapling, kColorLeavesCherry)
  K(cherry_sign, kColorPlanksCherry)
  K(cherry_slab, kColorPlanksCherry)
  K(cherry_stairs, kColorPlanksCherry)
  K(cherry_trapdoor, kColorPlanksCherry)
  K(cherry_wall_hanging_sign, kColorPlanksCherry)
  K(cherry_wall_sign, kColorPlanksCherry)
  K(cherry_wood, kColorLogCherry)
  K(decorated_pot, kColorPotter)
  K(pink_petals, kColorLeavesCherry)
  K(potted_cherry_sapling, kColorPotter)
  K(potted_torchflower, kColorPotter)
  K(stripped_cherry_log, kColorLogCherryStripped)
  K(stripped_cherry_wood, kColorLogCherryStripped)
  K(suspicious_sand, kColorSand)
  K(torchflower, kColorTorchflower)
  K(torchflower_crop, kColorTorchflower)
  K(calibrated_sculk_sensor, kColorSculkSensor)
  K(pitcher_crop, kColorPitcher)
  K(pitcher_plant, kColorPitcher)
  C(sniffer_egg, 186, 77, 57)
  K(suspicious_gravel, kColorGravel)
#undef C
#undef K
  return ret;
}

std::unordered_map<mcfile::blocks::BlockId, juce::Colour> const &Table() {
  static std::unique_ptr<std::unordered_map<mcfile::blocks::BlockId, juce::Colour> const> const sTable(CreateTable());
  return *sTable;
}
} // namespace

std::optional<juce::Colour> Palette::ColorFromId(mcfile::blocks::BlockId id) {
  auto const &table = Table();
  if (auto found = table.find(id); found != table.end()) {
    return found->second;
  } else {
    return std::nullopt;
  }
}

} // namespace mcview
