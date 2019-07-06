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
    {mcfile::blocks::minecraft::rail, Colour(255, 255, 255)},
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

static PixelARGB ToPixelInfo(uint8_t height, uint8_t waterDepth, uint8_t biome, uint32_t block)
{
    // h:           8bit
    // waterDepth:  8bit
    // biome:       4bit
    // block:      12bit
    static_assert((int)Biome::max_Biome <= 1 << 4, "");
    static_assert(mcfile::blocks::minecraft::minecraft_max_block_id <= 1 << 12, "");
    uint32_t const num = ((0xFF & (uint32_t)height) << 24)
        | ((0xFF & (uint32_t)waterDepth) << 16)
        | ((0xF & (uint32_t)biome) << 12)
        | (0xFFF & (uint32_t)block);
    PixelARGB p;
    p.setARGB(0xFF & (num >> 24), 0xFF & (num >> 16), 0xFF & (num >> 8), 0xFF & num);
    return p;
}

void RegionToTexture::Load(mcfile::Region const& region, ThreadPoolJob *job, Dimension dim, std::function<void(PixelARGB *)> completion) {
    int const width = 512;
    int const height = 512;

    std::unique_ptr<PixelARGB[]> pixels(new PixelARGB[width * height]);
    PixelARGB * const pixelsPtr = pixels.get();
    std::fill_n(pixelsPtr, width * height, PixelARGB(0, 0, 0, 0));

    int const minX = region.minBlockX();
    int const minZ = region.minBlockZ();

    bool error = false;
    bool didset = false;
    region.loadAllChunks(error, [&pixels, minX, minZ, width, height, job, dim, &didset](mcfile::Chunk const& chunk) {
        colormap::kbinani::Altitude altitude;
        int const sZ = chunk.minBlockZ();
        int const eZ = chunk.maxBlockZ();
        int const sX = chunk.minBlockX();
        int const eX = chunk.maxBlockX();
        for (int z = sZ; z <= eZ; z++) {
            for (int x = sX; x <= eX; x++) {
                int const idx = (z - minZ) * width + (x - minX);
                assert(0 <= idx && idx < width * height);
                if (job->shouldExit()) {
                    pixels.reset();
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
                            std::cout << "{" << name << ", Colour(, , )}," << name << std::endl;
                        }
#endif
                    } else {
                        uint8_t const h = (uint8)std::min(std::max(y, 0), 255);
                        Biome biome = ToBiome(chunk.biomeAt(x, z));
                        pixels[idx] = ToPixelInfo(h, waterDepth, (uint8_t)biome, block);
                        didset = true;
                        break;
                    }
                }
                if (all_transparent) {
                    pixels[idx] = ToPixelInfo(0, 0, 0, mcfile::blocks::minecraft::air);
                    didset = true;
                }
            }
        }
        return true;
    });

    if (didset) {
        completion(pixels.release());
    } else {
        completion(nullptr);
    }
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
    String hash = String("v0.") + String(file.getParentDirectory().getFullPathName().hashCode64());
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
    File cache = CacheFile(fRegionFile);
    if (fUseCache && cache.existsAsFile()) {
        FileInputStream stream(cache);
        GZIPDecompressorInputStream ungzip(stream);
        fPixels.reset(new PixelARGB[512 * 512]);
        int expectedBytes = sizeof(PixelARGB) * 512 * 512;
        if (ungzip.read(fPixels.get(), expectedBytes) != expectedBytes) {
            fPixels.reset();
        }
        return ThreadPoolJob::jobHasFinished;
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
        gzip.write(fPixels.get(), sizeof(PixelARGB) * 512 * 512);
    }
    return ThreadPoolJob::jobHasFinished;
}
