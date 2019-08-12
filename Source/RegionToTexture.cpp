#include "RegionToTexture.h"
#include <cassert>
#include "minecraft-file.hpp"
#include <colormap/colormap.h>
#include <map>
#include <set>

std::map<mcfile::blocks::BlockId, Colour> const RegionToTexture::kBlockToColor {
    {mcfile::blocks::minecraft::stone, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::granite, Colour(149, 108, 76)},
    {mcfile::blocks::minecraft::diorite, Colour(252, 249, 242)},
    {mcfile::blocks::minecraft::andesite, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::chest, Colour(141, 118, 71)},
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
    {mcfile::blocks::minecraft::mossy_cobblestone, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::oak_stairs, Colour(127, 85, 48)},
    {mcfile::blocks::minecraft::gravel, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::oak_log, Colour(141, 118, 71)},
    {mcfile::blocks::minecraft::oak_planks, Colour(127, 85, 48)},
    {mcfile::blocks::minecraft::wall_torch, Colour(255, 255, 255)},
    {mcfile::blocks::minecraft::farmland, Colour(149, 108, 76)},
    {mcfile::blocks::minecraft::oak_fence, Colour(127, 85, 48)},
    {mcfile::blocks::minecraft::cobblestone_stairs, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::grass_path, Colour(204, 204, 204)}, //
    {mcfile::blocks::minecraft::birch_fence, Colour(244, 230, 161)},
    {mcfile::blocks::minecraft::birch_planks, Colour(244, 230, 161)},
    {mcfile::blocks::minecraft::birch_stairs, Colour(244, 230, 161)},
    {mcfile::blocks::minecraft::dark_oak_fence, Colour(101, 75, 50)},
    {mcfile::blocks::minecraft::dark_oak_log, Colour(101, 75, 50)},
    {mcfile::blocks::minecraft::dark_oak_planks, Colour(191,152,63)}, //
    {mcfile::blocks::minecraft::dark_oak_slab, Colour(101, 75, 50)},
    {mcfile::blocks::minecraft::dark_oak_stairs, Colour(101, 75, 50)},
    {mcfile::blocks::minecraft::dark_oak_trapdoor, Colour(141, 118, 71)},
    {mcfile::blocks::minecraft::diamond_ore, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::gold_ore, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::ice, Colour(158, 158, 252)},
    {mcfile::blocks::minecraft::jungle_fence, Colour(149, 108, 76)},
    {mcfile::blocks::minecraft::jungle_log, Colour(149, 108, 76)},
    {mcfile::blocks::minecraft::jungle_planks, Colour(149, 108, 76)},
    {mcfile::blocks::minecraft::jungle_slab, Colour(149, 108, 76)},
    {mcfile::blocks::minecraft::jungle_stairs, Colour(149, 108, 76)},
    {mcfile::blocks::minecraft::jungle_button, Colour(149, 108, 76)},
    {mcfile::blocks::minecraft::jungle_door, Colour(149, 108, 76)},
    {mcfile::blocks::minecraft::jungle_trapdoor, Colour(141, 118, 71)},
    {mcfile::blocks::minecraft::lapis_ore, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::lava, Colour(179, 71, 3)},
    {mcfile::blocks::minecraft::oak_door, Colour(141, 118, 71)},
    {mcfile::blocks::minecraft::oak_slab, Colour(127, 85, 48)},
    {mcfile::blocks::minecraft::oak_trapdoor, Colour(141, 118, 71)},
    {mcfile::blocks::minecraft::obsidian, Colour(29, 14, 52)},
    {mcfile::blocks::minecraft::packed_ice, Colour(158, 158, 252)},
    {mcfile::blocks::minecraft::polished_granite, Colour(149, 108, 76)},
    {mcfile::blocks::minecraft::prismarine, Colour(75, 125, 151)},
    {mcfile::blocks::minecraft::prismarine_bricks, Colour(91, 216, 210)},
    {mcfile::blocks::minecraft::rail, Colour(154, 154, 154)},
    {mcfile::blocks::minecraft::redstone_ore, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::sandstone, Colour(244, 230, 161)},
    {mcfile::blocks::minecraft::sea_lantern, Colour(252, 249, 242)},
    {mcfile::blocks::minecraft::snow, Colour(229, 229, 229)}, //
    {mcfile::blocks::minecraft::snow_block, Colour(252, 252, 252)},
    {mcfile::blocks::minecraft::spruce_door, Colour(141, 118, 71)},
    {mcfile::blocks::minecraft::spruce_fence, Colour(141, 118, 71)},
    {mcfile::blocks::minecraft::spruce_leaves, Colour(56, 95, 31)}, //
    {mcfile::blocks::minecraft::stone_brick_stairs, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::stone_bricks, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::stone_slab, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::spruce_log, Colour(141, 118, 71)},
    {mcfile::blocks::minecraft::spruce_planks, Colour(127, 85, 48)},
    {mcfile::blocks::minecraft::spruce_slab, Colour(127, 85, 48)},
    {mcfile::blocks::minecraft::spruce_stairs, Colour(141, 118, 71)},
    {mcfile::blocks::minecraft::spruce_trapdoor, Colour(141, 118, 71)},
    {mcfile::blocks::minecraft::mossy_stone_bricks, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::chiseled_stone_bricks, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::cracked_stone_bricks, Colour(111, 111, 111)},
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
    {mcfile::blocks::minecraft::purpur_slab, Colour(170, 122, 170)},
    {mcfile::blocks::minecraft::purpur_block, Colour(170, 122, 170)},
    {mcfile::blocks::minecraft::purpur_pillar, Colour(170, 122, 170)},
    {mcfile::blocks::minecraft::ender_chest, Colour(39, 54, 56)},
    {mcfile::blocks::minecraft::tnt, Colour(216, 46, 26)},
    {mcfile::blocks::minecraft::prismarine, Colour(93, 158, 149)},
    {mcfile::blocks::minecraft::prismarine_slab, Colour(93, 158, 149)},
    {mcfile::blocks::minecraft::prismarine_stairs, Colour(93, 158, 149)},
    {mcfile::blocks::minecraft::prismarine_bricks, Colour(89, 173, 162)},
    {mcfile::blocks::minecraft::prismarine_brick_slab, Colour(89, 173, 162)},
    {mcfile::blocks::minecraft::prismarine_brick_stairs, Colour(89, 173, 162)},
    {mcfile::blocks::minecraft::dark_prismarine, Colour(55, 97, 80)},
    {mcfile::blocks::minecraft::dark_prismarine_slab, Colour(55, 97, 80)},
    {mcfile::blocks::minecraft::dark_prismarine_stairs, Colour(55, 97, 80)},
    {mcfile::blocks::minecraft::netherrack, Colour(86, 32, 31)},
    {mcfile::blocks::minecraft::nether_brick, Colour(33, 17, 20)},
    {mcfile::blocks::minecraft::nether_bricks, Colour(33, 17, 20)},
    {mcfile::blocks::minecraft::nether_brick_slab, Colour(33, 17, 20)},
    {mcfile::blocks::minecraft::nether_brick_wall, Colour(33, 17, 20)},
    {mcfile::blocks::minecraft::red_nether_bricks, Colour(89, 0, 0)},
    {mcfile::blocks::minecraft::red_nether_brick_slab, Colour(89, 0, 0)},
    {mcfile::blocks::minecraft::red_nether_brick_wall, Colour(89, 0, 0)},
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
    {mcfile::blocks::minecraft::bricks, Colour(175, 98, 76)},
    {mcfile::blocks::minecraft::cut_sandstone, Colour(244, 230, 161)},
    {mcfile::blocks::minecraft::sandstone_stairs, Colour(244, 230, 161)},
    {mcfile::blocks::minecraft::chiseled_sandstone, Colour(244, 230, 161)},
    {mcfile::blocks::minecraft::sandstone_slab, Colour(244, 230, 161)},
    {mcfile::blocks::minecraft::dark_oak_door, Colour(101, 75, 50)},
    {mcfile::blocks::minecraft::polished_diorite, Colour(252, 249, 242)},
    {mcfile::blocks::minecraft::coarse_dirt, Colour(96, 67, 45)},
    {mcfile::blocks::minecraft::acacia_log, Colour(104, 97, 88)},
    {mcfile::blocks::minecraft::oak_pressure_plate, Colour(127, 85, 48)},
    {mcfile::blocks::minecraft::fire, Colour(202, 115, 3)},
    {mcfile::blocks::minecraft::cobblestone_wall, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::cobblestone_slab, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::podzol, Colour(105, 67, 23)},
    {mcfile::blocks::minecraft::sticky_piston, Colour(131, 131, 131)},
    {mcfile::blocks::minecraft::piston_head, Colour(186, 150, 97)},
    {mcfile::blocks::minecraft::piston, Colour(131, 131, 131)},
    {mcfile::blocks::minecraft::lever, Colour(134, 133, 134)},
    {mcfile::blocks::minecraft::observer, Colour(100, 100, 100)},
    {mcfile::blocks::minecraft::slime_block, Colour(112, 187, 94)},
    {mcfile::blocks::minecraft::activator_rail, Colour(154, 154, 154)},
    {mcfile::blocks::minecraft::oak_fence_gate, Colour(127, 85, 48)},
    {mcfile::blocks::minecraft::dark_oak_fence_gate, Colour(101, 75, 50)},
    {mcfile::blocks::minecraft::birch_button, Colour(244, 230, 161)},
    {mcfile::blocks::minecraft::birch_slab, Colour(244, 230, 161)},
    {mcfile::blocks::minecraft::acacia_stairs, Colour(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_pressure_plate, Colour(184, 98, 55)},
    {mcfile::blocks::minecraft::redstone_wire, Colour(75, 1, 0)},
    {mcfile::blocks::minecraft::redstone_block, Colour(162, 24, 8)},
    {mcfile::blocks::minecraft::redstone_torch, Colour(184, 0, 0)},
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
    {mcfile::blocks::minecraft::smooth_sandstone, Colour(244, 230, 161)},
    {mcfile::blocks::minecraft::smooth_sandstone_slab, Colour(244, 230, 161)},
    {mcfile::blocks::minecraft::smooth_sandstone_stairs, Colour(244, 230, 161)},
    {mcfile::blocks::minecraft::sandstone_wall, Colour(244, 230, 161)},
    {mcfile::blocks::minecraft::polished_andesite, Colour(111, 111, 111)},
    {mcfile::blocks::minecraft::carved_pumpkin, Colour(213, 125, 50)},
    {mcfile::blocks::minecraft::stripped_oak_wood, Colour(127, 85, 48)},
    {mcfile::blocks::minecraft::stonecutter, Colour(131, 131, 131)},
    {mcfile::blocks::minecraft::smoker, Colour(131, 131, 131)},
    {mcfile::blocks::minecraft::hay_block, Colour(203, 176, 7)},
    {mcfile::blocks::minecraft::birch_log, Colour(252, 252, 252)},
    {mcfile::blocks::minecraft::torch, Colour(252, 252, 149)},
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
    {mcfile::blocks::minecraft::furnace, Colour(131, 131, 131)},
    {mcfile::blocks::minecraft::composter, Colour(139, 91, 49)},
    {mcfile::blocks::minecraft::campfire, Colour(199, 107, 3)},
    {mcfile::blocks::minecraft::cartography_table, Colour(86, 53, 24)},
    {mcfile::blocks::minecraft::brewing_stand, Colour(47, 47, 47)},
    {mcfile::blocks::minecraft::grindstone, Colour(141, 141, 141)},
    {mcfile::blocks::minecraft::fletching_table, Colour(212, 191, 131)},
    {mcfile::blocks::minecraft::iron_bars, Colour(154, 154, 154)},
    {mcfile::blocks::minecraft::bookshelf, Colour(192, 155, 97)},
    {mcfile::blocks::minecraft::acacia_sapling, Colour(125, 150, 33)},
    {mcfile::blocks::minecraft::potted_dead_bush, Colour(135, 75, 58)},
    {mcfile::blocks::minecraft::potted_cactus, Colour(135, 75, 58)},
    {mcfile::blocks::minecraft::jack_o_lantern, Colour(213, 125, 50)},

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

    //TODO
    {mcfile::blocks::minecraft::stripped_oak_log, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stripped_spruce_log, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stripped_birch_log, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stripped_jungle_log, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stripped_dark_oak_log, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stripped_spruce_wood, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stripped_birch_wood, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stripped_jungle_wood, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stripped_acacia_wood, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stripped_dark_oak_wood, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::oak_wood, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::spruce_wood, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::birch_wood, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::jungle_wood, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dark_oak_wood, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::sponge, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::wet_sponge, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::lapis_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::gold_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::iron_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::petrified_oak_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::brick_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stone_brick_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::quartz_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::red_sandstone_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::smooth_quartz, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::smooth_red_sandstone, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::purpur_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::diamond_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::brick_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::nether_brick_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::end_stone_bricks, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::emerald_ore, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::emerald_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::chiseled_quartz_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::quartz_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::quartz_pillar, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::quartz_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::coal_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::white_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::orange_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::magenta_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::light_blue_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::yellow_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::lime_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::pink_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::gray_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::light_gray_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cyan_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::purple_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::blue_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::brown_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::green_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::red_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::black_stained_glass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::red_sandstone, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::chiseled_red_sandstone, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cut_red_sandstone, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::red_sandstone_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::nether_wart_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::bone_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::white_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::orange_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::magenta_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::light_blue_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::yellow_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::lime_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::pink_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::gray_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::light_gray_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cyan_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::purple_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::blue_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::brown_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::green_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::red_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::black_concrete, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::white_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::orange_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::magenta_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::light_blue_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::yellow_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::lime_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::pink_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::gray_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::light_gray_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cyan_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::purple_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::blue_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::brown_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::green_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::red_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::black_concrete_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_tube_coral_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_brain_coral_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_bubble_coral_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_fire_coral_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_horn_coral_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::oak_sapling, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::spruce_sapling, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::birch_sapling, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::jungle_sapling, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dark_oak_sapling, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::end_rod, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::jukebox, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::infested_cobblestone, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::infested_stone_bricks, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::infested_mossy_stone_bricks, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::infested_cracked_stone_bricks, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::infested_chiseled_stone_bricks, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::nether_brick_fence, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::enchanting_table, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::mossy_cobblestone_wall, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::anvil, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::chipped_anvil, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::damaged_anvil, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::white_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::orange_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::magenta_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::light_blue_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::yellow_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::lime_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::pink_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::gray_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::light_gray_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cyan_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::purple_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::blue_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::brown_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::green_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::red_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::black_stained_glass_pane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::red_glazed_terracotta, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::black_glazed_terracotta, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_tube_coral_fan, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_brain_coral_fan, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_bubble_coral_fan, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_fire_coral_fan, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_horn_coral_fan, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::painting, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::sign, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::item_frame, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::flower_pot, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::skeleton_skull, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::wither_skeleton_skull, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::player_head, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::zombie_head, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::creeper_head, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dragon_head, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::armor_stand, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::white_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::orange_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::magenta_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::light_blue_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::yellow_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::lime_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::pink_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::gray_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::light_gray_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cyan_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::purple_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::blue_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::brown_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::green_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::red_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::black_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::end_crystal, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dispenser, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::note_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stone_pressure_plate, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::spruce_pressure_plate, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::birch_pressure_plate, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::jungle_pressure_plate, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dark_oak_pressure_plate, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stone_button, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::birch_trapdoor, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::acacia_trapdoor, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::spruce_fence_gate, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::birch_fence_gate, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::jungle_fence_gate, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::tripwire_hook, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::oak_button, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::spruce_button, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::acacia_button, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dark_oak_button, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::trapped_chest, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::light_weighted_pressure_plate, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::heavy_weighted_pressure_plate, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::daylight_detector, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dropper, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::iron_door, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::birch_door, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::repeater, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::comparator, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::redstone, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::powered_rail, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::detector_rail, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::minecart, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::saddle, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::oak_boat, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::chest_minecart, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::furnace_minecart, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::carrot_on_a_stick, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::tnt_minecart, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::hopper_minecart, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::elytra, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::spruce_boat, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::birch_boat, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::jungle_boat, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::acacia_boat, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dark_oak_boat, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::turtle_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::conduit, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::scute, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::coal, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::charcoal, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::diamond, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::iron_ingot, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::gold_ingot, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stick, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::bowl, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::string, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::feather, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::gunpowder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::wheat_seeds, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::flint, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::bucket, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::water_bucket, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::lava_bucket, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::snowball, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::leather, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::milk_bucket, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::pufferfish_bucket, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::salmon_bucket, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cod_bucket, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::tropical_fish_bucket, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::brick, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::clay_ball, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::paper, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::book, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::slime_ball, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::glowstone_dust, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::ink_sac, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::rose_red, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cactus_green, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cocoa_beans, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::lapis_lazuli, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::purple_dye, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cyan_dye, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::light_gray_dye, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::gray_dye, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::pink_dye, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::lime_dye, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dandelion_yellow, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::light_blue_dye, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::magenta_dye, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::orange_dye, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::bone_meal, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::bone, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::sugar, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::pumpkin_seeds, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::melon_seeds, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::ender_pearl, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::blaze_rod, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::gold_nugget, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::ender_eye, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::bat_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::blaze_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cave_spider_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::chicken_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cod_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cow_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::creeper_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dolphin_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::donkey_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::drowned_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::elder_guardian_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::enderman_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::endermite_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::evoker_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::ghast_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::guardian_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::horse_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::husk_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::llama_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::magma_cube_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::mooshroom_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::mule_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::ocelot_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::parrot_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::phantom_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::pig_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::polar_bear_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::pufferfish_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::rabbit_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::salmon_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::sheep_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::shulker_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::silverfish_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::skeleton_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::skeleton_horse_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::slime_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::spider_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::squid_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stray_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::tropical_fish_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::turtle_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::vex_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::villager_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::vindicator_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::witch_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::wither_skeleton_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::wolf_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::zombie_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::zombie_horse_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::zombie_pigman_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::zombie_villager_spawn_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::experience_bottle, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::fire_charge, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::writable_book, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::emerald, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::map, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::nether_star, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::firework_rocket, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::firework_star, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::quartz, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::prismarine_shard, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::prismarine_crystals, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::rabbit_hide, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::iron_horse_armor, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::golden_horse_armor, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::diamond_horse_armor, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::chorus_fruit, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::popped_chorus_fruit, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::beetroot_seeds, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::shulker_shell, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::iron_nugget, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::music_disc_13, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::music_disc_cat, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::music_disc_blocks, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::music_disc_chirp, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::music_disc_far, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::music_disc_mall, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::music_disc_mellohi, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::music_disc_stal, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::music_disc_strad, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::music_disc_ward, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::music_disc_11, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::music_disc_wait, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::nautilus_shell, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::heart_of_the_sea, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::apple, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::mushroom_stew, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::bread, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::porkchop, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cooked_porkchop, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::golden_apple, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::enchanted_golden_apple, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cod, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::salmon, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::tropical_fish, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::pufferfish, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cooked_cod, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cooked_salmon, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cake, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cookie, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::melon_slice, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dried_kelp, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::beef, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cooked_beef, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::chicken, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cooked_chicken, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::rotten_flesh, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::spider_eye, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::carrot, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potato, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::baked_potato, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::poisonous_potato, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::pumpkin_pie, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::rabbit, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cooked_rabbit, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::rabbit_stew, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::mutton, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cooked_mutton, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::beetroot, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::beetroot_soup, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::iron_shovel, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::iron_pickaxe, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::iron_axe, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::flint_and_steel, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::wooden_shovel, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::wooden_pickaxe, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::wooden_axe, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stone_shovel, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stone_pickaxe, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stone_axe, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::diamond_shovel, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::diamond_pickaxe, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::diamond_axe, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::golden_shovel, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::golden_pickaxe, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::golden_axe, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::wooden_hoe, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stone_hoe, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::iron_hoe, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::diamond_hoe, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::golden_hoe, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::compass, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::fishing_rod, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::clock, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::shears, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::enchanted_book, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::lead, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::name_tag, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::turtle_helmet, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::bow, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::arrow, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::iron_sword, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::wooden_sword, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stone_sword, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::diamond_sword, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::golden_sword, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::leather_helmet, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::leather_chestplate, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::leather_leggings, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::leather_boots, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::chainmail_helmet, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::chainmail_chestplate, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::chainmail_leggings, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::chainmail_boots, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::iron_helmet, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::iron_chestplate, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::iron_leggings, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::iron_boots, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::diamond_helmet, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::diamond_chestplate, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::diamond_leggings, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::diamond_boots, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::golden_helmet, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::golden_chestplate, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::golden_leggings, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::golden_boots, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::spectral_arrow, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::tipped_arrow, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::shield, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::totem_of_undying, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::trident, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::ghast_tear, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potion, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::glass_bottle, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::fermented_spider_eye, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::blaze_powder, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::magma_cream, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cauldron, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::glistering_melon_slice, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::golden_carrot, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::rabbit_foot, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dragon_breath, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::splash_potion, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::lingering_potion, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::phantom_membrane, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::command_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::repeating_command_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::chain_command_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::command_block_minecart, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::debug_stick, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dragon_egg, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::written_book, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::filled_map, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::knowledge_book, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::spawner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::barrier, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::structure_block, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::structure_void, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::void_air, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::water, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::attached_melon_stem, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::moving_piston, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::nether_portal, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::end_gateway, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::redstone_wall_torch, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::tripwire, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::frosted_ice, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::bubble_column, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_tube_coral_wall_fan, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_brain_coral_wall_fan, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_bubble_coral_wall_fan, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_fire_coral_wall_fan, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_horn_coral_wall_fan, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::wall_sign, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::white_wall_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::orange_wall_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::magenta_wall_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::light_blue_wall_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::yellow_wall_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::lime_wall_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::pink_wall_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::gray_wall_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::light_gray_wall_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cyan_wall_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::purple_wall_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::blue_wall_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::green_wall_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::red_wall_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::black_wall_banner, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::skeleton_wall_skull, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::wither_skeleton_wall_skull, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::player_wall_head, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::zombie_wall_head, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::creeper_wall_head, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dragon_wall_head, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_oak_sapling, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_spruce_sapling, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_birch_sapling, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_acacia_sapling, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_jungle_sapling, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_dark_oak_sapling, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_brown_mushroom, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_red_mushroom, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_allium, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_azure_bluet, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_blue_orchid, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_dandelion, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_fern, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_oxeye_daisy, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_poppy, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_orange_tulip, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_pink_tulip, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_red_tulip, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_white_tulip, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::acacia_sign, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::acacia_wall_sign, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::andesite_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::andesite_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::andesite_wall, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::bamboo_sapling, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::barrel, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::birch_sign, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::birch_wall_sign, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::blast_furnace, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::brick_wall, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cocoa, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cut_red_sandstone_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::cut_sandstone_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dark_oak_sign, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dark_oak_wall_sign, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_brain_coral, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_bubble_coral, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_fire_coral, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_horn_coral, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::dead_tube_coral, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::diorite_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::diorite_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::diorite_wall, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::end_stone_brick_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::end_stone_brick_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::end_stone_brick_wall, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::granite_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::granite_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::granite_wall, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::jigsaw, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::jungle_sign, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::jungle_wall_sign, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::lantern, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::loom, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::mossy_cobblestone_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::mossy_cobblestone_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::mossy_stone_brick_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::mossy_stone_brick_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::mossy_stone_brick_wall, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::oak_sign, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::oak_wall_sign, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::polished_andesite_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::polished_andesite_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::polished_diorite_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::polished_diorite_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::polished_granite_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::polished_granite_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_bamboo, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_cornflower, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_lily_of_the_valley, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::potted_wither_rose, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::prismarine_wall, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::red_nether_brick_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::red_sandstone_wall, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::scaffolding, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::smithing_table, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::smooth_quartz_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::smooth_quartz_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::smooth_red_sandstone_slab, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::smooth_red_sandstone_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::spruce_sign, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::spruce_wall_sign, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stone_brick_wall, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::stone_stairs, Colour(0, 0, 0)},
    {mcfile::blocks::minecraft::wither_rose, Colour(0, 0, 0)},
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
};

static std::set<mcfile::blocks::BlockId> transparentBlocks = {
    mcfile::blocks::minecraft::air,
    mcfile::blocks::minecraft::cave_air,
    mcfile::blocks::minecraft::vine, // Colour(56, 95, 31)}, //
    mcfile::blocks::minecraft::ladder, // Colour(255, 255, 255)},
    mcfile::blocks::minecraft::glass_pane,
    mcfile::blocks::minecraft::glass,
    mcfile::blocks::minecraft::brown_wall_banner,
};

Colour const RegionToTexture::kDefaultOceanColor(51, 89, 162);

std::map<Biome, Colour> const RegionToTexture::kOceanToColor = {
    {Biome::Ocean, RegionToTexture::kDefaultOceanColor},
    {Biome::LukewarmOcean, Colour(43, 122, 170)},
    {Biome::WarmOcean, Colour(56, 150, 177)},
    {Biome::ColdOcean, Colour(50, 66, 158)},
    {Biome::FrozenOcean, Colour(50, 47, 155)},
    {Biome::Swamp, Colour(115, 133, 120)},
};

Colour const RegionToTexture::kDefaultFoliageColor(56, 95, 31);

std::map<Biome, Colour> const RegionToTexture::kFoliageToColor = {
    {Biome::Swamp, Colour(6975545)},
    {Biome::Badlands, Colour(10387789)},
};

static PixelARGB ToPixelInfo(uint8_t height, uint8_t waterDepth, uint8_t biome, uint32_t block, uint8_t biomeRadius)
{
    // h:            8bit
    // waterDepth:   7bit
    // biome:        4bit
    // block:       10bit
    // biomeRadius:  3bit
    static_assert((int)Biome::max_Biome <= 1 << 4, "");
    static_assert(mcfile::blocks::minecraft::minecraft_max_block_id <= 1 << 10, "");

    uint32_t depth = std::min(std::max((uint32_t)(waterDepth / double(0xFF) * double(0x7F)), (uint32_t)0), (uint32_t)0x7F);
    if (waterDepth > 0 && depth == 0) {
        depth = 1;
    }
    
    uint32_t const num = ((0xFF & (uint32_t)height) << 24)
        | ((0x7F & (uint32_t)depth) << 17)
        | ((0xF & (uint32_t)biome) << 13)
        | ((0x3FF & (uint32_t)block) << 3)
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

void RegionToTexture::Load(mcfile::Region const& region, ThreadPoolJob *job, Dimension dim, std::function<void(PixelARGB *)> completion) {
    int const width = 512;
    int const height = 512;

    std::vector<PixelInfo> pixelInfo(width * height);
    std::fill(pixelInfo.begin(), pixelInfo.end(), PixelInfo { -1, 0, 0 });
    std::vector<Biome> biomes(width * height);
    
    int const minX = region.minBlockX();
    int const minZ = region.minBlockZ();

    bool error = false;
    bool didset = false;
    region.loadAllChunks(error, [&pixelInfo, &biomes, minX, minZ, width, height, job, dim, &didset](mcfile::Chunk const& chunk) {
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
                int yini = 255;
                if (dim == Dimension::TheNether) {
                    yini = 127;
                    for (int y = 127; y >= 0; y--) {
                        auto block = chunk.blockIdAt(x, y, z);
                        if (!block) continue;
                        if (block == mcfile::blocks::minecraft::air) {
                            yini = y;
                            break;
                        }
                    }
                }
                bool all_transparent = true;
                for (int y = yini; y >= 0; y--) {
                    auto block = chunk.blockIdAt(x, y, z);
                    if (!block) {
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
#if JUCE_DEBUG
                        static std::set<std::string> unknown_blocks;
                        static CriticalSection unknown_blocks_cs;
                        
                        ScopedLock lk(unknown_blocks_cs);
                        auto b = chunk.blockAt(x, y, z);
                        auto name = b->fName;
                        if (unknown_blocks.find(name) == unknown_blocks.end()) {
                            unknown_blocks.insert(name);
                            String n(name);
                            std::cout << "{mcfile::blocks::" << n.replace(":", "::") << ", Colour(0, 0, 0)}," << std::endl;
                        }
#endif
                    } else {
                        uint8_t const h = (uint8)std::min(std::max(y, 0), 255);
                        PixelInfo info;
                        info.height = h;
                        info.waterDepth = waterDepth;
                        info.blockId = block;
                        pixelInfo[idx] = info;
                        didset = true;
                        break;
                    }
                }
                if (all_transparent) {
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
    String hash = String("v2.") + String(file.getParentDirectory().getFullPathName().hashCode64());
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

        auto region = mcfile::Region::MakeRegion(fRegionFile.getFullPathName().toStdString());
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
