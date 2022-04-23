#include "RegionToTexture.h"
#include "File.h"
#include <cassert>
#include "minecraft-file.hpp"
#include <colormap/colormap.h>
#include <map>
#include <set>

using namespace juce;

static Colour const kColorPotter(135, 75, 58);
static Colour const kColorPlanksBirch(244, 230, 161);
static Colour const kColorPlanksDarkOak(101, 75, 50);
static Colour const kColorPlanksOak(127, 85, 48);
static Colour const kColorPlanksJungle(149, 108, 76);
static Colour const kColorPlanksSpruce(122, 89, 51);
static Colour const kColorPlanksCrimson(125, 57, 85);
static Colour const kColorPlanksWarped(56, 129, 128);
static Colour const kColorPlanksManvrove(137, 76, 57);
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
static Colour const kColorSandstone(244, 230, 161);
static Colour const kColorDragonHead(22, 22, 22);
static Colour const kColorQuartz(235, 227, 219);
static Colour const kColorMossyStone(115, 131, 82);
static Colour const kColorPistonHead(186, 150, 97);
static Colour const kColorPurPur(170, 122, 170);
static Colour const kColorPrismarine(75, 125, 151);
static Colour const kColorRedNetherBricks(89, 0, 0);
static Colour const kColorCreaperHead(96, 202, 77);
static Colour const kColorOakLog(141, 118, 71);
static Colour const kColorPlayerHead(46, 31, 14);
static Colour const kColorSkeltonSkull(186, 186, 186);
static Colour const kColorSpruceLog(141, 118, 71);
static Colour const kColorChest(141, 118, 71);
static Colour const kColorWitherSkeltonSkull(31, 31, 31);
static Colour const kColorZombieHead(61, 104, 45);
static Colour const kColorDeepslate(104, 104, 104);
static Colour const kColorCopper(224, 128, 107);
static Colour const kColorExposedCopper(150, 138, 104);
static Colour const kColorWeatheredCopper(99, 158, 118);
static Colour const kColorOxidizedCopper(75, 146, 130);

std::map<mcfile::blocks::BlockId, Colour> const RegionToTexture::kBlockToColor {
    {mcfile::blocks::minecraft::stone, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::granite, kColorStoneGranite},
    {mcfile::blocks::minecraft::diorite, kColorStoneDiorite},
    {mcfile::blocks::minecraft::andesite, kColorStoneAndesite},
    {mcfile::blocks::minecraft::chest, kColorChest},
    {mcfile::blocks::minecraft::clay, Colour(162, 166, 182)},
    {mcfile::blocks::minecraft::coal_ore, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::cobblestone, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::dirt, Colour(149, 108, 76)},
    {mcfile::blocks::minecraft::brown_mushroom, Colour(0, 123, 0)},
    {mcfile::blocks::minecraft::grass_block, Colour(130, 148, 58)},
    {mcfile::blocks::minecraft::iron_ore, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::sand, Colour(201,192,154)}, //
    {mcfile::blocks::minecraft::oak_leaves, Colour(56, 95, 31)}, //
    {mcfile::blocks::minecraft::jungle_leaves, Colour(56, 95, 31)}, //
    {mcfile::blocks::minecraft::birch_leaves, Colour(67, 124, 37)},
    {mcfile::blocks::minecraft::red_mushroom, Colour(0, 123, 0)},
    {mcfile::blocks::minecraft::mossy_cobblestone, kColorMossyStone},
    {mcfile::blocks::minecraft::oak_stairs, kColorPlanksOak},
    {mcfile::blocks::minecraft::gravel, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::oak_log, kColorOakLog},
    {mcfile::blocks::minecraft::oak_planks, kColorPlanksOak},
    {mcfile::blocks::minecraft::farmland, Colour(149, 108, 76)},
    {mcfile::blocks::minecraft::oak_fence, kColorPlanksOak},
    {mcfile::blocks::minecraft::cobblestone_stairs, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::dirt_path, Colour(204, 204, 204)}, //
    {mcfile::blocks::minecraft::birch_fence, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_planks, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_stairs, kColorPlanksBirch},
    {mcfile::blocks::minecraft::dark_oak_fence, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::dark_oak_log, Colour(101, 75, 50)},
    {mcfile::blocks::minecraft::dark_oak_planks, Colour(191,152,63)}, //
    {mcfile::blocks::minecraft::dark_oak_slab, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::dark_oak_stairs, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::dark_oak_trapdoor, Colour(141, 118, 71)},
    {mcfile::blocks::minecraft::diamond_ore, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::gold_ore, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::ice, Colour(158, 158, 252)},
    {mcfile::blocks::minecraft::jungle_fence, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_log, Colour(149, 108, 76)},
    {mcfile::blocks::minecraft::jungle_planks, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_slab, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_stairs, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_button, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_door, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_trapdoor, Colour(141, 118, 71)},
    {mcfile::blocks::minecraft::lapis_ore, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::lava, Colour(179, 71, 3)},
    {mcfile::blocks::minecraft::oak_door, Colour(141, 118, 71)},
    {mcfile::blocks::minecraft::oak_slab, kColorPlanksOak},
    {mcfile::blocks::minecraft::oak_trapdoor, Colour(141, 118, 71)},
    {mcfile::blocks::minecraft::obsidian, Colour(29, 14, 52)},
    {mcfile::blocks::minecraft::packed_ice, Colour(158, 158, 252)},
    {mcfile::blocks::minecraft::polished_granite, kColorStoneGranite},
    {mcfile::blocks::minecraft::prismarine, kColorPrismarine},
    {mcfile::blocks::minecraft::prismarine_bricks, Colour(91, 216, 210)},
    {mcfile::blocks::minecraft::rail, kColorRail},
    {mcfile::blocks::minecraft::redstone_ore, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::sandstone, kColorSandstone},
    {mcfile::blocks::minecraft::sea_lantern, Colour(252, 249, 242)},
    {mcfile::blocks::minecraft::snow, Colour(229, 229, 229)}, //
    {mcfile::blocks::minecraft::snow_block, Colour(252, 252, 252)},
    {mcfile::blocks::minecraft::powder_snow, Colour(252, 252, 252)},
    {mcfile::blocks::minecraft::spruce_door, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_fence, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_leaves, Colour(56, 95, 31)}, //
    {mcfile::blocks::minecraft::stone_brick_stairs, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::stone_bricks, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::stone_slab, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::spruce_log, kColorSpruceLog},
    {mcfile::blocks::minecraft::spruce_planks, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_slab, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_stairs, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_trapdoor, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::mossy_stone_bricks, kColorMossyStone},
    {mcfile::blocks::minecraft::chiseled_stone_bricks, kColorStone},
    {mcfile::blocks::minecraft::cracked_stone_bricks, kColorStone},
    {mcfile::blocks::minecraft::infested_stone, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::cobweb, Colour(255, 255, 255)},
    {mcfile::blocks::minecraft::blue_ice, Colour(102, 151, 246)},
    {mcfile::blocks::minecraft::magma_block, Colour(181, 64, 9)},
    {mcfile::blocks::minecraft::end_stone, Colour(219, 219, 172)},
    {mcfile::blocks::minecraft::end_portal, Colour(4, 18, 24)},
    {mcfile::blocks::minecraft::end_portal_frame, Colour(65, 114, 102)},
    {mcfile::blocks::minecraft::bedrock, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::mycelium, Colour(114, 96, 97)},
    {mcfile::blocks::minecraft::white_terracotta, Colour(209, 180, 161)},
    {mcfile::blocks::minecraft::orange_terracotta, Colour(165, 82, 40)},
    {mcfile::blocks::minecraft::magenta_terracotta, Colour(147, 87, 108)},
    {mcfile::blocks::minecraft::light_blue_terracotta, Colour(110, 106, 135)},
    {mcfile::blocks::minecraft::yellow_terracotta, Colour(184, 129, 33)},
    {mcfile::blocks::minecraft::lime_terracotta, Colour(102, 116, 52)},
    {mcfile::blocks::minecraft::pink_terracotta, Colour(160, 77, 78)},
    {mcfile::blocks::minecraft::gray_terracotta, Colour(57, 41, 36)},
    {mcfile::blocks::minecraft::light_gray_terracotta, Colour(137, 106, 99)},
    {mcfile::blocks::minecraft::cyan_terracotta, Colour(90, 94, 93)},
    {mcfile::blocks::minecraft::purple_terracotta, Colour(117, 69, 86)},
    {mcfile::blocks::minecraft::blue_terracotta, Colour(73, 58, 90)},
    {mcfile::blocks::minecraft::brown_terracotta, Colour(76, 50, 36)},
    {mcfile::blocks::minecraft::green_terracotta, Colour(75, 82, 41)},
    {mcfile::blocks::minecraft::red_terracotta, Colour(139, 58, 45)},
    {mcfile::blocks::minecraft::black_terracotta, Colour(37, 22, 15)},
    {mcfile::blocks::minecraft::terracotta, Colour(153, 95, 68)},
    {mcfile::blocks::minecraft::red_sand, Colour(201, 109, 36)},
    {mcfile::blocks::minecraft::purpur_slab, kColorPurPur},
    {mcfile::blocks::minecraft::purpur_block, kColorPurPur},
    {mcfile::blocks::minecraft::purpur_pillar, kColorPurPur},
    {mcfile::blocks::minecraft::ender_chest, Colour(39, 54, 56)},
    {mcfile::blocks::minecraft::tnt, Colour(216, 46, 26)},
    {mcfile::blocks::minecraft::prismarine_slab, kColorPrismarine},
    {mcfile::blocks::minecraft::prismarine_stairs, kColorPrismarine},
    {mcfile::blocks::minecraft::prismarine_bricks, Colour(89, 173, 162)},
    {mcfile::blocks::minecraft::prismarine_brick_slab, Colour(89, 173, 162)},
    {mcfile::blocks::minecraft::prismarine_brick_stairs, Colour(89, 173, 162)},
    {mcfile::blocks::minecraft::dark_prismarine, Colour(55, 97, 80)},
    {mcfile::blocks::minecraft::dark_prismarine_slab, Colour(55, 97, 80)},
    {mcfile::blocks::minecraft::dark_prismarine_stairs, Colour(55, 97, 80)},
    {mcfile::blocks::minecraft::netherrack, Colour(86, 32, 31)},
    {mcfile::blocks::minecraft::nether_bricks, kColorNetherBricks},
    {mcfile::blocks::minecraft::nether_brick_slab, kColorNetherBricks},
    {mcfile::blocks::minecraft::nether_brick_wall, kColorNetherBricks},
    {mcfile::blocks::minecraft::red_nether_bricks, kColorRedNetherBricks},
    {mcfile::blocks::minecraft::red_nether_brick_slab, kColorRedNetherBricks},
    {mcfile::blocks::minecraft::red_nether_brick_wall, kColorRedNetherBricks},
    {mcfile::blocks::minecraft::glowstone, Colour(248, 215, 115)},
    {mcfile::blocks::minecraft::nether_quartz_ore, Colour(170, 112, 105)},
    {mcfile::blocks::minecraft::soul_sand, Colour(72, 54, 43)},
	{mcfile::blocks::minecraft::white_wool, Colour(247, 247, 247)},
	{mcfile::blocks::minecraft::orange_wool, Colour(244, 122, 25)},
	{mcfile::blocks::minecraft::magenta_wool, Colour(193, 73, 183)},
	{mcfile::blocks::minecraft::light_blue_wool, Colour(65, 186, 220)},
	{mcfile::blocks::minecraft::yellow_wool, Colour(249, 206, 47)},
	{mcfile::blocks::minecraft::lime_wool, Colour(123, 193, 27)},
	{mcfile::blocks::minecraft::pink_wool, Colour(241, 160, 186)},
	{mcfile::blocks::minecraft::gray_wool, Colour(70, 78, 81)},
	{mcfile::blocks::minecraft::light_gray_wool, Colour(151, 151, 145)},
	{mcfile::blocks::minecraft::cyan_wool, Colour(22, 153, 154)},
	{mcfile::blocks::minecraft::purple_wool, Colour(132, 47, 179)},
	{mcfile::blocks::minecraft::blue_wool, Colour(57, 63, 164)},
	{mcfile::blocks::minecraft::brown_wool, Colour(125, 79, 46)},
	{mcfile::blocks::minecraft::green_wool, Colour(91, 119, 22)},
	{mcfile::blocks::minecraft::red_wool, Colour(170, 42, 36)},
	{mcfile::blocks::minecraft::black_wool, Colour(28, 28, 32)},
	{mcfile::blocks::minecraft::white_carpet, Colour(247, 247, 247)},
	{mcfile::blocks::minecraft::orange_carpet, Colour(244, 122, 25)},
	{mcfile::blocks::minecraft::magenta_carpet, Colour(193, 73, 183)},
	{mcfile::blocks::minecraft::light_blue_carpet, Colour(65, 186, 220)},
	{mcfile::blocks::minecraft::yellow_carpet, Colour(249, 206, 47)},
	{mcfile::blocks::minecraft::lime_carpet, Colour(123, 193, 27)},
	{mcfile::blocks::minecraft::pink_carpet, Colour(241, 160, 186)},
	{mcfile::blocks::minecraft::gray_carpet, Colour(70, 78, 81)},
	{mcfile::blocks::minecraft::light_gray_carpet, Colour(151, 151, 145)},
	{mcfile::blocks::minecraft::cyan_carpet, Colour(22, 153, 154)},
	{mcfile::blocks::minecraft::purple_carpet, Colour(132, 47, 179)},
	{mcfile::blocks::minecraft::blue_carpet, Colour(57, 63, 164)},
	{mcfile::blocks::minecraft::brown_carpet, Colour(125, 79, 46)},
	{mcfile::blocks::minecraft::green_carpet, Colour(91, 119, 22)},
	{mcfile::blocks::minecraft::red_carpet, Colour(170, 42, 36)},
	{mcfile::blocks::minecraft::black_carpet, Colour(28, 28, 32)},
	{mcfile::blocks::minecraft::white_bed, Colour(247, 247, 247)},
	{mcfile::blocks::minecraft::orange_bed, Colour(244, 122, 25)},
	{mcfile::blocks::minecraft::magenta_bed, Colour(193, 73, 183)},
	{mcfile::blocks::minecraft::light_blue_bed, Colour(65, 186, 220)},
	{mcfile::blocks::minecraft::yellow_bed, Colour(249, 206, 47)},
	{mcfile::blocks::minecraft::lime_bed, Colour(123, 193, 27)},
	{mcfile::blocks::minecraft::pink_bed, Colour(241, 160, 186)},
	{mcfile::blocks::minecraft::gray_bed, Colour(70, 78, 81)},
	{mcfile::blocks::minecraft::light_gray_bed, Colour(151, 151, 145)},
	{mcfile::blocks::minecraft::cyan_bed, Colour(22, 153, 154)},
	{mcfile::blocks::minecraft::purple_bed, Colour(132, 47, 179)},
	{mcfile::blocks::minecraft::blue_bed, Colour(57, 63, 164)},
	{mcfile::blocks::minecraft::brown_bed, Colour(125, 79, 46)},
	{mcfile::blocks::minecraft::green_bed, Colour(91, 119, 22)},
	{mcfile::blocks::minecraft::red_bed, Colour(170, 42, 36)},
	{mcfile::blocks::minecraft::black_bed, Colour(28, 28, 32)},
	{mcfile::blocks::minecraft::dried_kelp_block, Colour(43, 55, 32)},
	{mcfile::blocks::minecraft::beacon, Colour(72, 210, 202)},
	{mcfile::blocks::minecraft::shulker_box, Colour(149, 101, 149)},
	{mcfile::blocks::minecraft::white_shulker_box, Colour(225, 230, 230)},
	{mcfile::blocks::minecraft::orange_shulker_box, Colour(225, 230, 230)},
	{mcfile::blocks::minecraft::magenta_shulker_box, Colour(183, 61, 172)},
	{mcfile::blocks::minecraft::light_blue_shulker_box, Colour(57, 177, 215)},
	{mcfile::blocks::minecraft::yellow_shulker_box, Colour(249, 194, 34)},
	{mcfile::blocks::minecraft::lime_shulker_box, Colour(108, 183, 24)},
	{mcfile::blocks::minecraft::pink_shulker_box, Colour(239, 135, 166)},
	{mcfile::blocks::minecraft::gray_shulker_box, Colour(59, 63, 67)},
	{mcfile::blocks::minecraft::light_gray_shulker_box, Colour(135, 135, 126)},
	{mcfile::blocks::minecraft::cyan_shulker_box, Colour(22, 133, 144)},
	{mcfile::blocks::minecraft::purple_shulker_box, Colour(115, 38, 167)},
	{mcfile::blocks::minecraft::blue_shulker_box, Colour(49, 52, 152)},
	{mcfile::blocks::minecraft::brown_shulker_box, Colour(111, 69, 39)},
	{mcfile::blocks::minecraft::green_shulker_box, Colour(83, 107, 29)},
	{mcfile::blocks::minecraft::red_shulker_box, Colour(152, 35, 33)},
	{mcfile::blocks::minecraft::black_shulker_box, Colour(31, 31, 34)},
    {mcfile::blocks::minecraft::bricks, kColorBricks},
    {mcfile::blocks::minecraft::cut_sandstone, kColorSandstone},
    {mcfile::blocks::minecraft::sandstone_stairs, kColorSandstone},
    {mcfile::blocks::minecraft::chiseled_sandstone, kColorSandstone},
    {mcfile::blocks::minecraft::sandstone_slab, kColorSandstone},
    {mcfile::blocks::minecraft::dark_oak_door, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::polished_diorite, kColorStoneDiorite},
    {mcfile::blocks::minecraft::coarse_dirt, Colour(96, 67, 45)},
    {mcfile::blocks::minecraft::acacia_log, Colour(104, 97, 88)},
    {mcfile::blocks::minecraft::oak_pressure_plate, kColorPlanksOak},
    {mcfile::blocks::minecraft::fire, Colour(202, 115, 3)},
    {mcfile::blocks::minecraft::cobblestone_wall, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::cobblestone_slab, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::podzol, Colour(105, 67, 23)},
    {mcfile::blocks::minecraft::sticky_piston, Colour(131, 131, 131)},
    {mcfile::blocks::minecraft::piston_head, kColorPistonHead},
    {mcfile::blocks::minecraft::piston, Colour(131, 131, 131)},
    {mcfile::blocks::minecraft::lever, Colour(134, 133, 134)},
    {mcfile::blocks::minecraft::observer, Colour(100, 100, 100)},
    {mcfile::blocks::minecraft::slime_block, Colour(112, 187, 94)},
    {mcfile::blocks::minecraft::activator_rail, kColorRail},
    {mcfile::blocks::minecraft::oak_fence_gate, kColorPlanksOak},
    {mcfile::blocks::minecraft::dark_oak_fence_gate, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::birch_button, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_slab, kColorPlanksBirch},
    {mcfile::blocks::minecraft::acacia_stairs, Colour(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_pressure_plate, Colour(184, 98, 55)},
    {mcfile::blocks::minecraft::redstone_wire, Colour(75, 1, 0)},
    {mcfile::blocks::minecraft::redstone_block, Colour(162, 24, 8)},
    {mcfile::blocks::minecraft::redstone_lamp, Colour(173, 104, 58)},
    {mcfile::blocks::minecraft::hopper, Colour(70, 70, 70)},
    {mcfile::blocks::minecraft::crafting_table, Colour(156, 88, 49)},
    {mcfile::blocks::minecraft::lectern, Colour(164, 128, 73)},
    {mcfile::blocks::minecraft::acacia_planks, Colour(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_wood, Colour(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_slab, Colour(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_fence, Colour(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_fence_gate, Colour(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_door, Colour(184, 98, 55)},
    {mcfile::blocks::minecraft::stripped_acacia_log, Colour(184, 98, 55)},
    {mcfile::blocks::minecraft::brain_coral, Colour(225, 125, 183)},
    {mcfile::blocks::minecraft::brain_coral_block, Colour(225, 125, 183)},
    {mcfile::blocks::minecraft::brain_coral_fan, Colour(225, 125, 183)},
    {mcfile::blocks::minecraft::brain_coral_wall_fan, Colour(225, 125, 183)},
    {mcfile::blocks::minecraft::bubble_coral, Colour(198, 25, 184)},
    {mcfile::blocks::minecraft::bubble_coral_block, Colour(198, 25, 184)},
    {mcfile::blocks::minecraft::bubble_coral_fan, Colour(198, 25, 184)},
    {mcfile::blocks::minecraft::bubble_coral_wall_fan, Colour(198, 25, 184)},
    {mcfile::blocks::minecraft::horn_coral, Colour(234, 233, 76)},
    {mcfile::blocks::minecraft::horn_coral_block, Colour(234, 233, 76)},
    {mcfile::blocks::minecraft::horn_coral_fan, Colour(234, 233, 76)},
    {mcfile::blocks::minecraft::horn_coral_wall_fan, Colour(234, 233, 76)},
    {mcfile::blocks::minecraft::tube_coral, Colour(48, 78, 218)},
    {mcfile::blocks::minecraft::tube_coral_block, Colour(48, 78, 218)},
    {mcfile::blocks::minecraft::tube_coral_fan, Colour(48, 78, 218)},
    {mcfile::blocks::minecraft::tube_coral_wall_fan, Colour(48, 78, 218)},
    {mcfile::blocks::minecraft::fire_coral, Colour(196, 42, 54)},
    {mcfile::blocks::minecraft::fire_coral_block, Colour(196, 42, 54)},
    {mcfile::blocks::minecraft::fire_coral_fan, Colour(196, 42, 54)},
    {mcfile::blocks::minecraft::fire_coral_wall_fan, Colour(196, 42, 54)},
    {mcfile::blocks::minecraft::smooth_sandstone, kColorSandstone},
    {mcfile::blocks::minecraft::smooth_sandstone_slab, kColorSandstone},
    {mcfile::blocks::minecraft::smooth_sandstone_stairs, kColorSandstone},
    {mcfile::blocks::minecraft::sandstone_wall, kColorSandstone},
    {mcfile::blocks::minecraft::polished_andesite, kColorStoneAndesite},
    {mcfile::blocks::minecraft::carved_pumpkin, Colour(213, 125, 50)},
    {mcfile::blocks::minecraft::stripped_oak_wood, Colour(127, 85, 48)},
    {mcfile::blocks::minecraft::stonecutter, Colour(131, 131, 131)},
    {mcfile::blocks::minecraft::smoker, Colour(131, 131, 131)},
    {mcfile::blocks::minecraft::hay_block, Colour(203, 176, 7)},
    {mcfile::blocks::minecraft::birch_log, Colour(252, 252, 252)},
    {mcfile::blocks::minecraft::iron_trapdoor, Colour(227, 227, 227)},
    {mcfile::blocks::minecraft::bell, Colour(250, 211, 56)},
    {mcfile::blocks::minecraft::white_glazed_terracotta, Colour(246, 252, 251)},
    {mcfile::blocks::minecraft::orange_glazed_terracotta, Colour(26, 196, 197)},
    {mcfile::blocks::minecraft::magenta_glazed_terracotta, Colour(201, 87, 192)},
    {mcfile::blocks::minecraft::light_blue_glazed_terracotta, Colour(86, 187, 220)},
    {mcfile::blocks::minecraft::yellow_glazed_terracotta, Colour(251, 219, 93)},
    {mcfile::blocks::minecraft::lime_glazed_terracotta, Colour(137, 214, 35)},
    {mcfile::blocks::minecraft::pink_glazed_terracotta, Colour(241, 179, 201)},
    {mcfile::blocks::minecraft::gray_glazed_terracotta, Colour(94, 114, 118)},
    {mcfile::blocks::minecraft::light_gray_glazed_terracotta, Colour(199, 203, 207)},
    {mcfile::blocks::minecraft::cyan_glazed_terracotta, Colour(20, 159, 160)},
    {mcfile::blocks::minecraft::purple_glazed_terracotta, Colour(146, 53, 198)},
    {mcfile::blocks::minecraft::blue_glazed_terracotta, Colour(59, 66, 167)},
    {mcfile::blocks::minecraft::brown_glazed_terracotta, Colour(167, 120, 79)},
    {mcfile::blocks::minecraft::green_glazed_terracotta, Colour(111, 151, 36)},
    {mcfile::blocks::minecraft::furnace, kColorFurnace},
    {mcfile::blocks::minecraft::composter, Colour(139, 91, 49)},
    {mcfile::blocks::minecraft::campfire, Colour(199, 107, 3)},
    {mcfile::blocks::minecraft::cartography_table, Colour(86, 53, 24)},
    {mcfile::blocks::minecraft::brewing_stand, Colour(47, 47, 47)},
    {mcfile::blocks::minecraft::grindstone, Colour(141, 141, 141)},
    {mcfile::blocks::minecraft::fletching_table, Colour(212, 191, 131)},
    {mcfile::blocks::minecraft::iron_bars, Colour(154, 154, 154)},
    {mcfile::blocks::minecraft::bookshelf, Colour(192, 155, 97)},
    {mcfile::blocks::minecraft::acacia_sapling, Colour(125, 150, 33)},
    {mcfile::blocks::minecraft::potted_dead_bush, kColorPotter},
    {mcfile::blocks::minecraft::potted_cactus, kColorPotter},
    {mcfile::blocks::minecraft::jack_o_lantern, Colour(213, 125, 50)},
    {mcfile::blocks::minecraft::acacia_button, Colour(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_sign, Colour(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_trapdoor, Colour(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_wall_sign, Colour(184, 98, 55)},
    {mcfile::blocks::minecraft::andesite_slab, kColorStoneAndesite},
    {mcfile::blocks::minecraft::andesite_stairs, kColorStoneAndesite},
    {mcfile::blocks::minecraft::andesite_wall, kColorStoneAndesite},
    {mcfile::blocks::minecraft::anvil, kColorAnvil},
    {mcfile::blocks::minecraft::attached_melon_stem, Colour(203, 196, 187)},
    {mcfile::blocks::minecraft::bamboo_sapling, Colour(67, 103, 8)},
    {mcfile::blocks::minecraft::barrel, Colour(137, 102, 60)},
    {mcfile::blocks::minecraft::birch_door, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_fence_gate, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_pressure_plate, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_sapling, Colour(107, 156, 55)},
    {mcfile::blocks::minecraft::birch_sign, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_trapdoor, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_wall_sign, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_wood, Colour(252, 252, 252)},
    {mcfile::blocks::minecraft::black_concrete, Colour(8, 10, 15)},
    {mcfile::blocks::minecraft::black_concrete_powder, Colour(22, 24, 29)},
    {mcfile::blocks::minecraft::black_glazed_terracotta, Colour(24, 24, 27)},
    {mcfile::blocks::minecraft::blast_furnace, Colour(131, 131, 131)},
    {mcfile::blocks::minecraft::coal_block, Colour(13, 13, 13)},
    {mcfile::blocks::minecraft::diamond_block, Colour(100, 242, 224)},
    {mcfile::blocks::minecraft::emerald_block, Colour(62, 240, 130)},
    {mcfile::blocks::minecraft::gold_block, Colour(251, 221, 72)},
    {mcfile::blocks::minecraft::iron_block, Colour(227, 227, 227)},
    {mcfile::blocks::minecraft::iron_door, Colour(227, 227, 227)},
    {mcfile::blocks::minecraft::potted_acacia_sapling, kColorPotter},
    {mcfile::blocks::minecraft::potted_allium, kColorPotter},
    {mcfile::blocks::minecraft::potted_azure_bluet, kColorPotter},
    {mcfile::blocks::minecraft::potted_bamboo, kColorPotter},
    {mcfile::blocks::minecraft::potted_birch_sapling, kColorPotter},
    {mcfile::blocks::minecraft::potted_blue_orchid, kColorPotter},
    {mcfile::blocks::minecraft::potted_brown_mushroom, kColorPotter},
    {mcfile::blocks::minecraft::potted_cornflower, kColorPotter},
    {mcfile::blocks::minecraft::potted_dandelion, kColorPotter},
    {mcfile::blocks::minecraft::potted_dark_oak_sapling, kColorPotter},
    {mcfile::blocks::minecraft::potted_fern, kColorPotter},
    {mcfile::blocks::minecraft::potted_jungle_sapling, kColorPotter},
    {mcfile::blocks::minecraft::potted_lily_of_the_valley, kColorPotter},
    {mcfile::blocks::minecraft::potted_oak_sapling, kColorPotter},
    {mcfile::blocks::minecraft::potted_orange_tulip, kColorPotter},
    {mcfile::blocks::minecraft::potted_oxeye_daisy, kColorPotter},
    {mcfile::blocks::minecraft::potted_pink_tulip, kColorPotter},
    {mcfile::blocks::minecraft::potted_poppy, kColorPotter},
    {mcfile::blocks::minecraft::potted_red_mushroom, kColorPotter},
    {mcfile::blocks::minecraft::potted_red_tulip, kColorPotter},
    {mcfile::blocks::minecraft::potted_spruce_sapling, kColorPotter},
    {mcfile::blocks::minecraft::potted_white_tulip, kColorPotter},
    {mcfile::blocks::minecraft::potted_wither_rose, kColorPotter},
    {mcfile::blocks::minecraft::dark_oak_button, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::dark_oak_pressure_plate, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::dark_oak_sign,kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::dark_oak_wall_sign, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::oak_button, kColorPlanksOak},
    {mcfile::blocks::minecraft::oak_sign, kColorPlanksOak},
    {mcfile::blocks::minecraft::oak_wall_sign, kColorPlanksOak},
    {mcfile::blocks::minecraft::brick_slab, kColorBricks},
    {mcfile::blocks::minecraft::brick_stairs, kColorBricks},
    {mcfile::blocks::minecraft::brick_wall, kColorBricks},
    {mcfile::blocks::minecraft::chipped_anvil, kColorAnvil},
    {mcfile::blocks::minecraft::damaged_anvil, kColorAnvil},
    {mcfile::blocks::minecraft::daylight_detector, Colour(188, 168, 140)},
    {mcfile::blocks::minecraft::dead_brain_coral, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_brain_coral_block, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_brain_coral_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_brain_coral_wall_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_bubble_coral, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_bubble_coral_block, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_bubble_coral_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_bubble_coral_wall_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_fire_coral, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_fire_coral_block, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_fire_coral_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_fire_coral_wall_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_horn_coral, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_horn_coral_block, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_horn_coral_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_horn_coral_wall_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_tube_coral, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_tube_coral_block, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_tube_coral_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_tube_coral_wall_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::detector_rail, kColorRail},
    {mcfile::blocks::minecraft::powered_rail, kColorRail},
    {mcfile::blocks::minecraft::diorite_slab, kColorStoneDiorite},
    {mcfile::blocks::minecraft::diorite_stairs, kColorStoneDiorite},
    {mcfile::blocks::minecraft::diorite_wall, kColorStoneDiorite},
    {mcfile::blocks::minecraft::polished_diorite_slab, kColorStoneDiorite},
    {mcfile::blocks::minecraft::polished_diorite_stairs, kColorStoneDiorite},
    {mcfile::blocks::minecraft::granite_slab, kColorStoneGranite},
    {mcfile::blocks::minecraft::granite_stairs, kColorStoneGranite},
    {mcfile::blocks::minecraft::granite_wall, kColorStoneGranite},
    {mcfile::blocks::minecraft::polished_granite_slab, kColorStoneGranite},
    {mcfile::blocks::minecraft::polished_granite_stairs, kColorStoneGranite},
    {mcfile::blocks::minecraft::jungle_fence_gate, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_pressure_plate, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_sign, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_wall_sign, kColorPlanksJungle},
    {mcfile::blocks::minecraft::nether_brick_fence, kColorNetherBricks},
    {mcfile::blocks::minecraft::nether_brick_stairs, kColorNetherBricks},
    {mcfile::blocks::minecraft::stone_button, kColorStone},
    {mcfile::blocks::minecraft::stone_pressure_plate, kColorStone},
    {mcfile::blocks::minecraft::stone_stairs, kColorStone},
    {mcfile::blocks::minecraft::spruce_button, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_fence_gate, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_pressure_plate, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_sign, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_wall_sign, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::dispenser, kColorFurnace},
    {mcfile::blocks::minecraft::dropper, kColorFurnace},
    {mcfile::blocks::minecraft::quartz_block, kColorQuartz},
    {mcfile::blocks::minecraft::blue_concrete_powder, Colour(72, 75, 175)},
    {mcfile::blocks::minecraft::brown_concrete_powder, Colour(120, 81, 50)},
    {mcfile::blocks::minecraft::cyan_concrete_powder, Colour(37, 154, 160)},
    {mcfile::blocks::minecraft::gray_concrete_powder, Colour(75, 79, 82)},
    {mcfile::blocks::minecraft::green_concrete_powder, Colour(103, 126, 37)},
    {mcfile::blocks::minecraft::light_blue_concrete_powder, Colour(91, 194, 216)},
    {mcfile::blocks::minecraft::light_gray_concrete_powder, Colour(154, 154, 148)},
    {mcfile::blocks::minecraft::lime_concrete_powder, Colour(138, 197, 45)},
    {mcfile::blocks::minecraft::magenta_concrete_powder, Colour(200, 93, 193)},
    {mcfile::blocks::minecraft::orange_concrete_powder, Colour(230, 128, 20)},
    {mcfile::blocks::minecraft::pink_concrete_powder, Colour(236, 172, 195)},
    {mcfile::blocks::minecraft::purple_concrete_powder, Colour(138, 58, 186)},
    {mcfile::blocks::minecraft::red_concrete_powder, Colour(180, 58, 55)},
    {mcfile::blocks::minecraft::white_concrete_powder, Colour(222, 223, 224)},
    {mcfile::blocks::minecraft::yellow_concrete_powder, Colour(235, 209, 64)},
    {mcfile::blocks::minecraft::end_stone_brick_slab, kColorEndStoneBricks},
    {mcfile::blocks::minecraft::end_stone_brick_stairs, kColorEndStoneBricks},
    {mcfile::blocks::minecraft::end_stone_brick_wall, kColorEndStoneBricks},
    {mcfile::blocks::minecraft::end_stone_bricks, kColorEndStoneBricks},
    {mcfile::blocks::minecraft::blue_concrete, Colour(44, 46, 142)},
    {mcfile::blocks::minecraft::bone_block, Colour(199, 195, 165)},
    {mcfile::blocks::minecraft::brown_concrete, Colour(95, 58, 31)},
    {mcfile::blocks::minecraft::cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::white_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::orange_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::magenta_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::light_blue_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::yellow_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::lime_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::pink_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::gray_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::light_gray_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::cyan_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::purple_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::blue_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::brown_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::green_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::red_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::black_candle_cake, Colour(238, 229, 203)},
    {mcfile::blocks::minecraft::chain_command_block, Colour(159, 193, 178)},
    {mcfile::blocks::minecraft::chiseled_quartz_block, kColorQuartz},
    {mcfile::blocks::minecraft::chiseled_red_sandstone, kColorRedSandstone},
    {mcfile::blocks::minecraft::command_block, Colour(196, 125, 78)},
    {mcfile::blocks::minecraft::conduit, Colour(126, 113, 81)},
    {mcfile::blocks::minecraft::cut_red_sandstone, kColorRedSandstone},
    {mcfile::blocks::minecraft::cut_red_sandstone_slab, kColorRedSandstone},
    {mcfile::blocks::minecraft::red_sandstone, kColorRedSandstone},
    {mcfile::blocks::minecraft::red_sandstone_slab, kColorRedSandstone},
    {mcfile::blocks::minecraft::red_sandstone_stairs, kColorRedSandstone},
    {mcfile::blocks::minecraft::red_sandstone_wall, kColorRedSandstone},
    {mcfile::blocks::minecraft::smooth_red_sandstone, kColorRedSandstone},
    {mcfile::blocks::minecraft::smooth_red_sandstone_slab, kColorRedSandstone},
    {mcfile::blocks::minecraft::smooth_red_sandstone_stairs, kColorRedSandstone},
    {mcfile::blocks::minecraft::cut_sandstone_slab, kColorSandstone},
    {mcfile::blocks::minecraft::cyan_concrete, Colour(21, 118, 134)},
    {mcfile::blocks::minecraft::dark_oak_sapling, Colour(31, 100, 25)},
    {mcfile::blocks::minecraft::dark_oak_wood, Colour(62, 48, 29)},
    {mcfile::blocks::minecraft::dragon_egg, Colour(9, 9, 9)},
    {mcfile::blocks::minecraft::dragon_head, kColorDragonHead},
    {mcfile::blocks::minecraft::dragon_wall_head, kColorDragonHead},
    {mcfile::blocks::minecraft::quartz_pillar, kColorQuartz},
    {mcfile::blocks::minecraft::quartz_slab, kColorQuartz},
    {mcfile::blocks::minecraft::quartz_stairs, kColorQuartz},
    {mcfile::blocks::minecraft::emerald_ore, kColorStone},
    {mcfile::blocks::minecraft::polished_andesite_slab, kColorStoneAndesite},
    {mcfile::blocks::minecraft::polished_andesite_stairs, kColorStoneAndesite},
    {mcfile::blocks::minecraft::mossy_cobblestone_slab, kColorMossyStone},
    {mcfile::blocks::minecraft::mossy_cobblestone_stairs, kColorMossyStone},
    {mcfile::blocks::minecraft::mossy_cobblestone_wall, kColorMossyStone},
    {mcfile::blocks::minecraft::infested_cobblestone, kColorStone},
    {mcfile::blocks::minecraft::infested_mossy_stone_bricks, kColorMossyStone},
    {mcfile::blocks::minecraft::mossy_stone_brick_slab, kColorMossyStone},
    {mcfile::blocks::minecraft::mossy_stone_brick_stairs, kColorMossyStone},
    {mcfile::blocks::minecraft::mossy_stone_brick_wall, kColorMossyStone},
    {mcfile::blocks::minecraft::infested_chiseled_stone_bricks, kColorStone},
    {mcfile::blocks::minecraft::infested_cracked_stone_bricks, kColorStone},
    {mcfile::blocks::minecraft::infested_stone_bricks, kColorStone},
    {mcfile::blocks::minecraft::moving_piston, kColorPistonHead},
    {mcfile::blocks::minecraft::smooth_quartz, kColorQuartz},
    {mcfile::blocks::minecraft::smooth_quartz_slab, kColorQuartz},
    {mcfile::blocks::minecraft::smooth_quartz_stairs, kColorQuartz},
    {mcfile::blocks::minecraft::stone_brick_slab, kColorStone},
    {mcfile::blocks::minecraft::stone_brick_wall, kColorStone},
    {mcfile::blocks::minecraft::purpur_stairs, kColorPurPur},
    {mcfile::blocks::minecraft::prismarine_wall, kColorPrismarine},
    {mcfile::blocks::minecraft::red_nether_brick_stairs, kColorRedNetherBricks},
    {mcfile::blocks::minecraft::creeper_head, kColorCreaperHead},
    {mcfile::blocks::minecraft::creeper_wall_head, kColorCreaperHead},
    {mcfile::blocks::minecraft::enchanting_table, Colour(73, 234, 207)},
    {mcfile::blocks::minecraft::end_gateway, Colour(3, 13, 20)},
    {mcfile::blocks::minecraft::gray_concrete, Colour(53, 57, 61)},
    {mcfile::blocks::minecraft::green_concrete, Colour(72, 90, 35)},
    {mcfile::blocks::minecraft::heavy_weighted_pressure_plate, Colour(182, 182, 182)},
    {mcfile::blocks::minecraft::jigsaw, Colour(147, 120, 148)},
    {mcfile::blocks::minecraft::jukebox, Colour(122, 79, 56)},
    {mcfile::blocks::minecraft::jungle_sapling, Colour(41, 73, 12)},
    {mcfile::blocks::minecraft::jungle_wood, Colour(88, 69, 26)},
    {mcfile::blocks::minecraft::lantern, Colour(72, 79, 100)},
    {mcfile::blocks::minecraft::lapis_block, Colour(24, 59, 115)},
    {mcfile::blocks::minecraft::light_blue_concrete, Colour(37, 136, 198)},
    {mcfile::blocks::minecraft::light_gray_concrete, Colour(125, 125, 115)},
    {mcfile::blocks::minecraft::light_weighted_pressure_plate, Colour(202, 171, 50)},
    {mcfile::blocks::minecraft::lime_concrete, Colour(93, 167, 24)},
    {mcfile::blocks::minecraft::loom, Colour(200, 164, 112)},
    {mcfile::blocks::minecraft::magenta_concrete, Colour(168, 49, 158)},
    {mcfile::blocks::minecraft::nether_wart_block, Colour(122, 1, 0)},
    {mcfile::blocks::minecraft::note_block, Colour(146, 92, 64)},
    {mcfile::blocks::minecraft::oak_sapling, Colour(63, 141, 46)},
    {mcfile::blocks::minecraft::oak_wood, kColorOakLog},
    {mcfile::blocks::minecraft::orange_concrete, Colour(222, 97, 0)},
    {mcfile::blocks::minecraft::petrified_oak_slab, kColorPlanksOak},
    {mcfile::blocks::minecraft::pink_concrete, Colour(210, 100, 141)},
    {mcfile::blocks::minecraft::player_head, kColorPlayerHead},
    {mcfile::blocks::minecraft::player_wall_head, kColorPlayerHead},
    {mcfile::blocks::minecraft::purple_concrete, Colour(99, 32, 154)},
    {mcfile::blocks::minecraft::red_concrete, Colour(138, 32, 32)},
    {mcfile::blocks::minecraft::red_glazed_terracotta, Colour(202, 65, 57)},
    {mcfile::blocks::minecraft::comparator, Colour(185, 185, 185)},
    {mcfile::blocks::minecraft::repeater, Colour(185, 185, 185)},
    {mcfile::blocks::minecraft::repeating_command_block, Colour(105, 78, 197)},
    {mcfile::blocks::minecraft::scaffolding, Colour(225, 196, 115)},
    {mcfile::blocks::minecraft::skeleton_skull, kColorSkeltonSkull},
    {mcfile::blocks::minecraft::skeleton_wall_skull, kColorSkeltonSkull},
    {mcfile::blocks::minecraft::smithing_table, Colour(63, 65, 82)},
    {mcfile::blocks::minecraft::spawner, Colour(24, 43, 56)},
    {mcfile::blocks::minecraft::sponge, Colour(203, 204, 73)},
    {mcfile::blocks::minecraft::spruce_sapling, Colour(34, 52, 34)},
    {mcfile::blocks::minecraft::spruce_wood, kColorSpruceLog},
    {mcfile::blocks::minecraft::stripped_acacia_wood, Colour(185, 94, 61)},
    {mcfile::blocks::minecraft::stripped_birch_log, Colour(205, 186, 126)},
    {mcfile::blocks::minecraft::stripped_birch_wood, Colour(205, 186, 126)},
    {mcfile::blocks::minecraft::stripped_dark_oak_log, Colour(107, 83, 51)},
    {mcfile::blocks::minecraft::stripped_dark_oak_wood, Colour(107, 83, 51)},
    {mcfile::blocks::minecraft::stripped_jungle_log, Colour(173, 126, 82)},
    {mcfile::blocks::minecraft::stripped_jungle_wood, Colour(173, 126, 82)},
    {mcfile::blocks::minecraft::stripped_oak_log, Colour(148, 115, 64)},
    {mcfile::blocks::minecraft::stripped_spruce_log, Colour(120, 90, 54)},
    {mcfile::blocks::minecraft::stripped_spruce_wood, Colour(120, 90, 54)},
    {mcfile::blocks::minecraft::structure_block, Colour(147, 120, 148)},
    {mcfile::blocks::minecraft::trapped_chest, kColorChest},
    {mcfile::blocks::minecraft::tripwire_hook, Colour(135, 135, 135)},
    {mcfile::blocks::minecraft::turtle_egg, Colour(224, 219, 197)},
    {mcfile::blocks::minecraft::wet_sponge, Colour(174, 189, 74)},
    {mcfile::blocks::minecraft::white_concrete, Colour(204, 209, 210)},
    {mcfile::blocks::minecraft::wither_rose, Colour(23, 18, 16)},
    {mcfile::blocks::minecraft::wither_skeleton_skull, kColorWitherSkeltonSkull},
    {mcfile::blocks::minecraft::wither_skeleton_wall_skull, kColorWitherSkeltonSkull},
    {mcfile::blocks::minecraft::yellow_concrete, Colour(239, 175, 22)},
    {mcfile::blocks::minecraft::zombie_head, kColorZombieHead},
    {mcfile::blocks::minecraft::zombie_wall_head, kColorZombieHead},
    {mcfile::blocks::minecraft::end_rod, Colour(202, 202, 202)},
    {mcfile::blocks::minecraft::flower_pot, kColorPotter},
    {mcfile::blocks::minecraft::frosted_ice, Colour(109, 146, 193)},
    {mcfile::blocks::minecraft::nether_portal, Colour(78, 30, 135)},

    // plants
    {mcfile::blocks::minecraft::lily_pad, Colour(0, 123, 0)},
    {mcfile::blocks::minecraft::wheat, Colour(0, 123, 0)},
    {mcfile::blocks::minecraft::melon, Colour(125, 202, 25)},
    {mcfile::blocks::minecraft::pumpkin, Colour(213, 125, 50)},
    {mcfile::blocks::minecraft::grass, Colour(109, 141, 35)},
    {mcfile::blocks::minecraft::tall_grass, Colour(109, 141, 35)},
    {mcfile::blocks::minecraft::dandelion, Colour(245, 238, 50)},
    {mcfile::blocks::minecraft::poppy, Colour(229, 31, 29)},
    {mcfile::blocks::minecraft::peony, Colour(232, 143, 213)},
    {mcfile::blocks::minecraft::pink_tulip, Colour(234, 182, 209)},
    {mcfile::blocks::minecraft::orange_tulip, Colour(242, 118, 33)},
    {mcfile::blocks::minecraft::lilac, Colour(212, 119, 197)},
    {mcfile::blocks::minecraft::sunflower, Colour(245, 238, 50)},
    {mcfile::blocks::minecraft::allium, Colour(200, 109, 241)},
    {mcfile::blocks::minecraft::red_tulip, Colour(229, 31, 29)},
    {mcfile::blocks::minecraft::white_tulip, Colour(255, 255, 255)},
    {mcfile::blocks::minecraft::rose_bush, Colour(136, 40, 27)},
    {mcfile::blocks::minecraft::blue_orchid, Colour(47, 181, 199)},
    {mcfile::blocks::minecraft::oxeye_daisy, Colour(236, 246, 247)},
    {mcfile::blocks::minecraft::sugar_cane, Colour(165, 214, 90)},
    {mcfile::blocks::minecraft::chorus_plant, Colour(90, 51, 90)},
    {mcfile::blocks::minecraft::chorus_flower, Colour(159, 119, 159)},
    {mcfile::blocks::minecraft::dark_oak_leaves, Colour(58, 82, 23)},
    {mcfile::blocks::minecraft::red_mushroom_block, Colour(199, 42, 41)},
    {mcfile::blocks::minecraft::mushroom_stem, Colour(203, 196, 187)},
    {mcfile::blocks::minecraft::brown_mushroom_block, Colour(149, 113, 80)},
    {mcfile::blocks::minecraft::acacia_leaves, Colour(63, 89, 25)},
    {mcfile::blocks::minecraft::dead_bush, Colour(146, 99, 40)},
    {mcfile::blocks::minecraft::cactus, Colour(90, 138, 42)},
    {mcfile::blocks::minecraft::sweet_berry_bush, Colour(40, 97, 63)},
    {mcfile::blocks::minecraft::cornflower, Colour(69, 105, 232)},
    {mcfile::blocks::minecraft::pumpkin_stem, Colour(72, 65, 9)},
    {mcfile::blocks::minecraft::nether_wart, Colour(163, 35, 41)},
    {mcfile::blocks::minecraft::attached_pumpkin_stem, Colour(72, 65, 9)},
    {mcfile::blocks::minecraft::lily_of_the_valley, Colour(252, 252, 252)},
    {mcfile::blocks::minecraft::melon_stem, Colour(72, 65, 9)},
    {mcfile::blocks::minecraft::smooth_stone, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::smooth_stone_slab, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::bamboo, Colour(67, 103, 8)},
    {mcfile::blocks::minecraft::sea_pickle, Colour(106, 113, 42)},
    {mcfile::blocks::minecraft::cocoa, Colour(109, 112, 52)},

    // 1.15
    
    {mcfile::blocks::minecraft::bee_nest, Colour(198, 132, 67)},
    {mcfile::blocks::minecraft::beehive, Colour(182, 146, 94)},
    {mcfile::blocks::minecraft::honey_block, Colour(233, 145, 38)},
    {mcfile::blocks::minecraft::honeycomb_block, Colour(229, 138, 8)},

    // 1.16

    {mcfile::blocks::minecraft::crimson_nylium, Colour(146, 24, 24)},
    {mcfile::blocks::minecraft::warped_nylium, Colour(22, 125, 132)},
    {mcfile::blocks::minecraft::crimson_planks, kColorPlanksCrimson},
    {mcfile::blocks::minecraft::warped_planks, kColorPlanksWarped},
    {mcfile::blocks::minecraft::nether_gold_ore, Colour(245, 173, 42)},
    {mcfile::blocks::minecraft::crimson_stem, kColorPlanksCrimson},
    {mcfile::blocks::minecraft::warped_stem, kColorPlanksWarped},
    {mcfile::blocks::minecraft::stripped_crimson_stem, Colour(148, 61, 97)},
    {mcfile::blocks::minecraft::stripped_warped_stem, Colour(67, 159, 157)},
    {mcfile::blocks::minecraft::crimson_hyphae, Colour(148, 21, 21)},
    {mcfile::blocks::minecraft::warped_hyphae, Colour(22, 96, 90)},
    {mcfile::blocks::minecraft::crimson_slab, kColorPlanksCrimson},
    {mcfile::blocks::minecraft::warped_slab, kColorPlanksWarped},
    {mcfile::blocks::minecraft::cracked_nether_bricks, kColorNetherBricks},
    {mcfile::blocks::minecraft::chiseled_nether_bricks, kColorNetherBricks},
    {mcfile::blocks::minecraft::crimson_stairs, kColorPlanksCrimson},
    {mcfile::blocks::minecraft::warped_stairs, kColorPlanksWarped},
    {mcfile::blocks::minecraft::netherite_block, Colour(76, 72, 76)},
    {mcfile::blocks::minecraft::soul_soil, Colour(90, 68, 55)},
    {mcfile::blocks::minecraft::basalt, Colour(91, 91, 91)},
    {mcfile::blocks::minecraft::polished_basalt, Colour(115, 115, 115)},
    {mcfile::blocks::minecraft::smooth_basalt, Colour(91, 91, 91)},
    {mcfile::blocks::minecraft::ancient_debris, Colour(125, 95, 88)},
    {mcfile::blocks::minecraft::crying_obsidian, Colour(42, 1, 119)},
    {mcfile::blocks::minecraft::blackstone, kColorStoneBlack},
    {mcfile::blocks::minecraft::blackstone_slab, kColorStoneBlack},
    {mcfile::blocks::minecraft::blackstone_stairs, kColorStoneBlack},
    {mcfile::blocks::minecraft::gilded_blackstone, Colour(125, 68, 14)},
    {mcfile::blocks::minecraft::polished_blackstone, kColorStonePolishedBlack},
    {mcfile::blocks::minecraft::polished_blackstone_slab, kColorStonePolishedBlack},
    {mcfile::blocks::minecraft::polished_blackstone_stairs, kColorStonePolishedBlack},
    {mcfile::blocks::minecraft::chiseled_polished_blackstone, kColorStonePolishedBlack},
    {mcfile::blocks::minecraft::polished_blackstone_bricks, kColorPolishedBlackStoneBricks},
    {mcfile::blocks::minecraft::polished_blackstone_brick_slab, kColorPolishedBlackStoneBricks},
    {mcfile::blocks::minecraft::polished_blackstone_brick_stairs, kColorPolishedBlackStoneBricks},
    {mcfile::blocks::minecraft::cracked_polished_blackstone_bricks, kColorPolishedBlackStoneBricks},
    {mcfile::blocks::minecraft::crimson_fungus, Colour(162, 36, 40)},
    {mcfile::blocks::minecraft::warped_fungus, Colour(20, 178, 131)},
    {mcfile::blocks::minecraft::crimson_roots, Colour(171, 16, 28)},
    {mcfile::blocks::minecraft::warped_roots, Colour(20, 178, 131)},
    {mcfile::blocks::minecraft::nether_sprouts, Colour(20, 178, 131)},
    {mcfile::blocks::minecraft::weeping_vines, Colour(171, 16, 28)},
    {mcfile::blocks::minecraft::weeping_vines_plant, Colour(171, 16, 28)},
    {mcfile::blocks::minecraft::twisting_vines, Colour(20, 178, 131)},
    {mcfile::blocks::minecraft::crimson_fence, kColorPlanksCrimson},
    {mcfile::blocks::minecraft::warped_fence, kColorPlanksWarped},
    {mcfile::blocks::minecraft::soul_torch, Colour(123, 239, 242)},
    {mcfile::blocks::minecraft::chain, Colour(60, 65, 80)},
    {mcfile::blocks::minecraft::blackstone_wall, kColorStoneBlack},
    {mcfile::blocks::minecraft::polished_blackstone_wall, kColorStonePolishedBlack},
    {mcfile::blocks::minecraft::polished_blackstone_brick_wall, kColorPolishedBlackStoneBricks},
    {mcfile::blocks::minecraft::soul_lantern, Colour(123, 239, 242)},
    {mcfile::blocks::minecraft::soul_campfire, Colour(123, 239, 242)},
    {mcfile::blocks::minecraft::soul_fire, Colour(123, 239, 242)},
    {mcfile::blocks::minecraft::soul_wall_torch, Colour(123, 239, 242)},
    {mcfile::blocks::minecraft::shroomlight, Colour(251, 170, 108)},
    {mcfile::blocks::minecraft::lodestone, Colour(160, 162, 170)},
    {mcfile::blocks::minecraft::respawn_anchor, Colour(129, 8, 225)},
    {mcfile::blocks::minecraft::crimson_pressure_plate, kColorPlanksCrimson},
    {mcfile::blocks::minecraft::warped_pressure_plate, kColorPlanksWarped},
    {mcfile::blocks::minecraft::crimson_trapdoor, kColorPlanksCrimson},
    {mcfile::blocks::minecraft::warped_trapdoor, kColorPlanksWarped},
    {mcfile::blocks::minecraft::crimson_fence_gate, kColorPlanksCrimson},
    {mcfile::blocks::minecraft::warped_fence_gate, kColorPlanksWarped},
    {mcfile::blocks::minecraft::crimson_button, kColorPlanksCrimson},
    {mcfile::blocks::minecraft::warped_button, kColorPlanksWarped},
    {mcfile::blocks::minecraft::crimson_door, kColorPlanksCrimson},
    {mcfile::blocks::minecraft::warped_door, kColorPlanksWarped},
    {mcfile::blocks::minecraft::target, Colour(183, 49, 49)},
    
    // bugfix for 1.16
    {mcfile::blocks::minecraft::twisting_vines_plant, Colour(17, 153, 131)},
    {mcfile::blocks::minecraft::warped_wart_block, Colour(17, 153, 131)},
    {mcfile::blocks::minecraft::quartz_bricks, kColorQuartz},
    {mcfile::blocks::minecraft::stripped_crimson_hyphae, Colour(148, 61, 97)},
    {mcfile::blocks::minecraft::stripped_warped_hyphae, Colour(67, 159, 157)},
    {mcfile::blocks::minecraft::crimson_sign, kColorPlanksCrimson},
    {mcfile::blocks::minecraft::warped_sign, kColorPlanksWarped},
    {mcfile::blocks::minecraft::polished_blackstone_pressure_plate, kColorStonePolishedBlack},
    {mcfile::blocks::minecraft::polished_blackstone_button, kColorStonePolishedBlack},

    {mcfile::blocks::minecraft::crimson_wall_sign, kColorPlanksCrimson},
    {mcfile::blocks::minecraft::warped_wall_sign, kColorPlanksWarped},

    {mcfile::blocks::minecraft::deepslate, kColorDeepslate},
    {mcfile::blocks::minecraft::cobbled_deepslate, kColorDeepslate},
    {mcfile::blocks::minecraft::polished_deepslate, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_coal_ore, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_iron_ore, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_copper_ore, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_gold_ore, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_redstone_ore, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_emerald_ore, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_lapis_ore, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_diamond_ore, kColorDeepslate},
    {mcfile::blocks::minecraft::infested_deepslate, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_bricks, kColorDeepslate},
    {mcfile::blocks::minecraft::cracked_deepslate_bricks, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_tiles, kColorDeepslate},
    {mcfile::blocks::minecraft::cracked_deepslate_tiles, kColorDeepslate},
    {mcfile::blocks::minecraft::chiseled_deepslate, kColorDeepslate},
    {mcfile::blocks::minecraft::cobbled_deepslate_stairs, kColorDeepslate},
    {mcfile::blocks::minecraft::polished_deepslate_stairs, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_brick_stairs, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_tile_stairs, kColorDeepslate},
    {mcfile::blocks::minecraft::cobbled_deepslate_slab, kColorDeepslate},
    {mcfile::blocks::minecraft::polished_deepslate_slab, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_brick_slab, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_tile_slab, kColorDeepslate},
    {mcfile::blocks::minecraft::cobbled_deepslate_wall, kColorDeepslate},
    {mcfile::blocks::minecraft::polished_deepslate_wall, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_brick_wall, kColorDeepslate},
    {mcfile::blocks::minecraft::deepslate_tile_wall, kColorDeepslate},

    {mcfile::blocks::minecraft::copper_ore, kColorStone},

    {mcfile::blocks::minecraft::calcite, kColorStoneDiorite},
    {mcfile::blocks::minecraft::tuff, kColorStone},
    {mcfile::blocks::minecraft::dripstone_block, Colour(140, 116, 97)},
    {mcfile::blocks::minecraft::pointed_dripstone, Colour(140, 116, 97)},
    {mcfile::blocks::minecraft::raw_iron_block, Colour(109, 89, 64)},
    {mcfile::blocks::minecraft::raw_copper_block, Colour(145, 83, 62)},
    {mcfile::blocks::minecraft::raw_gold_block, Colour(173, 137, 34)},
    {mcfile::blocks::minecraft::amethyst_block, Colour(145, 104, 174)},
    {mcfile::blocks::minecraft::budding_amethyst, Colour(145, 104, 174)},

    {mcfile::blocks::minecraft::copper_block, kColorCopper},
    {mcfile::blocks::minecraft::cut_copper, kColorCopper},
    {mcfile::blocks::minecraft::waxed_copper_block, kColorCopper},
    {mcfile::blocks::minecraft::waxed_cut_copper, kColorCopper},
    {mcfile::blocks::minecraft::cut_copper_stairs, kColorCopper},
    {mcfile::blocks::minecraft::waxed_cut_copper_stairs, kColorCopper},
    {mcfile::blocks::minecraft::cut_copper_slab, kColorCopper},
    {mcfile::blocks::minecraft::waxed_cut_copper_slab, kColorCopper},

    {mcfile::blocks::minecraft::exposed_copper, kColorExposedCopper},
    {mcfile::blocks::minecraft::exposed_cut_copper, kColorExposedCopper},
    {mcfile::blocks::minecraft::waxed_exposed_copper, kColorExposedCopper},
    {mcfile::blocks::minecraft::waxed_exposed_cut_copper, kColorExposedCopper},
    {mcfile::blocks::minecraft::exposed_cut_copper_stairs, kColorExposedCopper},
    {mcfile::blocks::minecraft::waxed_exposed_cut_copper_stairs, kColorExposedCopper},
    {mcfile::blocks::minecraft::exposed_cut_copper_slab, kColorExposedCopper},
    {mcfile::blocks::minecraft::waxed_exposed_cut_copper_slab, kColorExposedCopper},

    {mcfile::blocks::minecraft::weathered_copper, kColorWeatheredCopper},
    {mcfile::blocks::minecraft::weathered_cut_copper, kColorWeatheredCopper},
    {mcfile::blocks::minecraft::waxed_weathered_copper, kColorWeatheredCopper},
    {mcfile::blocks::minecraft::waxed_weathered_cut_copper, kColorWeatheredCopper},
    {mcfile::blocks::minecraft::weathered_cut_copper_stairs, kColorWeatheredCopper},
    {mcfile::blocks::minecraft::waxed_weathered_cut_copper_stairs, kColorWeatheredCopper},
    {mcfile::blocks::minecraft::weathered_cut_copper_slab, kColorWeatheredCopper},
    {mcfile::blocks::minecraft::waxed_weathered_cut_copper_slab, kColorWeatheredCopper},

    {mcfile::blocks::minecraft::oxidized_copper, kColorOxidizedCopper},
    {mcfile::blocks::minecraft::oxidized_cut_copper, kColorOxidizedCopper},
    {mcfile::blocks::minecraft::waxed_oxidized_copper, kColorOxidizedCopper},
    {mcfile::blocks::minecraft::waxed_oxidized_cut_copper, kColorOxidizedCopper},
    {mcfile::blocks::minecraft::oxidized_cut_copper_stairs, kColorOxidizedCopper},
    {mcfile::blocks::minecraft::waxed_oxidized_cut_copper_stairs, kColorOxidizedCopper},
    {mcfile::blocks::minecraft::oxidized_cut_copper_slab, kColorOxidizedCopper},
    {mcfile::blocks::minecraft::waxed_oxidized_cut_copper_slab, kColorOxidizedCopper},

    {mcfile::blocks::minecraft::azalea_leaves, Colour(111, 144, 44)},
    {mcfile::blocks::minecraft::azalea, Colour(111, 144, 44)},
    {mcfile::blocks::minecraft::flowering_azalea, Colour(184, 97, 204)},
    {mcfile::blocks::minecraft::spore_blossom, Colour(184, 97, 204)},
    {mcfile::blocks::minecraft::moss_carpet, Colour(111, 144, 44)},
    {mcfile::blocks::minecraft::moss_block, Colour(111, 144, 44)},

    {mcfile::blocks::minecraft::potted_azalea_bush, kColorPotter},
    {mcfile::blocks::minecraft::potted_flowering_azalea_bush, kColorPotter},
    {mcfile::blocks::minecraft::cauldron, Colour(53, 52, 52)},
    {mcfile::blocks::minecraft::water_cauldron, Colour(53, 52, 52)},
    {mcfile::blocks::minecraft::lava_cauldron, Colour(53, 52, 52)},
    {mcfile::blocks::minecraft::powder_snow_cauldron, Colour(53, 52, 52)},
    {mcfile::blocks::minecraft::rooted_dirt, Colour(149, 108, 76)},
    {mcfile::blocks::minecraft::flowering_azalea_leaves, Colour(184, 97, 204)},
    {mcfile::blocks::minecraft::small_amethyst_bud, Colour(145, 104, 174)},
    {mcfile::blocks::minecraft::medium_amethyst_bud, Colour(145, 104, 174)},
    {mcfile::blocks::minecraft::large_amethyst_bud, Colour(145, 104, 174)},
    {mcfile::blocks::minecraft::amethyst_cluster, Colour(145, 104, 174)},
    {mcfile::blocks::minecraft::cave_vines, Colour(106, 126, 48)},
    {mcfile::blocks::minecraft::cave_vines_plant, Colour(106, 126, 48)},
    {mcfile::blocks::minecraft::potted_crimson_fungus, kColorPotter},
    {mcfile::blocks::minecraft::potted_warped_fungus, kColorPotter},
    {mcfile::blocks::minecraft::potted_crimson_roots, kColorPotter},
    {mcfile::blocks::minecraft::potted_warped_roots, kColorPotter},
    {mcfile::blocks::minecraft::sculk_sensor, Colour(7, 71, 86)},
    {mcfile::blocks::minecraft::reinforced_deepslate, kColorDeepslate},
    {mcfile::blocks::minecraft::sculk, Colour(5, 41, 49)},
    {mcfile::blocks::minecraft::sculk_catalyst, Colour(5, 41, 49)},
    {mcfile::blocks::minecraft::sculk_shrieker, Colour(5, 41, 49)},
    {mcfile::blocks::minecraft::mangrove_planks, kColorPlanksManvrove},
    {mcfile::blocks::minecraft::mangrove_roots, Colour(89, 71, 43)},
    {mcfile::blocks::minecraft::muddy_mangrove_roots, Colour(57, 55, 60)},
    {mcfile::blocks::minecraft::mangrove_log, Colour(89, 71, 43)},
    {mcfile::blocks::minecraft::stripped_mangrove_log, kColorPlanksManvrove},
    {mcfile::blocks::minecraft::stripped_mangrove_wood, kColorPlanksManvrove},
    {mcfile::blocks::minecraft::mangrove_wood, Colour(89, 71, 43)},
    {mcfile::blocks::minecraft::mangrove_slab, kColorPlanksManvrove},
    {mcfile::blocks::minecraft::mud_brick_slab, Colour(147, 112, 79)},
    {mcfile::blocks::minecraft::packed_mud, Colour(147, 112, 79)},
    {mcfile::blocks::minecraft::mud_bricks, Colour(147, 112, 79)},
    {mcfile::blocks::minecraft::mangrove_fence, kColorPlanksManvrove},
    {mcfile::blocks::minecraft::mangrove_stairs, kColorPlanksManvrove},
    {mcfile::blocks::minecraft::mangrove_fence_gate, kColorPlanksManvrove},
    {mcfile::blocks::minecraft::mud_brick_wall, Colour(147, 112, 79)},
    {mcfile::blocks::minecraft::mangrove_sign, kColorPlanksManvrove},
    {mcfile::blocks::minecraft::mangrove_wall_sign, kColorPlanksManvrove},
    {mcfile::blocks::minecraft::ochre_froglight, Colour(252, 249, 242)},
    {mcfile::blocks::minecraft::verdant_froglight, Colour(252, 249, 242)},
    {mcfile::blocks::minecraft::pearlescent_froglight, Colour(252, 249, 242)},
    {mcfile::blocks::minecraft::mangrove_leaves, Colour(59, 73, 16)},
    {mcfile::blocks::minecraft::mangrove_button, kColorPlanksManvrove},
    {mcfile::blocks::minecraft::mangrove_pressure_plate, kColorPlanksManvrove},
    {mcfile::blocks::minecraft::mangrove_door, kColorPlanksManvrove},
    {mcfile::blocks::minecraft::mangrove_trapdoor, kColorPlanksManvrove},
    {mcfile::blocks::minecraft::potted_mangrove_propagule, kColorPotter},
    {mcfile::blocks::minecraft::mud, Colour(57, 55, 60)},
};

static std::set<mcfile::blocks::BlockId> plantBlocks = {
    mcfile::blocks::minecraft::beetroots,
    mcfile::blocks::minecraft::carrots,
    mcfile::blocks::minecraft::potatoes,
    mcfile::blocks::minecraft::seagrass,
    mcfile::blocks::minecraft::tall_seagrass,
    mcfile::blocks::minecraft::fern,
    mcfile::blocks::minecraft::azure_bluet,
    mcfile::blocks::minecraft::kelp,
    mcfile::blocks::minecraft::large_fern,
    mcfile::blocks::minecraft::kelp_plant,
    mcfile::blocks::minecraft::big_dripleaf,
    mcfile::blocks::minecraft::big_dripleaf_stem,
    mcfile::blocks::minecraft::small_dripleaf,
};

static std::set<mcfile::blocks::BlockId> transparentBlocks = {
    mcfile::blocks::minecraft::air,
    mcfile::blocks::minecraft::cave_air,
    mcfile::blocks::minecraft::vine, // Colour(56, 95, 31)}, //
    mcfile::blocks::minecraft::glow_lichen,
    mcfile::blocks::minecraft::ladder, // Colour(255, 255, 255)},
    mcfile::blocks::minecraft::glass_pane,
    mcfile::blocks::minecraft::glass,
    mcfile::blocks::minecraft::brown_wall_banner,
    mcfile::blocks::minecraft::redstone_wall_torch,
    mcfile::blocks::minecraft::wall_torch,
    mcfile::blocks::minecraft::redstone_torch,
    mcfile::blocks::minecraft::torch,
    mcfile::blocks::minecraft::barrier,
    mcfile::blocks::minecraft::black_banner,
    mcfile::blocks::minecraft::black_wall_banner,
    mcfile::blocks::minecraft::black_stained_glass,
    mcfile::blocks::minecraft::black_stained_glass_pane,
    mcfile::blocks::minecraft::blue_banner,
    mcfile::blocks::minecraft::blue_stained_glass,
    mcfile::blocks::minecraft::blue_stained_glass_pane,
    mcfile::blocks::minecraft::blue_wall_banner,
    mcfile::blocks::minecraft::brown_banner,
    mcfile::blocks::minecraft::brown_stained_glass,
    mcfile::blocks::minecraft::brown_stained_glass_pane,
    mcfile::blocks::minecraft::gray_wall_banner,
    mcfile::blocks::minecraft::cyan_banner,
    mcfile::blocks::minecraft::cyan_wall_banner,
    mcfile::blocks::minecraft::cyan_stained_glass,
    mcfile::blocks::minecraft::cyan_stained_glass_pane,
    mcfile::blocks::minecraft::gray_banner,
    mcfile::blocks::minecraft::gray_stained_glass,
    mcfile::blocks::minecraft::gray_stained_glass_pane,
    mcfile::blocks::minecraft::green_banner,
    mcfile::blocks::minecraft::green_stained_glass,
    mcfile::blocks::minecraft::green_stained_glass_pane,
    mcfile::blocks::minecraft::green_wall_banner,
    mcfile::blocks::minecraft::light_blue_banner,
    mcfile::blocks::minecraft::light_blue_stained_glass,
    mcfile::blocks::minecraft::light_blue_stained_glass_pane,
    mcfile::blocks::minecraft::light_blue_wall_banner,
    mcfile::blocks::minecraft::light_gray_banner,
    mcfile::blocks::minecraft::light_gray_stained_glass,
    mcfile::blocks::minecraft::light_gray_stained_glass_pane,
    mcfile::blocks::minecraft::light_gray_wall_banner,
    mcfile::blocks::minecraft::lime_banner,
    mcfile::blocks::minecraft::lime_stained_glass,
    mcfile::blocks::minecraft::lime_stained_glass_pane,
    mcfile::blocks::minecraft::lime_wall_banner,
    mcfile::blocks::minecraft::magenta_banner,
    mcfile::blocks::minecraft::magenta_stained_glass,
    mcfile::blocks::minecraft::magenta_stained_glass_pane,
    mcfile::blocks::minecraft::magenta_wall_banner,
    mcfile::blocks::minecraft::orange_banner,
    mcfile::blocks::minecraft::orange_stained_glass,
    mcfile::blocks::minecraft::orange_stained_glass_pane,
    mcfile::blocks::minecraft::orange_wall_banner,
    mcfile::blocks::minecraft::pink_banner,
    mcfile::blocks::minecraft::pink_stained_glass,
    mcfile::blocks::minecraft::pink_stained_glass_pane,
    mcfile::blocks::minecraft::pink_wall_banner,
    mcfile::blocks::minecraft::purple_banner,
    mcfile::blocks::minecraft::purple_stained_glass,
    mcfile::blocks::minecraft::purple_stained_glass_pane,
    mcfile::blocks::minecraft::purple_wall_banner,
    mcfile::blocks::minecraft::red_banner,
    mcfile::blocks::minecraft::red_stained_glass,
    mcfile::blocks::minecraft::red_stained_glass_pane,
    mcfile::blocks::minecraft::red_wall_banner,
    mcfile::blocks::minecraft::white_banner,
    mcfile::blocks::minecraft::white_stained_glass,
    mcfile::blocks::minecraft::white_stained_glass_pane,
    mcfile::blocks::minecraft::white_wall_banner,
    mcfile::blocks::minecraft::yellow_banner,
    mcfile::blocks::minecraft::yellow_stained_glass,
    mcfile::blocks::minecraft::yellow_stained_glass_pane,
    mcfile::blocks::minecraft::yellow_wall_banner,
    mcfile::blocks::minecraft::void_air,
    mcfile::blocks::minecraft::structure_void,
    mcfile::blocks::minecraft::tripwire,

    mcfile::blocks::minecraft::hanging_roots,
    mcfile::blocks::minecraft::candle,
    mcfile::blocks::minecraft::white_candle,
    mcfile::blocks::minecraft::orange_candle,
    mcfile::blocks::minecraft::magenta_candle,
    mcfile::blocks::minecraft::light_blue_candle,
    mcfile::blocks::minecraft::yellow_candle,
    mcfile::blocks::minecraft::lime_candle,
    mcfile::blocks::minecraft::pink_candle,
    mcfile::blocks::minecraft::gray_candle,
    mcfile::blocks::minecraft::light_gray_candle,
    mcfile::blocks::minecraft::cyan_candle,
    mcfile::blocks::minecraft::purple_candle,
    mcfile::blocks::minecraft::blue_candle,
    mcfile::blocks::minecraft::brown_candle,
    mcfile::blocks::minecraft::green_candle,
    mcfile::blocks::minecraft::red_candle,
    mcfile::blocks::minecraft::black_candle,
    mcfile::blocks::minecraft::light,
    mcfile::blocks::minecraft::lightning_rod,
    mcfile::blocks::minecraft::tinted_glass,

    mcfile::blocks::minecraft::frogspawn,
    mcfile::blocks::minecraft::sculk_vein,
    mcfile::blocks::minecraft::mangrove_propagule,
};

Colour const RegionToTexture::kDefaultOceanColor(51, 89, 162);

std::map<Biome, Colour> const RegionToTexture::kOceanToColor = {
    {Biome::Ocean, RegionToTexture::kDefaultOceanColor},
    {Biome::LukewarmOcean, Colour(43, 122, 170)},
    {Biome::WarmOcean, Colour(56, 150, 177)},
    {Biome::ColdOcean, Colour(50, 66, 158)},
    {Biome::Swamp, Colour(115, 133, 120)},
    {Biome::MangroveSwamp, Colour(41, 81, 73)},
};

Colour const RegionToTexture::kDefaultFoliageColor(56, 95, 31);

std::map<Biome, Colour> const RegionToTexture::kFoliageToColor = {
    {Biome::Swamp, Colour(6975545)},
    {Biome::Badlands, Colour(10387789)},
};

static PixelARGB ToPixelInfo(uint32_t height, uint8_t waterDepth, uint8_t biome, uint32_t block, uint8_t biomeRadius)
{
    // [v3 pixel info]
    // h:            9bit
    // waterDepth:   7bit
    // biome:        3bit
    // block:       10bit
    // biomeRadius:  3bit
    static_assert((int)Biome::max_Biome <= 1 << 3, "");
    static_assert(mcfile::blocks::minecraft::minecraft_max_block_id <= 1 << 10, "");

    uint32_t depth = std::min(std::max((uint32_t)(waterDepth / double(0xFF) * double(0x7F)), (uint32_t)0), (uint32_t)0x7F);
    if (waterDepth > 0 && depth == 0) {
        depth = 1;
    }

    uint32_t const num = (0xFF800000 & (height << 23))
        | (0x7F0000 & (uint32_t(depth) << 16))
        | (0xE000 & (uint32_t(biome) << 13))
        | (0x1FF8 & (uint32_t(block) << 3))
        | (0x7 & (uint32_t)biomeRadius);
    PixelARGB p;
    p.setARGB(0xFF & (num >> 24), 0xFF & (num >> 16), 0xFF & (num >> 8), 0xFF & num);
    return p;
}

struct PixelInfo {
    int height;
    int waterDepth;
    mcfile::blocks::BlockId blockId;
};

void RegionToTexture::Load(mcfile::je::Region const& region, ThreadPoolJob *job, Dimension dim, std::function<void(PixelARGB *)> completion) {
    int const width = 512;
    int const height = 512;

    std::vector<PixelInfo> pixelInfo(width * height);
    std::fill(pixelInfo.begin(), pixelInfo.end(), PixelInfo { -1, 0, 0 });
    std::vector<Biome> biomes(width * height);
    
    int const minX = region.minBlockX();
    int const minZ = region.minBlockZ();

#if 0
    static std::atomic_bool init = true;
    if (init.exchange(false)) {
        std::set<mcfile::blocks::BlockId> ids;
        for (auto it : kBlockToColor) {
            ids.insert(it.first);
        }
        for (auto id : plantBlocks) {
            ids.insert(id);
        }
        for (auto id : transparentBlocks) {
            ids.insert(id);
        }
        int count = 0;
        for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
            auto const it = ids.find(id);
            if (id == mcfile::blocks::minecraft::water) {
                continue;
            }
            if (id == mcfile::blocks::minecraft::bubble_column) {
                continue;
            }
            if (it == ids.end()) {
                String name = mcfile::blocks::Name(id);
                count++;
                String n = name.replace(":", "::");
                //std::cout << "setblock " << count << " 4 0 " << name << std::endl;
                std::cout << "{mcfile::blocks::" << n << ", Colour()}," << std::endl;

                /*
                 saves/world/datapacks/pack_name/data/directory_for_namespace/functions/name.mcfunction
                 /function directory_for_namespace:name
                 */
            }
        }
    }
#endif

    bool error = false;
    bool didset = false;
    region.loadAllChunks(error, [&pixelInfo, &biomes, minX, minZ, width, height, job, dim, &didset](mcfile::je::Chunk const& chunk) {
        colormap::kbinani::Altitude altitude;
        int const sZ = chunk.minBlockZ();
        int const eZ = chunk.maxBlockZ();
        int const sX = chunk.minBlockX();
        int const eX = chunk.maxBlockX();
        for (int z = sZ; z <= eZ; z++) {
            for (int x = sX; x <= eX; x++) {
                Biome biome = ToBiome(chunk.biomeAt(x, z));
                int i = (z - minZ) * width + (x - minX);
                biomes[i] = biome;
            }
            if (job->shouldExit()) {
                return false;
            }
        }
        for (int z = sZ; z <= eZ; z++) {
            for (int x = sX; x <= eX; x++) {
                int const idx = (z - minZ) * width + (x - minX);
                assert(0 <= idx && idx < width * height);
                if (job->shouldExit()) {
                    return false;
                }
                uint8_t waterDepth = 0;
                int ymax = 319;
                int ymin = -64;

                int yini = ymax;
                if (dim == Dimension::TheNether) {
                    ymax = 127;
                    ymin = 0;
                    yini = ymax;
                    for (int y = ymax; y >= ymin; y--) {
                        auto block = chunk.blockIdAt(x, y, z);
                        if (!block) continue;
                        if (block == mcfile::blocks::minecraft::air) {
                            yini = y;
                            break;
                        }
                    }
                } else if (dim == Dimension::TheEnd) {
                    ymax = 255;
                    ymin = 0;
                }
                bool all_transparent = true;
                bool found_opaque_block = false;
                for (int y = yini; y >= ymin; y--) {
                    auto block = chunk.blockIdAt(x, y, z);
                    if (block == mcfile::blocks::unknown) {
                        continue;
                    }
                    if (block == mcfile::blocks::minecraft::water || block == mcfile::blocks::minecraft::bubble_column) {
                        waterDepth++;
                        all_transparent = false;
                        continue;
                    }
                    if (transparentBlocks.find(block) != transparentBlocks.end()) {
                        continue;
                    }
                    if (plantBlocks.find(block) != plantBlocks.end()) {
                        continue;
                    }
                    all_transparent = false;
                    auto it = kBlockToColor.find(block);
                    if (it == kBlockToColor.end()) {

                    } else {
                        int const h = std::min(std::max(y, 0), 511);
                        PixelInfo info;
                        info.height = h;
                        info.waterDepth = waterDepth;
                        info.blockId = block;
                        pixelInfo[idx] = info;
                        didset = true;
                        found_opaque_block = true;
                        break;
                    }
                }
                if (!found_opaque_block && waterDepth > 0) {
                    PixelInfo info;
                    info.height = 0;
                    info.waterDepth = waterDepth;
                    info.blockId = mcfile::blocks::minecraft::water;
                    pixelInfo[idx] = info;
                    didset = true;
                } else if (all_transparent) {
                    PixelInfo info;
                    info.height = 0;
                    info.waterDepth = 0;
                    info.blockId = mcfile::blocks::minecraft::air;
                    pixelInfo[idx] = info;
                    didset = true;
                }
            }
        }
        return true;
    });

    if (!didset) {
        completion(nullptr);
        return;
    }
    
    std::unique_ptr<PixelARGB[]> pixels(new PixelARGB[width * height]);
    std::fill_n(pixels.get(), width * height, PixelARGB(0, 0, 0, 0));
    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            int idx = z * width + x;
            PixelInfo info = pixelInfo[idx];
            if (info.height < 0) {
                continue;
            }
            Biome biome = biomes[idx];
            int biomeRadius;
            if (7 <= x && x < width - 7 && 7 <= z && z < height - 7) {
                biomeRadius = 7;
                for (int iz = -7; iz <= 7; iz++) {
                    for (int ix = -7; ix <= 7; ix++) {
                        int i = (z + iz) * width + x + ix;
                        Biome b = biomes[i];
                        if (b != biome) {
                            biomeRadius = std::min(std::min(biomeRadius, abs(ix)), abs(iz));
                        }
                    }
                }
            } else {
                biomeRadius = 0;
            }
            pixels[idx] = ToPixelInfo(info.height, info.waterDepth, (uint8_t)biome, (uint32_t)info.blockId, biomeRadius);
        }
    }

    completion(pixels.release());
}

File RegionToTexture::CacheFile(File const& file)
{
#if JUCE_WINDOWS
	File tmp = File::getSpecialLocation(File::SpecialLocationType::tempDirectory).getChildFile("mcview").getChildFile("cache");
#else
    File tmp = File::getSpecialLocation(File::SpecialLocationType::tempDirectory).getChildFile("cache");
#endif
	if (!tmp.exists()) {
        tmp.createDirectory();
    }
    String hash = String("v3.") + String(file.getParentDirectory().getFullPathName().hashCode64());
    File dir = tmp.getChildFile(hash);
    if (!dir.exists()) {
        dir.createDirectory();
    }
    return dir.getChildFile(file.getFileNameWithoutExtension() + String(".gz"));
}

RegionToTexture::RegionToTexture(File const& mcaFile, Region region, Dimension dim, bool useCache)
    : ThreadPoolJob(mcaFile.getFileName())
    , fRegionFile(mcaFile)
    , fRegion(region)
    , fDimension(dim)
    , fUseCache(useCache)
{
}

RegionToTexture::~RegionToTexture()
{
}

ThreadPoolJob::JobStatus RegionToTexture::runJob()
{
    try {
        int64 const modified = fRegionFile.getLastModificationTime().toMilliseconds();
        File cache = CacheFile(fRegionFile);
        if (fUseCache && cache.existsAsFile()) {
            FileInputStream stream(cache);
            GZIPDecompressorInputStream ungzip(stream);
            int expectedBytes = sizeof(PixelARGB) * 512 * 512;
            int64 cachedModificationTime = 0;
            if (ungzip.read(&cachedModificationTime, sizeof(cachedModificationTime)) != sizeof(cachedModificationTime)) {
                return ThreadPoolJob::jobHasFinished;
            }
            if (cachedModificationTime >= modified) {
                fPixels.reset(new PixelARGB[512 * 512]);
                if (ungzip.read(fPixels.get(), expectedBytes) != expectedBytes) {
                    fPixels.reset();
                }
                return ThreadPoolJob::jobHasFinished;
            }
        }

        auto region = mcfile::je::Region::MakeRegion(PathFromFile(fRegionFile));
        if (!region) {
            return ThreadPoolJob::jobHasFinished;
        }
        Load(*region, this, fDimension, [this](PixelARGB *pixels) {
            fPixels.reset(pixels);
        });
        if (shouldExit()) {
            return ThreadPoolJob::jobHasFinished;
        }
        FileOutputStream out(cache);
        out.truncate();
        out.setPosition(0);
        GZIPCompressorOutputStream gzip(out, 9);
        if (fPixels) {
            gzip.write(&modified, sizeof(modified));
            gzip.write(fPixels.get(), sizeof(PixelARGB) * 512 * 512);
        }
        return ThreadPoolJob::jobHasFinished;
    } catch (std::exception &e) {
        Logger::writeToLog(e.what());
        fPixels.reset();
        return ThreadPoolJob::jobHasFinished;
    } catch (...) {
        Logger::writeToLog("Unknown error");
        fPixels.reset();
        return ThreadPoolJob::jobHasFinished;
    }
}
