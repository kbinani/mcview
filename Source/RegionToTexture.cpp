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
    {mcfile::blocks::minecraft::black_wool, Colour(25, 25, 25)},
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
    {mcfile::blocks::minecraft::obsidian, Colour(25, 25, 25)},
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

void RegionToTexture::Load(mcfile::Region const& region, ThreadPoolJob *job, std::function<void(PixelARGB *)> completion) {
    int const width = 512;
    int const height = 512;

    std::unique_ptr<PixelARGB[]> pixels(new PixelARGB[width * height]);
    PixelARGB * const pixelsPtr = pixels.get();
    std::fill_n(pixelsPtr, width * height, PixelARGB(0, 0, 0, 0));

    int const minX = region.minBlockX();
    int const minZ = region.minBlockZ();

    bool error = false;
    region.loadAllChunks(error, [pixelsPtr, minX, minZ, width, height, job](mcfile::Chunk const& chunk) {
        colormap::kbinani::Altitude altitude;
        int const sZ = chunk.minBlockZ();
        int const eZ = chunk.maxBlockZ();
        int const sX = chunk.minBlockX();
        int const eX = chunk.maxBlockX();
        for (int z = sZ; z <= eZ; z++) {
            for (int x = sX; x <= eX; x++) {
                if (job->shouldExit()) {
                    return false;
                }
                uint8 waterDepth = 0;
                for (int y = 255; y >= 0; y--) {
                    auto block = chunk.blockIdAt(x, y, z);
                    if (!block) {
                        continue;
                    }
                    if (block == mcfile::blocks::minecraft::water || block == mcfile::blocks::minecraft::bubble_column) {
                        waterDepth++;
                        continue;
                    }
                    if (transparentBlocks.find(block) != transparentBlocks.end()) {
                        continue;
                    }
                    if (plantBlocks.find(block) != plantBlocks.end()) {
                        continue;
                    }
                    auto it = kBlockToColor.find(block);
                    if (it == kBlockToColor.end()) {

                    } else {
                        int const idx = (z - minZ) * width + (x - minX);
                        assert(0 <= idx && idx < width * height);
                        uint8 const h = (uint8)std::min(std::max(y, 0), 255);
                        PixelARGB p;
                        p.setARGB(h, waterDepth, 0xFF & (block >> 8), 0xFF & block);
                        pixelsPtr[idx] = p;
                        break;
                    }
                }
            }
        }
        return true;
    });

    completion(pixels.release());
}

RegionToTexture::RegionToTexture(File const& mcaFile, Region region)
    : ThreadPoolJob(mcaFile.getFileName())
    , fRegionFile(mcaFile)
    , fRegion(region)
{
    
}

RegionToTexture::~RegionToTexture()
{
}

ThreadPoolJob::JobStatus RegionToTexture::runJob()
{
    auto region = mcfile::Region::MakeRegion(fRegionFile.getFullPathName().toStdString());
    if (!region) {
        return ThreadPoolJob::jobHasFinished;
    }
    Load(*region, this, [this](PixelARGB *pixels) {
        fPixels.reset(pixels);
    });
    return ThreadPoolJob::jobHasFinished;
}
