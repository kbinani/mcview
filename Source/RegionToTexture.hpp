#pragma once

namespace mcview {

enum class Biome : uint8_t {
  Other = 0,
  Swamp,
  Ocean,
  LukewarmOcean,
  WarmOcean,
  ColdOcean,
  Badlands,
  MangroveSwamp,

  max_Biome,
};

class RegionToTexture {
  RegionToTexture() = delete;

  struct PixelInfo {
    int height;
    int waterDepth;
    mcfile::blocks::BlockId blockId;
  };

public:
  static std::optional<PixelInfo> PillarPixelInfo(Dimension dim, int x, int z, int maxBlockY, std::function<mcfile::blocks::BlockId(int, int, int)> blockIdAt) {
    uint8_t waterDepth = 0;
    int ymax = 319;
    int ymin = -64;

    int yini = ymax;
    if (dim == Dimension::TheNether) {
      ymax = 127;
      ymin = 0;
      yini = ymax;
      for (int y = ymax; y >= ymin; y--) {
        auto block = blockIdAt(x, y, z);
        if (!block) {
          continue;
        }
        if (block == mcfile::blocks::minecraft::air) {
          yini = y;
          break;
        }
      }
    } else if (dim == Dimension::TheEnd) {
      ymax = 255;
      ymin = 0;
      yini = ymax;
    }
    yini = std::min(yini, maxBlockY);
    bool all_transparent = true;
    bool found_opaque_block = false;
    for (int y = yini; y >= ymin; y--) {
      auto block = blockIdAt(x, y, z);
      if (block == mcfile::blocks::unknown) {
        continue;
      }
      if (Palette::IsWater(block)) {
        waterDepth++;
        all_transparent = false;
        continue;
      }
      if (kTransparentBlocks.find(block) != kTransparentBlocks.end()) {
        continue;
      }
      if (kPlantBlocks.find(block) != kPlantBlocks.end()) {
        continue;
      }
      all_transparent = false;
      int const h = std::min(std::max(y + 64, 0), 511);
      PixelInfo info;
      info.height = h;
      info.waterDepth = waterDepth;
      info.blockId = block;
      return info;
    }
    if (waterDepth > 0) {
      PixelInfo info;
      info.height = 0;
      info.waterDepth = waterDepth;
      info.blockId = mcfile::blocks::minecraft::water;
      return info;
    } else if (all_transparent) {
      RegionToTexture::PixelInfo info;
      info.height = 0;
      info.waterDepth = 0;
      info.blockId = mcfile::blocks::minecraft::air;
      return info;
    }
    return std::nullopt;
  }

  static juce::PixelARGB *Pack(std::vector<PixelInfo> const &pixelInfo, std::vector<Biome> const &biomes, int width, int height) {
    using namespace juce;
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
        pixels[idx] = PackPixelInfoToARGB(info.height, info.waterDepth, (uint8_t)biome, (uint32_t)info.blockId, biomeRadius);
      }
    }

    return pixels.release();
  }

  static juce::PixelARGB *LoadJava(mcfile::je::Region const &region, ThreadPoolJob *job, Dimension dim) {
    using namespace juce;
    using namespace mcfile::blocks::minecraft;

    int const width = 512;
    int const height = 512;

    std::vector<PixelInfo> pixelInfo(width * height, PixelInfo{-1, 0, 0});
    std::vector<Biome> biomes(width * height, Biome::Other);

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
    bool completed = region.loadAllChunks(
        error, [&pixelInfo, &biomes, minX, minZ, width, height, job, dim, &didset](mcfile::je::Chunk const &chunk) {
          int maxSectionY = -9999;
          for (int i = (int)chunk.fSections.size() - 1; i >= 0; i--) {
            if (chunk.fSections[i]) {
              maxSectionY = chunk.fSections[i]->y();
              break;
            }
          }
          if (maxSectionY < -4) {
            return !job->shouldExit();
          }
          int const sZ = chunk.minBlockZ();
          int const eZ = chunk.maxBlockZ();
          int const sX = chunk.minBlockX();
          int const eX = chunk.maxBlockX();
          for (int z = sZ; z <= eZ; z++) {
            for (int x = sX; x <= eX; x++) {
              if (job->shouldExit()) {
                return false;
              }
              Biome biome = ToBiome(chunk.biomeAt(x, z));
              int i = (z - minZ) * width + (x - minX);
              biomes[i] = biome;
            }
          }
          for (int z = sZ; z <= eZ; z++) {
            for (int x = sX; x <= eX; x++) {
              if (job->shouldExit()) {
                return false;
              }
              int const idx = (z - minZ) * width + (x - minX);
              assert(0 <= idx && idx < width * height);
              auto info = PillarPixelInfo(dim, x, z, maxSectionY * 16 + 15, [&chunk](int x, int y, int z) { return chunk.blockIdAt(x, y, z); });
              if (info) {
                pixelInfo[idx] = *info;
                didset = true;
              }
            }
          }
          return !job->shouldExit();
        },
        true);

    if (error || !didset || !completed) {
      return nullptr;
    }

    return Pack(pixelInfo, biomes, width, height);
  }

  static juce::PixelARGB *LoadBedrock(leveldb::DB &db, int rx, int rz, ThreadPoolJob *job, Dimension dim);

private:
  static juce::PixelARGB PackPixelInfoToARGB(uint32_t height, uint8_t waterDepth, uint8_t biome, uint32_t block, uint8_t biomeRadius) {
    using namespace juce;
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

public:
  static std::map<Biome, juce::Colour> const kOceanToColor;

  static juce::Colour const kDefaultFoliageColor;
  static std::map<Biome, juce::Colour> const kFoliageToColor;

  static std::set<mcfile::blocks::BlockId> const kPlantBlocks;
  static std::set<mcfile::blocks::BlockId> const kTransparentBlocks;
};

} // namespace mcview
