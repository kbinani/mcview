#include <juce_gui_basics/juce_gui_basics.h>
#include <minecraft-file.hpp>

#include "Dimension.hpp"
#include "Palette.hpp"

#include "RegionToTexture.hpp"

using namespace juce;
using namespace mcview;
using namespace mcfile::blocks::minecraft;

namespace mcview {
std::set<mcfile::blocks::BlockId> const RegionToTexture::kPlantBlocks = {
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

std::set<mcfile::blocks::BlockId> const RegionToTexture::kTransparentBlocks = {
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

} // namespace mcview
