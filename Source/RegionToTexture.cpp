#include <juce_gui_basics/juce_gui_basics.h>
#include <minecraft-file.hpp>

#include <fstream>

#include "Dimension.hpp"

#include "RegionToTexture.hpp"

using namespace juce;
using namespace mcview;
using namespace mcfile::blocks::minecraft;

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

static PixelARGB ToPixelInfo(uint32_t height, uint8_t waterDepth, uint8_t biome, uint32_t block, uint8_t biomeRadius) {
  // [v3 pixel info]
  // h:            9bit
  // waterDepth:   7bit
  // biome:        3bit
  // block:       10bit
  // biomeRadius:  3bit
  static_assert((int)Biome::max_Biome <= 1 << 3, "");
  static_assert(minecraft_max_block_id <= 1 << 10, "");

  uint32_t depth = std::min(std::max((uint32_t)(waterDepth / double(0xFF) * double(0x7F)), (uint32_t)0), (uint32_t)0x7F);
  if (waterDepth > 0 && depth == 0) {
    depth = 1;
  }

  uint32_t const num = (0xFF800000 & (height << 23)) | (0x7F0000 & (uint32_t(depth) << 16)) | (0xE000 & (uint32_t(biome) << 13)) | (0x1FF8 & (uint32_t(block) << 3)) | (0x7 & (uint32_t)biomeRadius);
  PixelARGB p;
  p.setARGB(0xFF & (num >> 24), 0xFF & (num >> 16), 0xFF & (num >> 8), 0xFF & num);
  return p;
}

static inline Biome ToBiome(mcfile::biomes::BiomeId b) {
  switch (b) {
  case mcfile::biomes::minecraft::ocean:
  case mcfile::biomes::minecraft::deep_ocean:
    return Biome::Ocean;
  case mcfile::biomes::minecraft::lukewarm_ocean:
  case mcfile::biomes::minecraft::deep_lukewarm_ocean:
    return Biome::LukewarmOcean;
  case mcfile::biomes::minecraft::warm_ocean:
  case mcfile::biomes::minecraft::deep_warm_ocean:
    return Biome::WarmOcean;
  case mcfile::biomes::minecraft::cold_ocean:
  case mcfile::biomes::minecraft::deep_cold_ocean:
  case mcfile::biomes::minecraft::frozen_ocean:
  case mcfile::biomes::minecraft::deep_frozen_ocean:
    return Biome::ColdOcean;
  case mcfile::biomes::minecraft::swamp:
  case mcfile::biomes::minecraft::swamp_hills:
    return Biome::Swamp;
  case mcfile::biomes::minecraft::badlands:
    return Biome::Badlands;
  case mcfile::biomes::minecraft::mangrove_swamp:
    return Biome::MangroveSwamp;
  default:
    return Biome::Other;
  }
}

struct PixelInfo {
  int height;
  int waterDepth;
  mcfile::blocks::BlockId blockId;
};

static bool IsWater(mcfile::blocks::BlockId id) {
  return id == water || id == bubble_column || id == kelp || id == kelp_plant || id == seagrass || id == tall_seagrass;
}

} // namespace

namespace mcview {
std::map<mcfile::blocks::BlockId, Colour> const RegionToTexture::kBlockToColor{
    {stone, kColorStone},
    {granite, kColorStoneGranite},
    {diorite, kColorStoneDiorite},
    {andesite, kColorStoneAndesite},
    {chest, kColorChest},
    {clay, Colour(162, 166, 182)},
    {coal_ore, Colour(111, 111, 111)},
    {cobblestone, Colour(111, 111, 111)},
    {dirt, Colour(149, 108, 76)},
    {brown_mushroom, Colour(0, 123, 0)},
    {grass_block, Colour(130, 148, 58)},
    {iron_ore, Colour(111, 111, 111)},
    {sand, kColorSand},                  //
    {oak_leaves, Colour(56, 95, 31)},    //
    {jungle_leaves, Colour(56, 95, 31)}, //
    {birch_leaves, Colour(67, 124, 37)},
    {red_mushroom, Colour(0, 123, 0)},
    {mossy_cobblestone, kColorMossyStone},
    {oak_stairs, kColorPlanksOak},
    {gravel, kColorGravel},
    {oak_log, kColorLogOak},
    {oak_planks, kColorPlanksOak},
    {farmland, Colour(149, 108, 76)},
    {oak_fence, kColorPlanksOak},
    {cobblestone_stairs, Colour(111, 111, 111)},
    {dirt_path, Colour(204, 204, 204)}, //
    {birch_fence, kColorPlanksBirch},
    {birch_planks, kColorPlanksBirch},
    {birch_stairs, kColorPlanksBirch},
    {dark_oak_fence, kColorPlanksDarkOak},
    {dark_oak_log, Colour(101, 75, 50)},
    {dark_oak_planks, kColorPlanksDarkOak},
    {dark_oak_slab, kColorPlanksDarkOak},
    {dark_oak_stairs, kColorPlanksDarkOak},
    {dark_oak_trapdoor, Colour(141, 118, 71)},
    {diamond_ore, Colour(111, 111, 111)},
    {gold_ore, Colour(111, 111, 111)},
    {ice, Colour(158, 158, 252)},
    {jungle_fence, kColorPlanksJungle},
    {jungle_log, Colour(149, 108, 76)},
    {jungle_planks, kColorPlanksJungle},
    {jungle_slab, kColorPlanksJungle},
    {jungle_stairs, kColorPlanksJungle},
    {jungle_button, kColorPlanksJungle},
    {jungle_door, kColorPlanksJungle},
    {jungle_trapdoor, Colour(141, 118, 71)},
    {lapis_ore, Colour(111, 111, 111)},
    {lava, Colour(179, 71, 3)},
    {oak_door, Colour(141, 118, 71)},
    {oak_slab, kColorPlanksOak},
    {oak_trapdoor, Colour(141, 118, 71)},
    {obsidian, Colour(29, 14, 52)},
    {packed_ice, Colour(158, 158, 252)},
    {polished_granite, kColorStoneGranite},
    {prismarine, kColorPrismarine},
    {prismarine_bricks, Colour(91, 216, 210)},
    {rail, kColorRail},
    {redstone_ore, Colour(111, 111, 111)},
    {sandstone, kColorSand},
    {sea_lantern, Colour(252, 249, 242)},
    {snow, Colour(229, 229, 229)}, //
    {snow_block, Colour(252, 252, 252)},
    {powder_snow, Colour(252, 252, 252)},
    {spruce_door, kColorPlanksSpruce},
    {spruce_fence, kColorPlanksSpruce},
    {spruce_leaves, Colour(56, 95, 31)}, //
    {stone_brick_stairs, Colour(111, 111, 111)},
    {stone_bricks, Colour(111, 111, 111)},
    {stone_slab, Colour(111, 111, 111)},
    {spruce_log, kColorLogSpruce},
    {spruce_planks, kColorPlanksSpruce},
    {spruce_slab, kColorPlanksSpruce},
    {spruce_stairs, kColorPlanksSpruce},
    {spruce_trapdoor, kColorPlanksSpruce},
    {mossy_stone_bricks, kColorMossyStone},
    {chiseled_stone_bricks, kColorStone},
    {cracked_stone_bricks, kColorStone},
    {infested_stone, Colour(111, 111, 111)},
    {cobweb, Colour(255, 255, 255)},
    {blue_ice, Colour(102, 151, 246)},
    {magma_block, Colour(181, 64, 9)},
    {end_stone, Colour(219, 219, 172)},
    {end_portal, Colour(4, 18, 24)},
    {end_portal_frame, Colour(65, 114, 102)},
    {bedrock, Colour(111, 111, 111)},
    {mycelium, Colour(114, 96, 97)},
    {white_terracotta, Colour(209, 180, 161)},
    {orange_terracotta, Colour(165, 82, 40)},
    {magenta_terracotta, Colour(147, 87, 108)},
    {light_blue_terracotta, Colour(110, 106, 135)},
    {yellow_terracotta, Colour(184, 129, 33)},
    {lime_terracotta, Colour(102, 116, 52)},
    {pink_terracotta, Colour(160, 77, 78)},
    {gray_terracotta, Colour(57, 41, 36)},
    {light_gray_terracotta, Colour(137, 106, 99)},
    {cyan_terracotta, Colour(90, 94, 93)},
    {purple_terracotta, Colour(117, 69, 86)},
    {blue_terracotta, Colour(73, 58, 90)},
    {brown_terracotta, Colour(76, 50, 36)},
    {green_terracotta, Colour(75, 82, 41)},
    {red_terracotta, Colour(139, 58, 45)},
    {black_terracotta, Colour(37, 22, 15)},
    {terracotta, Colour(153, 95, 68)},
    {red_sand, Colour(201, 109, 36)},
    {purpur_slab, kColorPurPur},
    {purpur_block, kColorPurPur},
    {purpur_pillar, kColorPurPur},
    {ender_chest, Colour(39, 54, 56)},
    {tnt, Colour(216, 46, 26)},
    {prismarine_slab, kColorPrismarine},
    {prismarine_stairs, kColorPrismarine},
    {prismarine_bricks, Colour(89, 173, 162)},
    {prismarine_brick_slab, Colour(89, 173, 162)},
    {prismarine_brick_stairs, Colour(89, 173, 162)},
    {dark_prismarine, Colour(55, 97, 80)},
    {dark_prismarine_slab, Colour(55, 97, 80)},
    {dark_prismarine_stairs, Colour(55, 97, 80)},
    {netherrack, Colour(86, 32, 31)},
    {nether_bricks, kColorNetherBricks},
    {nether_brick_slab, kColorNetherBricks},
    {nether_brick_wall, kColorNetherBricks},
    {red_nether_bricks, kColorRedNetherBricks},
    {red_nether_brick_slab, kColorRedNetherBricks},
    {red_nether_brick_wall, kColorRedNetherBricks},
    {glowstone, Colour(248, 215, 115)},
    {nether_quartz_ore, Colour(170, 112, 105)},
    {soul_sand, Colour(72, 54, 43)},
    {white_wool, Colour(247, 247, 247)},
    {orange_wool, Colour(244, 122, 25)},
    {magenta_wool, Colour(193, 73, 183)},
    {light_blue_wool, Colour(65, 186, 220)},
    {yellow_wool, Colour(249, 206, 47)},
    {lime_wool, Colour(123, 193, 27)},
    {pink_wool, Colour(241, 160, 186)},
    {gray_wool, Colour(70, 78, 81)},
    {light_gray_wool, Colour(151, 151, 145)},
    {cyan_wool, Colour(22, 153, 154)},
    {purple_wool, Colour(132, 47, 179)},
    {blue_wool, Colour(57, 63, 164)},
    {brown_wool, Colour(125, 79, 46)},
    {green_wool, Colour(91, 119, 22)},
    {red_wool, Colour(170, 42, 36)},
    {black_wool, Colour(28, 28, 32)},
    {white_carpet, Colour(247, 247, 247)},
    {orange_carpet, Colour(244, 122, 25)},
    {magenta_carpet, Colour(193, 73, 183)},
    {light_blue_carpet, Colour(65, 186, 220)},
    {yellow_carpet, Colour(249, 206, 47)},
    {lime_carpet, Colour(123, 193, 27)},
    {pink_carpet, Colour(241, 160, 186)},
    {gray_carpet, Colour(70, 78, 81)},
    {light_gray_carpet, Colour(151, 151, 145)},
    {cyan_carpet, Colour(22, 153, 154)},
    {purple_carpet, Colour(132, 47, 179)},
    {blue_carpet, Colour(57, 63, 164)},
    {brown_carpet, Colour(125, 79, 46)},
    {green_carpet, Colour(91, 119, 22)},
    {red_carpet, Colour(170, 42, 36)},
    {black_carpet, Colour(28, 28, 32)},
    {white_bed, Colour(247, 247, 247)},
    {orange_bed, Colour(244, 122, 25)},
    {magenta_bed, Colour(193, 73, 183)},
    {light_blue_bed, Colour(65, 186, 220)},
    {yellow_bed, Colour(249, 206, 47)},
    {lime_bed, Colour(123, 193, 27)},
    {pink_bed, Colour(241, 160, 186)},
    {gray_bed, Colour(70, 78, 81)},
    {light_gray_bed, Colour(151, 151, 145)},
    {cyan_bed, Colour(22, 153, 154)},
    {purple_bed, Colour(132, 47, 179)},
    {blue_bed, Colour(57, 63, 164)},
    {brown_bed, Colour(125, 79, 46)},
    {green_bed, Colour(91, 119, 22)},
    {red_bed, Colour(170, 42, 36)},
    {black_bed, Colour(28, 28, 32)},
    {dried_kelp_block, Colour(43, 55, 32)},
    {beacon, Colour(72, 210, 202)},
    {shulker_box, Colour(149, 101, 149)},
    {white_shulker_box, Colour(225, 230, 230)},
    {orange_shulker_box, Colour(225, 230, 230)},
    {magenta_shulker_box, Colour(183, 61, 172)},
    {light_blue_shulker_box, Colour(57, 177, 215)},
    {yellow_shulker_box, Colour(249, 194, 34)},
    {lime_shulker_box, Colour(108, 183, 24)},
    {pink_shulker_box, Colour(239, 135, 166)},
    {gray_shulker_box, Colour(59, 63, 67)},
    {light_gray_shulker_box, Colour(135, 135, 126)},
    {cyan_shulker_box, Colour(22, 133, 144)},
    {purple_shulker_box, Colour(115, 38, 167)},
    {blue_shulker_box, Colour(49, 52, 152)},
    {brown_shulker_box, Colour(111, 69, 39)},
    {green_shulker_box, Colour(83, 107, 29)},
    {red_shulker_box, Colour(152, 35, 33)},
    {black_shulker_box, Colour(31, 31, 34)},
    {bricks, kColorBricks},
    {cut_sandstone, kColorSand},
    {sandstone_stairs, kColorSand},
    {chiseled_sandstone, kColorSand},
    {sandstone_slab, kColorSand},
    {dark_oak_door, kColorPlanksDarkOak},
    {polished_diorite, kColorStoneDiorite},
    {coarse_dirt, Colour(96, 67, 45)},
    {acacia_log, Colour(104, 97, 88)},
    {oak_pressure_plate, kColorPlanksOak},
    {fire, Colour(202, 115, 3)},
    {cobblestone_wall, Colour(111, 111, 111)},
    {cobblestone_slab, Colour(111, 111, 111)},
    {podzol, Colour(105, 67, 23)},
    {sticky_piston, Colour(131, 131, 131)},
    {piston_head, kColorPistonHead},
    {piston, Colour(131, 131, 131)},
    {lever, Colour(134, 133, 134)},
    {observer, Colour(100, 100, 100)},
    {slime_block, Colour(112, 187, 94)},
    {activator_rail, kColorRail},
    {oak_fence_gate, kColorPlanksOak},
    {dark_oak_fence_gate, kColorPlanksDarkOak},
    {birch_button, kColorPlanksBirch},
    {birch_slab, kColorPlanksBirch},
    {acacia_stairs, kColorPlanksAcacia},
    {acacia_pressure_plate, kColorPlanksAcacia},
    {redstone_wire, Colour(75, 1, 0)},
    {redstone_block, Colour(162, 24, 8)},
    {redstone_lamp, Colour(173, 104, 58)},
    {hopper, Colour(70, 70, 70)},
    {crafting_table, Colour(156, 88, 49)},
    {lectern, Colour(164, 128, 73)},
    {acacia_planks, kColorPlanksAcacia},
    {acacia_wood, kColorPlanksAcacia},
    {acacia_slab, kColorPlanksAcacia},
    {acacia_fence, kColorPlanksAcacia},
    {acacia_fence_gate, kColorPlanksAcacia},
    {acacia_door, kColorPlanksAcacia},
    {stripped_acacia_log, kColorPlanksAcacia},
    {brain_coral, Colour(225, 125, 183)},
    {brain_coral_block, Colour(225, 125, 183)},
    {brain_coral_fan, Colour(225, 125, 183)},
    {brain_coral_wall_fan, Colour(225, 125, 183)},
    {bubble_coral, Colour(198, 25, 184)},
    {bubble_coral_block, Colour(198, 25, 184)},
    {bubble_coral_fan, Colour(198, 25, 184)},
    {bubble_coral_wall_fan, Colour(198, 25, 184)},
    {horn_coral, Colour(234, 233, 76)},
    {horn_coral_block, Colour(234, 233, 76)},
    {horn_coral_fan, Colour(234, 233, 76)},
    {horn_coral_wall_fan, Colour(234, 233, 76)},
    {tube_coral, Colour(48, 78, 218)},
    {tube_coral_block, Colour(48, 78, 218)},
    {tube_coral_fan, Colour(48, 78, 218)},
    {tube_coral_wall_fan, Colour(48, 78, 218)},
    {fire_coral, Colour(196, 42, 54)},
    {fire_coral_block, Colour(196, 42, 54)},
    {fire_coral_fan, Colour(196, 42, 54)},
    {fire_coral_wall_fan, Colour(196, 42, 54)},
    {smooth_sandstone, kColorSand},
    {smooth_sandstone_slab, kColorSand},
    {smooth_sandstone_stairs, kColorSand},
    {sandstone_wall, kColorSand},
    {polished_andesite, kColorStoneAndesite},
    {carved_pumpkin, Colour(213, 125, 50)},
    {stripped_oak_wood, Colour(127, 85, 48)},
    {stonecutter, Colour(131, 131, 131)},
    {smoker, Colour(131, 131, 131)},
    {hay_block, Colour(203, 176, 7)},
    {birch_log, Colour(252, 252, 252)},
    {iron_trapdoor, Colour(227, 227, 227)},
    {bell, Colour(250, 211, 56)},
    {white_glazed_terracotta, Colour(246, 252, 251)},
    {orange_glazed_terracotta, Colour(26, 196, 197)},
    {magenta_glazed_terracotta, Colour(201, 87, 192)},
    {light_blue_glazed_terracotta, Colour(86, 187, 220)},
    {yellow_glazed_terracotta, Colour(251, 219, 93)},
    {lime_glazed_terracotta, Colour(137, 214, 35)},
    {pink_glazed_terracotta, Colour(241, 179, 201)},
    {gray_glazed_terracotta, Colour(94, 114, 118)},
    {light_gray_glazed_terracotta, Colour(199, 203, 207)},
    {cyan_glazed_terracotta, Colour(20, 159, 160)},
    {purple_glazed_terracotta, Colour(146, 53, 198)},
    {blue_glazed_terracotta, Colour(59, 66, 167)},
    {brown_glazed_terracotta, Colour(167, 120, 79)},
    {green_glazed_terracotta, Colour(111, 151, 36)},
    {furnace, kColorFurnace},
    {composter, Colour(139, 91, 49)},
    {campfire, Colour(199, 107, 3)},
    {cartography_table, Colour(86, 53, 24)},
    {brewing_stand, Colour(47, 47, 47)},
    {grindstone, Colour(141, 141, 141)},
    {fletching_table, Colour(212, 191, 131)},
    {iron_bars, Colour(154, 154, 154)},
    {bookshelf, kColorBookshelf},
    {acacia_sapling, Colour(125, 150, 33)},
    {potted_dead_bush, kColorPotter},
    {potted_cactus, kColorPotter},
    {jack_o_lantern, Colour(213, 125, 50)},
    {acacia_button, kColorPlanksAcacia},
    {acacia_sign, kColorPlanksAcacia},
    {acacia_trapdoor, kColorPlanksAcacia},
    {acacia_wall_sign, kColorPlanksAcacia},
    {andesite_slab, kColorStoneAndesite},
    {andesite_stairs, kColorStoneAndesite},
    {andesite_wall, kColorStoneAndesite},
    {anvil, kColorAnvil},
    {attached_melon_stem, Colour(203, 196, 187)},
    {bamboo_sapling, Colour(67, 103, 8)},
    {barrel, Colour(137, 102, 60)},
    {birch_door, kColorPlanksBirch},
    {birch_fence_gate, kColorPlanksBirch},
    {birch_pressure_plate, kColorPlanksBirch},
    {birch_sapling, Colour(107, 156, 55)},
    {birch_sign, kColorPlanksBirch},
    {birch_trapdoor, kColorPlanksBirch},
    {birch_wall_sign, kColorPlanksBirch},
    {birch_wood, Colour(252, 252, 252)},
    {black_concrete, Colour(8, 10, 15)},
    {black_concrete_powder, Colour(22, 24, 29)},
    {black_glazed_terracotta, Colour(24, 24, 27)},
    {blast_furnace, Colour(131, 131, 131)},
    {coal_block, Colour(13, 13, 13)},
    {diamond_block, Colour(100, 242, 224)},
    {emerald_block, Colour(62, 240, 130)},
    {gold_block, Colour(251, 221, 72)},
    {iron_block, Colour(227, 227, 227)},
    {iron_door, Colour(227, 227, 227)},
    {potted_acacia_sapling, kColorPotter},
    {potted_allium, kColorPotter},
    {potted_azure_bluet, kColorPotter},
    {potted_bamboo, kColorPotter},
    {potted_birch_sapling, kColorPotter},
    {potted_blue_orchid, kColorPotter},
    {potted_brown_mushroom, kColorPotter},
    {potted_cornflower, kColorPotter},
    {potted_dandelion, kColorPotter},
    {potted_dark_oak_sapling, kColorPotter},
    {potted_fern, kColorPotter},
    {potted_jungle_sapling, kColorPotter},
    {potted_lily_of_the_valley, kColorPotter},
    {potted_oak_sapling, kColorPotter},
    {potted_orange_tulip, kColorPotter},
    {potted_oxeye_daisy, kColorPotter},
    {potted_pink_tulip, kColorPotter},
    {potted_poppy, kColorPotter},
    {potted_red_mushroom, kColorPotter},
    {potted_red_tulip, kColorPotter},
    {potted_spruce_sapling, kColorPotter},
    {potted_white_tulip, kColorPotter},
    {potted_wither_rose, kColorPotter},
    {dark_oak_button, kColorPlanksDarkOak},
    {dark_oak_pressure_plate, kColorPlanksDarkOak},
    {dark_oak_sign, kColorPlanksDarkOak},
    {dark_oak_wall_sign, kColorPlanksDarkOak},
    {oak_button, kColorPlanksOak},
    {oak_sign, kColorPlanksOak},
    {oak_wall_sign, kColorPlanksOak},
    {brick_slab, kColorBricks},
    {brick_stairs, kColorBricks},
    {brick_wall, kColorBricks},
    {chipped_anvil, kColorAnvil},
    {damaged_anvil, kColorAnvil},
    {daylight_detector, Colour(188, 168, 140)},
    {dead_brain_coral, kColorDeadCoral},
    {dead_brain_coral_block, kColorDeadCoral},
    {dead_brain_coral_fan, kColorDeadCoral},
    {dead_brain_coral_wall_fan, kColorDeadCoral},
    {dead_bubble_coral, kColorDeadCoral},
    {dead_bubble_coral_block, kColorDeadCoral},
    {dead_bubble_coral_fan, kColorDeadCoral},
    {dead_bubble_coral_wall_fan, kColorDeadCoral},
    {dead_fire_coral, kColorDeadCoral},
    {dead_fire_coral_block, kColorDeadCoral},
    {dead_fire_coral_fan, kColorDeadCoral},
    {dead_fire_coral_wall_fan, kColorDeadCoral},
    {dead_horn_coral, kColorDeadCoral},
    {dead_horn_coral_block, kColorDeadCoral},
    {dead_horn_coral_fan, kColorDeadCoral},
    {dead_horn_coral_wall_fan, kColorDeadCoral},
    {dead_tube_coral, kColorDeadCoral},
    {dead_tube_coral_block, kColorDeadCoral},
    {dead_tube_coral_fan, kColorDeadCoral},
    {dead_tube_coral_wall_fan, kColorDeadCoral},
    {detector_rail, kColorRail},
    {powered_rail, kColorRail},
    {diorite_slab, kColorStoneDiorite},
    {diorite_stairs, kColorStoneDiorite},
    {diorite_wall, kColorStoneDiorite},
    {polished_diorite_slab, kColorStoneDiorite},
    {polished_diorite_stairs, kColorStoneDiorite},
    {granite_slab, kColorStoneGranite},
    {granite_stairs, kColorStoneGranite},
    {granite_wall, kColorStoneGranite},
    {polished_granite_slab, kColorStoneGranite},
    {polished_granite_stairs, kColorStoneGranite},
    {jungle_fence_gate, kColorPlanksJungle},
    {jungle_pressure_plate, kColorPlanksJungle},
    {jungle_sign, kColorPlanksJungle},
    {jungle_wall_sign, kColorPlanksJungle},
    {nether_brick_fence, kColorNetherBricks},
    {nether_brick_stairs, kColorNetherBricks},
    {stone_button, kColorStone},
    {stone_pressure_plate, kColorStone},
    {stone_stairs, kColorStone},
    {spruce_button, kColorPlanksSpruce},
    {spruce_fence_gate, kColorPlanksSpruce},
    {spruce_pressure_plate, kColorPlanksSpruce},
    {spruce_sign, kColorPlanksSpruce},
    {spruce_wall_sign, kColorPlanksSpruce},
    {dispenser, kColorFurnace},
    {dropper, kColorFurnace},
    {quartz_block, kColorQuartz},
    {blue_concrete_powder, Colour(72, 75, 175)},
    {brown_concrete_powder, Colour(120, 81, 50)},
    {cyan_concrete_powder, Colour(37, 154, 160)},
    {gray_concrete_powder, Colour(75, 79, 82)},
    {green_concrete_powder, Colour(103, 126, 37)},
    {light_blue_concrete_powder, Colour(91, 194, 216)},
    {light_gray_concrete_powder, Colour(154, 154, 148)},
    {lime_concrete_powder, Colour(138, 197, 45)},
    {magenta_concrete_powder, Colour(200, 93, 193)},
    {orange_concrete_powder, Colour(230, 128, 20)},
    {pink_concrete_powder, Colour(236, 172, 195)},
    {purple_concrete_powder, Colour(138, 58, 186)},
    {red_concrete_powder, Colour(180, 58, 55)},
    {white_concrete_powder, Colour(222, 223, 224)},
    {yellow_concrete_powder, Colour(235, 209, 64)},
    {end_stone_brick_slab, kColorEndStoneBricks},
    {end_stone_brick_stairs, kColorEndStoneBricks},
    {end_stone_brick_wall, kColorEndStoneBricks},
    {end_stone_bricks, kColorEndStoneBricks},
    {blue_concrete, Colour(44, 46, 142)},
    {bone_block, Colour(199, 195, 165)},
    {brown_concrete, Colour(95, 58, 31)},
    {cake, Colour(238, 229, 203)},
    {candle_cake, Colour(238, 229, 203)},
    {white_candle_cake, Colour(238, 229, 203)},
    {orange_candle_cake, Colour(238, 229, 203)},
    {magenta_candle_cake, Colour(238, 229, 203)},
    {light_blue_candle_cake, Colour(238, 229, 203)},
    {yellow_candle_cake, Colour(238, 229, 203)},
    {lime_candle_cake, Colour(238, 229, 203)},
    {pink_candle_cake, Colour(238, 229, 203)},
    {gray_candle_cake, Colour(238, 229, 203)},
    {light_gray_candle_cake, Colour(238, 229, 203)},
    {cyan_candle_cake, Colour(238, 229, 203)},
    {purple_candle_cake, Colour(238, 229, 203)},
    {blue_candle_cake, Colour(238, 229, 203)},
    {brown_candle_cake, Colour(238, 229, 203)},
    {green_candle_cake, Colour(238, 229, 203)},
    {red_candle_cake, Colour(238, 229, 203)},
    {black_candle_cake, Colour(238, 229, 203)},
    {chain_command_block, Colour(159, 193, 178)},
    {chiseled_quartz_block, kColorQuartz},
    {chiseled_red_sandstone, kColorRedSandstone},
    {command_block, Colour(196, 125, 78)},
    {conduit, Colour(126, 113, 81)},
    {cut_red_sandstone, kColorRedSandstone},
    {cut_red_sandstone_slab, kColorRedSandstone},
    {red_sandstone, kColorRedSandstone},
    {red_sandstone_slab, kColorRedSandstone},
    {red_sandstone_stairs, kColorRedSandstone},
    {red_sandstone_wall, kColorRedSandstone},
    {smooth_red_sandstone, kColorRedSandstone},
    {smooth_red_sandstone_slab, kColorRedSandstone},
    {smooth_red_sandstone_stairs, kColorRedSandstone},
    {cut_sandstone_slab, kColorSand},
    {cyan_concrete, Colour(21, 118, 134)},
    {dark_oak_sapling, Colour(31, 100, 25)},
    {dark_oak_wood, Colour(62, 48, 29)},
    {dragon_egg, Colour(9, 9, 9)},
    {dragon_head, kColorDragonHead},
    {dragon_wall_head, kColorDragonHead},
    {quartz_pillar, kColorQuartz},
    {quartz_slab, kColorQuartz},
    {quartz_stairs, kColorQuartz},
    {emerald_ore, kColorStone},
    {polished_andesite_slab, kColorStoneAndesite},
    {polished_andesite_stairs, kColorStoneAndesite},
    {mossy_cobblestone_slab, kColorMossyStone},
    {mossy_cobblestone_stairs, kColorMossyStone},
    {mossy_cobblestone_wall, kColorMossyStone},
    {infested_cobblestone, kColorStone},
    {infested_mossy_stone_bricks, kColorMossyStone},
    {mossy_stone_brick_slab, kColorMossyStone},
    {mossy_stone_brick_stairs, kColorMossyStone},
    {mossy_stone_brick_wall, kColorMossyStone},
    {infested_chiseled_stone_bricks, kColorStone},
    {infested_cracked_stone_bricks, kColorStone},
    {infested_stone_bricks, kColorStone},
    {moving_piston, kColorPistonHead},
    {smooth_quartz, kColorQuartz},
    {smooth_quartz_slab, kColorQuartz},
    {smooth_quartz_stairs, kColorQuartz},
    {stone_brick_slab, kColorStone},
    {stone_brick_wall, kColorStone},
    {purpur_stairs, kColorPurPur},
    {prismarine_wall, kColorPrismarine},
    {red_nether_brick_stairs, kColorRedNetherBricks},
    {creeper_head, kColorCreaperHead},
    {creeper_wall_head, kColorCreaperHead},
    {enchanting_table, Colour(73, 234, 207)},
    {end_gateway, Colour(3, 13, 20)},
    {gray_concrete, Colour(53, 57, 61)},
    {green_concrete, Colour(72, 90, 35)},
    {heavy_weighted_pressure_plate, Colour(182, 182, 182)},
    {jigsaw, Colour(147, 120, 148)},
    {jukebox, Colour(122, 79, 56)},
    {jungle_sapling, Colour(41, 73, 12)},
    {jungle_wood, Colour(88, 69, 26)},
    {lantern, Colour(72, 79, 100)},
    {lapis_block, Colour(24, 59, 115)},
    {light_blue_concrete, Colour(37, 136, 198)},
    {light_gray_concrete, Colour(125, 125, 115)},
    {light_weighted_pressure_plate, Colour(202, 171, 50)},
    {lime_concrete, Colour(93, 167, 24)},
    {loom, Colour(200, 164, 112)},
    {magenta_concrete, Colour(168, 49, 158)},
    {nether_wart_block, Colour(122, 1, 0)},
    {note_block, Colour(146, 92, 64)},
    {oak_sapling, Colour(63, 141, 46)},
    {oak_wood, kColorLogOak},
    {orange_concrete, Colour(222, 97, 0)},
    {petrified_oak_slab, kColorPlanksOak},
    {pink_concrete, Colour(210, 100, 141)},
    {player_head, kColorPlayerHead},
    {player_wall_head, kColorPlayerHead},
    {purple_concrete, Colour(99, 32, 154)},
    {red_concrete, Colour(138, 32, 32)},
    {red_glazed_terracotta, Colour(202, 65, 57)},
    {comparator, Colour(185, 185, 185)},
    {repeater, Colour(185, 185, 185)},
    {repeating_command_block, Colour(105, 78, 197)},
    {scaffolding, Colour(225, 196, 115)},
    {skeleton_skull, kColorSkeltonSkull},
    {skeleton_wall_skull, kColorSkeltonSkull},
    {smithing_table, Colour(63, 65, 82)},
    {spawner, Colour(24, 43, 56)},
    {sponge, Colour(203, 204, 73)},
    {spruce_sapling, Colour(34, 52, 34)},
    {spruce_wood, kColorLogSpruce},
    {stripped_acacia_wood, Colour(185, 94, 61)},
    {stripped_birch_log, Colour(205, 186, 126)},
    {stripped_birch_wood, Colour(205, 186, 126)},
    {stripped_dark_oak_log, Colour(107, 83, 51)},
    {stripped_dark_oak_wood, Colour(107, 83, 51)},
    {stripped_jungle_log, Colour(173, 126, 82)},
    {stripped_jungle_wood, Colour(173, 126, 82)},
    {stripped_oak_log, Colour(148, 115, 64)},
    {stripped_spruce_log, Colour(120, 90, 54)},
    {stripped_spruce_wood, Colour(120, 90, 54)},
    {structure_block, Colour(147, 120, 148)},
    {trapped_chest, kColorChest},
    {tripwire_hook, Colour(135, 135, 135)},
    {turtle_egg, Colour(224, 219, 197)},
    {wet_sponge, Colour(174, 189, 74)},
    {white_concrete, Colour(204, 209, 210)},
    {wither_rose, Colour(23, 18, 16)},
    {wither_skeleton_skull, kColorWitherSkeltonSkull},
    {wither_skeleton_wall_skull, kColorWitherSkeltonSkull},
    {yellow_concrete, Colour(239, 175, 22)},
    {zombie_head, kColorZombieHead},
    {zombie_wall_head, kColorZombieHead},
    {end_rod, Colour(202, 202, 202)},
    {flower_pot, kColorPotter},
    {frosted_ice, Colour(109, 146, 193)},
    {nether_portal, Colour(78, 30, 135)},

    // plants
    {lily_pad, Colour(0, 123, 0)},
    {wheat, Colour(0, 123, 0)},
    {melon, Colour(125, 202, 25)},
    {pumpkin, Colour(213, 125, 50)},
    {grass, Colour(109, 141, 35)},
    {tall_grass, Colour(109, 141, 35)},
    {dandelion, Colour(245, 238, 50)},
    {poppy, Colour(229, 31, 29)},
    {peony, Colour(232, 143, 213)},
    {pink_tulip, Colour(234, 182, 209)},
    {orange_tulip, Colour(242, 118, 33)},
    {lilac, Colour(212, 119, 197)},
    {sunflower, Colour(245, 238, 50)},
    {allium, Colour(200, 109, 241)},
    {red_tulip, Colour(229, 31, 29)},
    {white_tulip, Colour(255, 255, 255)},
    {rose_bush, Colour(136, 40, 27)},
    {blue_orchid, Colour(47, 181, 199)},
    {oxeye_daisy, Colour(236, 246, 247)},
    {sugar_cane, Colour(165, 214, 90)},
    {chorus_plant, Colour(90, 51, 90)},
    {chorus_flower, Colour(159, 119, 159)},
    {dark_oak_leaves, Colour(58, 82, 23)},
    {red_mushroom_block, Colour(199, 42, 41)},
    {mushroom_stem, Colour(203, 196, 187)},
    {brown_mushroom_block, Colour(149, 113, 80)},
    {acacia_leaves, Colour(63, 89, 25)},
    {dead_bush, Colour(146, 99, 40)},
    {cactus, Colour(90, 138, 42)},
    {sweet_berry_bush, Colour(40, 97, 63)},
    {cornflower, Colour(69, 105, 232)},
    {pumpkin_stem, Colour(72, 65, 9)},
    {nether_wart, Colour(163, 35, 41)},
    {attached_pumpkin_stem, Colour(72, 65, 9)},
    {lily_of_the_valley, Colour(252, 252, 252)},
    {melon_stem, Colour(72, 65, 9)},
    {smooth_stone, Colour(111, 111, 111)},
    {smooth_stone_slab, Colour(111, 111, 111)},
    {bamboo, Colour(67, 103, 8)},
    {sea_pickle, Colour(106, 113, 42)},
    {cocoa, Colour(109, 112, 52)},

    // 1.15

    {bee_nest, Colour(198, 132, 67)},
    {beehive, Colour(182, 146, 94)},
    {honey_block, Colour(233, 145, 38)},
    {honeycomb_block, Colour(229, 138, 8)},

    // 1.16

    {crimson_nylium, Colour(146, 24, 24)},
    {warped_nylium, Colour(22, 125, 132)},
    {crimson_planks, kColorPlanksCrimson},
    {warped_planks, kColorPlanksWarped},
    {nether_gold_ore, Colour(245, 173, 42)},
    {crimson_stem, kColorPlanksCrimson},
    {warped_stem, kColorPlanksWarped},
    {stripped_crimson_stem, Colour(148, 61, 97)},
    {stripped_warped_stem, Colour(67, 159, 157)},
    {crimson_hyphae, Colour(148, 21, 21)},
    {warped_hyphae, Colour(22, 96, 90)},
    {crimson_slab, kColorPlanksCrimson},
    {warped_slab, kColorPlanksWarped},
    {cracked_nether_bricks, kColorNetherBricks},
    {chiseled_nether_bricks, kColorNetherBricks},
    {crimson_stairs, kColorPlanksCrimson},
    {warped_stairs, kColorPlanksWarped},
    {netherite_block, Colour(76, 72, 76)},
    {soul_soil, Colour(90, 68, 55)},
    {basalt, Colour(91, 91, 91)},
    {polished_basalt, Colour(115, 115, 115)},
    {smooth_basalt, Colour(91, 91, 91)},
    {ancient_debris, Colour(125, 95, 88)},
    {crying_obsidian, Colour(42, 1, 119)},
    {blackstone, kColorStoneBlack},
    {blackstone_slab, kColorStoneBlack},
    {blackstone_stairs, kColorStoneBlack},
    {gilded_blackstone, Colour(125, 68, 14)},
    {polished_blackstone, kColorStonePolishedBlack},
    {polished_blackstone_slab, kColorStonePolishedBlack},
    {polished_blackstone_stairs, kColorStonePolishedBlack},
    {chiseled_polished_blackstone, kColorStonePolishedBlack},
    {polished_blackstone_bricks, kColorPolishedBlackStoneBricks},
    {polished_blackstone_brick_slab, kColorPolishedBlackStoneBricks},
    {polished_blackstone_brick_stairs, kColorPolishedBlackStoneBricks},
    {cracked_polished_blackstone_bricks, kColorPolishedBlackStoneBricks},
    {crimson_fungus, Colour(162, 36, 40)},
    {warped_fungus, Colour(20, 178, 131)},
    {crimson_roots, Colour(171, 16, 28)},
    {warped_roots, Colour(20, 178, 131)},
    {nether_sprouts, Colour(20, 178, 131)},
    {weeping_vines, Colour(171, 16, 28)},
    {weeping_vines_plant, Colour(171, 16, 28)},
    {twisting_vines, Colour(20, 178, 131)},
    {crimson_fence, kColorPlanksCrimson},
    {warped_fence, kColorPlanksWarped},
    {soul_torch, Colour(123, 239, 242)},
    {chain, Colour(60, 65, 80)},
    {blackstone_wall, kColorStoneBlack},
    {polished_blackstone_wall, kColorStonePolishedBlack},
    {polished_blackstone_brick_wall, kColorPolishedBlackStoneBricks},
    {soul_lantern, Colour(123, 239, 242)},
    {soul_campfire, Colour(123, 239, 242)},
    {soul_fire, Colour(123, 239, 242)},
    {soul_wall_torch, Colour(123, 239, 242)},
    {shroomlight, Colour(251, 170, 108)},
    {lodestone, Colour(160, 162, 170)},
    {respawn_anchor, Colour(129, 8, 225)},
    {crimson_pressure_plate, kColorPlanksCrimson},
    {warped_pressure_plate, kColorPlanksWarped},
    {crimson_trapdoor, kColorPlanksCrimson},
    {warped_trapdoor, kColorPlanksWarped},
    {crimson_fence_gate, kColorPlanksCrimson},
    {warped_fence_gate, kColorPlanksWarped},
    {crimson_button, kColorPlanksCrimson},
    {warped_button, kColorPlanksWarped},
    {crimson_door, kColorPlanksCrimson},
    {warped_door, kColorPlanksWarped},
    {target, Colour(183, 49, 49)},

    // bugfix for 1.16
    {twisting_vines_plant, Colour(17, 153, 131)},
    {warped_wart_block, Colour(17, 153, 131)},
    {quartz_bricks, kColorQuartz},
    {stripped_crimson_hyphae, Colour(148, 61, 97)},
    {stripped_warped_hyphae, Colour(67, 159, 157)},
    {crimson_sign, kColorPlanksCrimson},
    {warped_sign, kColorPlanksWarped},
    {polished_blackstone_pressure_plate, kColorStonePolishedBlack},
    {polished_blackstone_button, kColorStonePolishedBlack},

    {crimson_wall_sign, kColorPlanksCrimson},
    {warped_wall_sign, kColorPlanksWarped},

    {deepslate, kColorDeepslate},
    {cobbled_deepslate, kColorDeepslate},
    {polished_deepslate, kColorDeepslate},
    {deepslate_coal_ore, kColorDeepslate},
    {deepslate_iron_ore, kColorDeepslate},
    {deepslate_copper_ore, kColorDeepslate},
    {deepslate_gold_ore, kColorDeepslate},
    {deepslate_redstone_ore, kColorDeepslate},
    {deepslate_emerald_ore, kColorDeepslate},
    {deepslate_lapis_ore, kColorDeepslate},
    {deepslate_diamond_ore, kColorDeepslate},
    {infested_deepslate, kColorDeepslate},
    {deepslate_bricks, kColorDeepslate},
    {cracked_deepslate_bricks, kColorDeepslate},
    {deepslate_tiles, kColorDeepslate},
    {cracked_deepslate_tiles, kColorDeepslate},
    {chiseled_deepslate, kColorDeepslate},
    {cobbled_deepslate_stairs, kColorDeepslate},
    {polished_deepslate_stairs, kColorDeepslate},
    {deepslate_brick_stairs, kColorDeepslate},
    {deepslate_tile_stairs, kColorDeepslate},
    {cobbled_deepslate_slab, kColorDeepslate},
    {polished_deepslate_slab, kColorDeepslate},
    {deepslate_brick_slab, kColorDeepslate},
    {deepslate_tile_slab, kColorDeepslate},
    {cobbled_deepslate_wall, kColorDeepslate},
    {polished_deepslate_wall, kColorDeepslate},
    {deepslate_brick_wall, kColorDeepslate},
    {deepslate_tile_wall, kColorDeepslate},

    {copper_ore, kColorStone},

    {calcite, kColorStoneDiorite},
    {tuff, kColorStone},
    {dripstone_block, Colour(140, 116, 97)},
    {pointed_dripstone, Colour(140, 116, 97)},
    {raw_iron_block, Colour(109, 89, 64)},
    {raw_copper_block, Colour(145, 83, 62)},
    {raw_gold_block, Colour(173, 137, 34)},
    {amethyst_block, Colour(145, 104, 174)},
    {budding_amethyst, Colour(145, 104, 174)},

    {copper_block, kColorCopper},
    {cut_copper, kColorCopper},
    {waxed_copper_block, kColorCopper},
    {waxed_cut_copper, kColorCopper},
    {cut_copper_stairs, kColorCopper},
    {waxed_cut_copper_stairs, kColorCopper},
    {cut_copper_slab, kColorCopper},
    {waxed_cut_copper_slab, kColorCopper},

    {exposed_copper, kColorExposedCopper},
    {exposed_cut_copper, kColorExposedCopper},
    {waxed_exposed_copper, kColorExposedCopper},
    {waxed_exposed_cut_copper, kColorExposedCopper},
    {exposed_cut_copper_stairs, kColorExposedCopper},
    {waxed_exposed_cut_copper_stairs, kColorExposedCopper},
    {exposed_cut_copper_slab, kColorExposedCopper},
    {waxed_exposed_cut_copper_slab, kColorExposedCopper},

    {weathered_copper, kColorWeatheredCopper},
    {weathered_cut_copper, kColorWeatheredCopper},
    {waxed_weathered_copper, kColorWeatheredCopper},
    {waxed_weathered_cut_copper, kColorWeatheredCopper},
    {weathered_cut_copper_stairs, kColorWeatheredCopper},
    {waxed_weathered_cut_copper_stairs, kColorWeatheredCopper},
    {weathered_cut_copper_slab, kColorWeatheredCopper},
    {waxed_weathered_cut_copper_slab, kColorWeatheredCopper},

    {oxidized_copper, kColorOxidizedCopper},
    {oxidized_cut_copper, kColorOxidizedCopper},
    {waxed_oxidized_copper, kColorOxidizedCopper},
    {waxed_oxidized_cut_copper, kColorOxidizedCopper},
    {oxidized_cut_copper_stairs, kColorOxidizedCopper},
    {waxed_oxidized_cut_copper_stairs, kColorOxidizedCopper},
    {oxidized_cut_copper_slab, kColorOxidizedCopper},
    {waxed_oxidized_cut_copper_slab, kColorOxidizedCopper},

    {azalea_leaves, Colour(111, 144, 44)},
    {azalea, Colour(111, 144, 44)},
    {flowering_azalea, Colour(184, 97, 204)},
    {spore_blossom, Colour(184, 97, 204)},
    {moss_carpet, Colour(111, 144, 44)},
    {moss_block, Colour(111, 144, 44)},

    {potted_azalea_bush, kColorPotter},
    {potted_flowering_azalea_bush, kColorPotter},
    {cauldron, Colour(53, 52, 52)},
    {water_cauldron, Colour(53, 52, 52)},
    {lava_cauldron, Colour(53, 52, 52)},
    {powder_snow_cauldron, Colour(53, 52, 52)},
    {rooted_dirt, Colour(149, 108, 76)},
    {flowering_azalea_leaves, Colour(184, 97, 204)},
    {small_amethyst_bud, Colour(145, 104, 174)},
    {medium_amethyst_bud, Colour(145, 104, 174)},
    {large_amethyst_bud, Colour(145, 104, 174)},
    {amethyst_cluster, Colour(145, 104, 174)},
    {cave_vines, Colour(106, 126, 48)},
    {cave_vines_plant, Colour(106, 126, 48)},
    {potted_crimson_fungus, kColorPotter},
    {potted_warped_fungus, kColorPotter},
    {potted_crimson_roots, kColorPotter},
    {potted_warped_roots, kColorPotter},
    {sculk_sensor, kColorSculkSensor},
    {reinforced_deepslate, kColorDeepslate},
    {sculk, kColorSculk},
    {sculk_catalyst, kColorSculk},
    {sculk_shrieker, kColorSculk},
    {mangrove_planks, kColorPlanksManvrove},
    {mangrove_roots, Colour(89, 71, 43)},
    {muddy_mangrove_roots, Colour(57, 55, 60)},
    {mangrove_log, Colour(89, 71, 43)},
    {stripped_mangrove_log, kColorPlanksManvrove},
    {stripped_mangrove_wood, kColorPlanksManvrove},
    {mangrove_wood, Colour(89, 71, 43)},
    {mangrove_slab, kColorPlanksManvrove},
    {mud_brick_slab, kColorMudBrick},
    {packed_mud, kColorMudBrick},
    {mud_bricks, kColorMudBrick},
    {mangrove_fence, kColorPlanksManvrove},
    {mangrove_stairs, kColorPlanksManvrove},
    {mangrove_fence_gate, kColorPlanksManvrove},
    {mud_brick_wall, kColorMudBrick},
    {mangrove_sign, kColorPlanksManvrove},
    {mangrove_wall_sign, kColorPlanksManvrove},
    {ochre_froglight, Colour(252, 249, 242)},
    {verdant_froglight, Colour(252, 249, 242)},
    {pearlescent_froglight, Colour(252, 249, 242)},
    {mangrove_leaves, Colour(59, 73, 16)},
    {mangrove_button, kColorPlanksManvrove},
    {mangrove_pressure_plate, kColorPlanksManvrove},
    {mangrove_door, kColorPlanksManvrove},
    {mangrove_trapdoor, kColorPlanksManvrove},
    {potted_mangrove_propagule, kColorPotter},
    {mud, Colour(57, 55, 60)},

    // 1.20 and bugfix
    {mud_brick_stairs, kColorMudBrick},
    {acacia_hanging_sign, kColorPlanksAcacia},
    {acacia_wall_hanging_sign, kColorPlanksAcacia},
    {bamboo_block, Colour(127, 132, 56)},
    {bamboo_button, kColorPlanksBamboo},
    {bamboo_door, kColorPlanksBamboo},
    {bamboo_fence, kColorPlanksBamboo},
    {bamboo_fence_gate, kColorPlanksBamboo},
    {bamboo_hanging_sign, kColorPlanksBamboo},
    {bamboo_mosaic, kColorPlanksBamboo},
    {bamboo_mosaic_slab, kColorPlanksBamboo},
    {bamboo_mosaic_stairs, kColorPlanksBamboo},
    {bamboo_planks, kColorPlanksBamboo},
    {bamboo_pressure_plate, kColorPlanksBamboo},
    {bamboo_sign, kColorPlanksBamboo},
    {bamboo_slab, kColorPlanksBamboo},
    {bamboo_stairs, kColorPlanksBamboo},
    {bamboo_trapdoor, kColorPlanksBamboo},
    {bamboo_wall_hanging_sign, kColorPlanksBamboo},
    {bamboo_wall_sign, kColorPlanksBamboo},
    {birch_hanging_sign, kColorPlanksBirch},
    {birch_wall_hanging_sign, kColorPlanksBirch},
    {chiseled_bookshelf, kColorBookshelf},
    {crimson_hanging_sign, kColorPlanksCrimson},
    {crimson_wall_hanging_sign, kColorPlanksCrimson},
    {dark_oak_hanging_sign, kColorPlanksDarkOak},
    {dark_oak_wall_hanging_sign, kColorPlanksDarkOak},
    {jungle_hanging_sign, kColorPlanksJungle},
    {jungle_wall_hanging_sign, kColorPlanksJungle},
    {mangrove_hanging_sign, kColorPlanksManvrove},
    {mangrove_wall_hanging_sign, kColorPlanksManvrove},
    {oak_hanging_sign, kColorPlanksOak},
    {oak_wall_hanging_sign, kColorPlanksOak},
    {piglin_head, kColorPiglinHead},
    {piglin_wall_head, kColorPiglinHead},
    {spruce_hanging_sign, kColorPlanksSpruce},
    {spruce_wall_hanging_sign, kColorPlanksSpruce},
    {stripped_bamboo_block, kColorPlanksBamboo},
    {warped_hanging_sign, kColorPlanksWarped},
    {warped_wall_hanging_sign, kColorPlanksWarped},
    {cherry_button, kColorPlanksCherry},
    {cherry_door, kColorPlanksCherry},
    {cherry_fence, kColorPlanksCherry},
    {cherry_fence_gate, kColorPlanksCherry},
    {cherry_hanging_sign, kColorPlanksCherry},
    {cherry_leaves, kColorLeavesCherry},
    {cherry_log, kColorLogCherry},
    {cherry_planks, kColorPlanksCherry},
    {cherry_pressure_plate, kColorPlanksCherry},
    {cherry_sapling, kColorLeavesCherry},
    {cherry_sign, kColorPlanksCherry},
    {cherry_slab, kColorPlanksCherry},
    {cherry_stairs, kColorPlanksCherry},
    {cherry_trapdoor, kColorPlanksCherry},
    {cherry_wall_hanging_sign, kColorPlanksCherry},
    {cherry_wall_sign, kColorPlanksCherry},
    {cherry_wood, kColorLogCherry},
    {decorated_pot, kColorPotter},
    {pink_petals, kColorLeavesCherry},
    {potted_cherry_sapling, kColorPotter},
    {potted_torchflower, kColorPotter},
    {stripped_cherry_log, kColorLogCherryStripped},
    {stripped_cherry_wood, kColorLogCherryStripped},
    {suspicious_sand, kColorSand},
    {torchflower, kColorTorchflower},
    {torchflower_crop, kColorTorchflower},
    {calibrated_sculk_sensor, kColorSculkSensor},
    {pitcher_crop, kColorPitcher},
    {pitcher_plant, kColorPitcher},
    {sniffer_egg, Colour(186, 77, 57)},
    {suspicious_gravel, kColorGravel},
};

static std::set<mcfile::blocks::BlockId> plantBlocks = {
    beetroots,
    carrots,
    potatoes,
    seagrass,
    tall_seagrass,
    fern,
    azure_bluet,
    large_fern,
    big_dripleaf,
    big_dripleaf_stem,
    small_dripleaf,
};

static std::set<mcfile::blocks::BlockId> transparentBlocks = {
    air,
    cave_air,
    vine, // Colour(56, 95, 31)}, //
    glow_lichen,
    ladder, // Colour(255, 255, 255)},
    glass_pane,
    glass,
    brown_wall_banner,
    redstone_wall_torch,
    wall_torch,
    redstone_torch,
    torch,
    barrier,
    black_banner,
    black_wall_banner,
    black_stained_glass,
    black_stained_glass_pane,
    blue_banner,
    blue_stained_glass,
    blue_stained_glass_pane,
    blue_wall_banner,
    brown_banner,
    brown_stained_glass,
    brown_stained_glass_pane,
    gray_wall_banner,
    cyan_banner,
    cyan_wall_banner,
    cyan_stained_glass,
    cyan_stained_glass_pane,
    gray_banner,
    gray_stained_glass,
    gray_stained_glass_pane,
    green_banner,
    green_stained_glass,
    green_stained_glass_pane,
    green_wall_banner,
    light_blue_banner,
    light_blue_stained_glass,
    light_blue_stained_glass_pane,
    light_blue_wall_banner,
    light_gray_banner,
    light_gray_stained_glass,
    light_gray_stained_glass_pane,
    light_gray_wall_banner,
    lime_banner,
    lime_stained_glass,
    lime_stained_glass_pane,
    lime_wall_banner,
    magenta_banner,
    magenta_stained_glass,
    magenta_stained_glass_pane,
    magenta_wall_banner,
    orange_banner,
    orange_stained_glass,
    orange_stained_glass_pane,
    orange_wall_banner,
    pink_banner,
    pink_stained_glass,
    pink_stained_glass_pane,
    pink_wall_banner,
    purple_banner,
    purple_stained_glass,
    purple_stained_glass_pane,
    purple_wall_banner,
    red_banner,
    red_stained_glass,
    red_stained_glass_pane,
    red_wall_banner,
    white_banner,
    white_stained_glass,
    white_stained_glass_pane,
    white_wall_banner,
    yellow_banner,
    yellow_stained_glass,
    yellow_stained_glass_pane,
    yellow_wall_banner,
    void_air,
    structure_void,
    tripwire,

    hanging_roots,
    candle,
    white_candle,
    orange_candle,
    magenta_candle,
    light_blue_candle,
    yellow_candle,
    lime_candle,
    pink_candle,
    gray_candle,
    light_gray_candle,
    cyan_candle,
    purple_candle,
    blue_candle,
    brown_candle,
    green_candle,
    red_candle,
    black_candle,
    light,
    lightning_rod,
    tinted_glass,

    frogspawn,
    sculk_vein,
    mangrove_propagule,
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

void RegionToTexture::Load(mcfile::je::Region const &region, ThreadPoolJob *job, Dimension dim, std::function<void(PixelARGB *)> completion) {
  int const width = 512;
  int const height = 512;

  std::vector<PixelInfo> pixelInfo(width * height);
  std::fill(pixelInfo.begin(), pixelInfo.end(), PixelInfo{-1, 0, 0});
  std::vector<Biome> biomes(width * height);

  int const minX = region.minBlockX();
  int const minZ = region.minBlockZ();

#if 1
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
    for (mcfile::blocks::BlockId id = 1; id < minecraft_max_block_id; id++) {
      auto const it = ids.find(id);
      if (IsWater(id)) {
        continue;
      }
      if (it == ids.end()) {
        std::u8string u8name = mcfile::blocks::Name(id);
        auto name = String::fromUTF8((char const *)u8name.c_str(), u8name.size());
        count++;
        String n = name.replace(":", "::");
        std::cout << "setblock " << count << " -60 0 " << name.toStdString() << std::endl;
        if (n.startsWith("minecraft::")) {
          n = n.substring(11);
        }
        juce::Logger::outputDebugString("{" + n.toStdString() + ", Colour()},");

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
  region.loadAllChunks(error, [&pixelInfo, &biomes, minX, minZ, width, height, job, dim, &didset](mcfile::je::Chunk const &chunk) {
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
            if (!block)
              continue;
            if (block == air) {
              yini = y;
              break;
            }
          }
        } else if (dim == Dimension::TheEnd) {
          ymax = 255;
          ymin = 0;
          yini = ymax;
        }
        bool all_transparent = true;
        bool found_opaque_block = false;
        for (int y = yini; y >= ymin; y--) {
          auto block = chunk.blockIdAt(x, y, z);
          if (block == mcfile::blocks::unknown) {
            continue;
          }
          if (IsWater(block)) {
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
            int const h = std::min(std::max(y + 64, 0), 511);
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
          info.blockId = water;
          pixelInfo[idx] = info;
          didset = true;
        } else if (all_transparent) {
          PixelInfo info;
          info.height = 0;
          info.waterDepth = 0;
          info.blockId = air;
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

File RegionToTexture::CacheFile(File const &file) {
#if JUCE_WINDOWS
  File tmp = File::getSpecialLocation(File::SpecialLocationType::tempDirectory).getChildFile("mcview").getChildFile("cache");
#else
  File tmp = File::getSpecialLocation(File::SpecialLocationType::tempDirectory).getChildFile("cache");
#endif
  if (!tmp.exists()) {
    tmp.createDirectory();
  }
  String hash = String("v4.") + String(file.getParentDirectory().getFullPathName().hashCode64());
  File dir = tmp.getChildFile(hash);
  if (!dir.exists()) {
    dir.createDirectory();
  }
  return dir.getChildFile(file.getFileNameWithoutExtension() + String(".gz"));
}

} // namespace mcview
