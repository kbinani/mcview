#include "RegionToTexture.h"
#include <cassert>
#include "minecraft-file.hpp"
#include <colormap/colormap.h>
#include <map>
#include <set>

using namespace mcfile;

static std::map<blocks::BlockId, Colour> const blockToColor {
    {blocks::minecraft::stone, Colour(111, 111, 111)},
    {blocks::minecraft::granite, Colour(149, 108, 76)},
    {blocks::minecraft::diorite, Colour(252, 249, 242)},
    {blocks::minecraft::andesite, Colour(111, 111, 111)},
    {blocks::minecraft::chest, Colour(141, 118, 71)},
    {blocks::minecraft::clay, Colour(162, 166, 182)},
    {blocks::minecraft::coal_ore, Colour(111, 111, 111)},
    {blocks::minecraft::cobblestone, Colour(111, 111, 111)},
    {blocks::minecraft::dirt, Colour(149, 108, 76)},
    {blocks::minecraft::brown_mushroom, Colour(0, 123, 0)},
    {blocks::minecraft::grass_block, Colour(130, 148, 58)},
    {blocks::minecraft::iron_ore, Colour(111, 111, 111)},
    {blocks::minecraft::sand, Colour(201,192,154)}, //
    {blocks::minecraft::oak_leaves, Colour(56, 95, 31)}, //
    {blocks::minecraft::jungle_leaves, Colour(56, 95, 31)}, //
    {blocks::minecraft::birch_leaves, Colour(67, 124, 37)},
    {blocks::minecraft::red_mushroom, Colour(0, 123, 0)},
    {blocks::minecraft::mossy_cobblestone, Colour(111, 111, 111)},
    {blocks::minecraft::oak_stairs, Colour(127, 85, 48)},
    {blocks::minecraft::gravel, Colour(111, 111, 111)},
    {blocks::minecraft::oak_log, Colour(141, 118, 71)},
    {blocks::minecraft::oak_planks, Colour(127, 85, 48)},
    {blocks::minecraft::wall_torch, Colour(255, 255, 255)},
    {blocks::minecraft::farmland, Colour(149, 108, 76)},
    {blocks::minecraft::oak_fence, Colour(127, 85, 48)},
    {blocks::minecraft::cobblestone_stairs, Colour(111, 111, 111)},
    {blocks::minecraft::black_wool, Colour(25, 25, 25)},
    {blocks::minecraft::grass_path, Colour(204, 204, 204)}, //
    {blocks::minecraft::birch_fence, Colour(244, 230, 161)},
    {blocks::minecraft::birch_planks, Colour(244, 230, 161)},
    {blocks::minecraft::birch_stairs, Colour(244, 230, 161)},
    {blocks::minecraft::dark_oak_fence, Colour(101, 75, 50)},
    {blocks::minecraft::dark_oak_log, Colour(101, 75, 50)},
    {blocks::minecraft::dark_oak_planks, Colour(191,152,63)}, //
    {blocks::minecraft::dark_oak_slab, Colour(101, 75, 50)},
    {blocks::minecraft::dark_oak_stairs, Colour(101, 75, 50)},
    {blocks::minecraft::dark_oak_trapdoor, Colour(141, 118, 71)},
    {blocks::minecraft::diamond_ore, Colour(111, 111, 111)},
    {blocks::minecraft::gold_ore, Colour(111, 111, 111)},
    {blocks::minecraft::ice, Colour(158, 158, 252)},
    {blocks::minecraft::jungle_fence, Colour(149, 108, 76)},
    {blocks::minecraft::jungle_log, Colour(149, 108, 76)},
    {blocks::minecraft::jungle_planks, Colour(149, 108, 76)},
    {blocks::minecraft::jungle_slab, Colour(149, 108, 76)},
    {blocks::minecraft::jungle_stairs, Colour(149, 108, 76)},
    {blocks::minecraft::jungle_trapdoor, Colour(141, 118, 71)},
    {blocks::minecraft::lapis_ore, Colour(111, 111, 111)},
    {blocks::minecraft::lava, Colour(179, 71, 3)},
    {blocks::minecraft::oak_door, Colour(141, 118, 71)},
    {blocks::minecraft::oak_slab, Colour(127, 85, 48)},
    {blocks::minecraft::oak_trapdoor, Colour(141, 118, 71)},
    {blocks::minecraft::obsidian, Colour(25, 25, 25)},
    {blocks::minecraft::packed_ice, Colour(158, 158, 252)},
    {blocks::minecraft::polished_granite, Colour(149, 108, 76)},
    {blocks::minecraft::prismarine, Colour(75, 125, 151)},
    {blocks::minecraft::prismarine_bricks, Colour(91, 216, 210)},
    {blocks::minecraft::rail, Colour(255, 255, 255)},
    {blocks::minecraft::redstone_ore, Colour(111, 111, 111)},
    {blocks::minecraft::sandstone, Colour(244, 230, 161)},
    {blocks::minecraft::sea_lantern, Colour(252, 249, 242)},
    {blocks::minecraft::snow, Colour(229, 229, 229)}, //
    {blocks::minecraft::snow_block, Colour(252, 252, 252)},
    {blocks::minecraft::spruce_door, Colour(141, 118, 71)},
    {blocks::minecraft::spruce_fence, Colour(141, 118, 71)},
    {blocks::minecraft::spruce_leaves, Colour(56, 95, 31)}, //
    {blocks::minecraft::stone_brick_stairs, Colour(111, 111, 111)},
    {blocks::minecraft::stone_bricks, Colour(111, 111, 111)},
    {blocks::minecraft::stone_slab, Colour(111, 111, 111)},
    {blocks::minecraft::spruce_log, Colour(141, 118, 71)},
    {blocks::minecraft::spruce_planks, Colour(127, 85, 48)},
    {blocks::minecraft::spruce_slab, Colour(127, 85, 48)},
    {blocks::minecraft::spruce_stairs, Colour(141, 118, 71)},
    {blocks::minecraft::spruce_trapdoor, Colour(141, 118, 71)},
    {blocks::minecraft::mossy_stone_bricks, Colour(111, 111, 111)},
    {blocks::minecraft::chiseled_stone_bricks, Colour(111, 111, 111)},
    {blocks::minecraft::cracked_stone_bricks, Colour(111, 111, 111)},
    {blocks::minecraft::infested_stone, Colour(111, 111, 111)},
    {blocks::minecraft::cobweb, Colour(255, 255, 255)},
    {blocks::minecraft::blue_ice, Colour(102, 151, 246)},
    {blocks::minecraft::magma_block, Colour(181, 64, 9)},

    // plants
    {blocks::minecraft::lily_pad, Colour(0, 123, 0)},
    {blocks::minecraft::wheat, Colour(0, 123, 0)},
    {blocks::minecraft::melon, Colour(125, 202, 25)},
    {blocks::minecraft::pumpkin, Colour(213, 125, 50)},
    {blocks::minecraft::grass, Colour(109, 141, 35)},
    {blocks::minecraft::tall_grass, Colour(109, 141, 35)},
    {blocks::minecraft::dandelion, Colour(245, 238, 50)},
    {blocks::minecraft::poppy, Colour(229, 31, 29)},
    {blocks::minecraft::peony, Colour(232, 143, 213)},
    {blocks::minecraft::pink_tulip, Colour(234, 182, 209)},
    {blocks::minecraft::orange_tulip, Colour(242, 118, 33)},
    {blocks::minecraft::lilac, Colour(212, 119, 197)},
    {blocks::minecraft::sunflower, Colour(245, 238, 50)},
    {blocks::minecraft::allium, Colour(200, 109, 241)},
    {blocks::minecraft::red_tulip, Colour(229, 31, 29)},
    {blocks::minecraft::white_tulip, Colour(255, 255, 255)},
    {blocks::minecraft::rose_bush, Colour(136, 40, 27)},
    {blocks::minecraft::blue_orchid, Colour(47, 181, 199)},
    {blocks::minecraft::oxeye_daisy, Colour(236, 246, 247)},
    {blocks::minecraft::sugar_cane, Colour(165, 214, 90)},
};

static std::set<blocks::BlockId> plantBlocks = {
    blocks::minecraft::beetroots,
    blocks::minecraft::carrots,
    blocks::minecraft::potatoes,
    blocks::minecraft::seagrass,
    blocks::minecraft::tall_seagrass,
    blocks::minecraft::fern,
    blocks::minecraft::azure_bluet,
    blocks::minecraft::kelp,
    blocks::minecraft::large_fern,
    blocks::minecraft::kelp_plant,
};

static std::set<blocks::BlockId> transparentBlocks = {
    blocks::minecraft::air,
    blocks::minecraft::cave_air,
    blocks::minecraft::vine, // Colour(56, 95, 31)}, //
    blocks::minecraft::ladder, // Colour(255, 255, 255)},
};

static Colour Diffuse(Colour base, float diffusion, float distance) {
    float intensity = pow(10., -diffusion * distance);
    return Colour::fromFloatRGBA(base.getFloatRed(), base.getFloatGreen(), base.getFloatBlue(), base.getFloatAlpha() * intensity);
}

static Colour Add(Colour a, Colour b) {
    return Colour::fromFloatRGBA(a.getFloatRed() * a.getFloatAlpha() + b.getFloatRed() * b.getFloatAlpha(),
                 a.getFloatGreen() * a.getFloatAlpha() + b.getFloatGreen() * b.getFloatAlpha(),
                 a.getFloatBlue() * a.getFloatAlpha() + b.getFloatBlue() * b.getFloatAlpha(),
                 1.f);
}

void RegionToTexture::Load(mcfile::Region const& region, std::function<void(PixelARGB *, uint8 *)> completion) {
    int const width = 512;
    int const height = 512;

    std::unique_ptr<PixelARGB[]> pixels(new PixelARGB[width * height]);
    PixelARGB * const pixelsPtr = pixels.get();
    std::fill_n(pixelsPtr, width * height, PixelARGB(255, 0, 0, 0));
    std::unique_ptr<uint8[]> heightMap(new uint8[width * height]);
    uint8 * const heightMapPtr = heightMap.get();

    int const minX = region.minBlockX();
    int const minZ = region.minBlockZ();

    bool error = false;
    region.loadAllChunks(error, [pixelsPtr, heightMapPtr, minX, minZ, width, height](Chunk const& chunk) {
        Colour waterColor(69, 91, 211);
        float const waterDiffusion = 0.02;
        colormap::kbinani::Altitude altitude;
        int const sZ = chunk.minBlockZ();
        int const eZ = chunk.maxBlockZ();
        int const sX = chunk.minBlockX();
        int const eX = chunk.maxBlockX();
        for (int z = sZ; z <= eZ; z++) {
            for (int x = sX; x <= eX; x++) {
                int waterDepth = 0;
                int airDepth = 0;
                Colour translucentBlock = Colour::fromRGBA(0, 0, 0, 0);
                for (int y = 255; y >= 0; y--) {
                    auto block = chunk.blockIdAt(x, y, z);
                    if (!block) {
                        airDepth++;
                        continue;
                    }
                    if (block == mcfile::blocks::minecraft::water || block == mcfile::blocks::minecraft::bubble_column) {
                        waterDepth++;
                        continue;
                    }
                    if (transparentBlocks.find(block) != transparentBlocks.end()) {
                        airDepth++;
                        continue;
                    }
                    if (plantBlocks.find(block) != plantBlocks.end()) {
                        airDepth++;
                        continue;
                    }
                    auto it = blockToColor.find(block);
                    if (it == blockToColor.end()) {

                    } else {
                        int const idx = (z - minZ) * width + (x - minX);
                        assert(0 <= idx && idx < width * height);
                        uint8 const h = (uint8)std::min(std::max(y, 0), 255);

                        Colour const opaqeBlockColor = it->second;
                        Colour colour = Colour::fromRGBA(0, 0, 0, 0);
                        if (waterDepth > 0) {
                            colour = Diffuse(waterColor, waterDiffusion, waterDepth);
                            translucentBlock = Colour::fromRGBA(0, 0, 0, 0);
                        } else if (block == blocks::minecraft::grass_block) {
                            float const v = std::min(std::max((y - 63.0) / 193.0, 0.0), 1.0);
                            auto c = altitude.getColor(v);
                            colour = Colour::fromFloatRGBA(c.r, c.g, c.b, 1);
                            heightMapPtr[idx] = h;
                        } else {
                            colour = opaqeBlockColor;
                            heightMapPtr[idx] = h;
                        }
                        PixelARGB p = Add(colour, translucentBlock.withAlpha(0.2f)).getPixelARGB();
                        pixelsPtr[idx] = p;
                        break;
                    }
                }
            }
        }
        return true;
    });

    completion(pixels.release(), heightMap.release());
}
