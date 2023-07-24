#pragma once

namespace mcview {

struct GLUniforms {
  GLUniforms(juce::OpenGLContext &openGLContext, juce::OpenGLShaderProgram &shader) {
    texture.reset(createUniform(openGLContext, shader, "texture"));
    fade.reset(createUniform(openGLContext, shader, "fade"));
    heightmap.reset(createUniform(openGLContext, shader, "heightmap"));
    blocksPerPixel.reset(createUniform(openGLContext, shader, "blocksPerPixel"));
    width.reset(createUniform(openGLContext, shader, "width"));
    height.reset(createUniform(openGLContext, shader, "height"));
    Xr.reset(createUniform(openGLContext, shader, "Xr"));
    Zr.reset(createUniform(openGLContext, shader, "Zr"));
    Cx.reset(createUniform(openGLContext, shader, "Cx"));
    Cz.reset(createUniform(openGLContext, shader, "Cz"));
    grassBlockId.reset(createUniform(openGLContext, shader, "grassBlockId"));
    foliageBlockId.reset(createUniform(openGLContext, shader, "foliageBlockId"));
    netherrackBlockId.reset(createUniform(openGLContext, shader, "netherrackBlockId"));
    waterBlockId.reset(createUniform(openGLContext, shader, "waterBlockId"));
    north.reset(createUniform(openGLContext, shader, "north"));
    northEast.reset(createUniform(openGLContext, shader, "norhtEast"));
    east.reset(createUniform(openGLContext, shader, "east"));
    southEast.reset(createUniform(openGLContext, shader, "southEast"));
    south.reset(createUniform(openGLContext, shader, "south"));
    southWest.reset(createUniform(openGLContext, shader, "southWest"));
    west.reset(createUniform(openGLContext, shader, "west"));
    northWest.reset(createUniform(openGLContext, shader, "northWest"));
    waterOpticalDensity.reset(createUniform(openGLContext, shader, "waterOpticalDensity"));
    waterTranslucent.reset(createUniform(openGLContext, shader, "waterTranslucent"));
    biomeBlend.reset(createUniform(openGLContext, shader, "biomeBlend"));
    enableBiome.reset(createUniform(openGLContext, shader, "enableBiome"));
    dimension.reset(createUniform(openGLContext, shader, "dimension"));
    palette.reset(createUniform(openGLContext, shader, "palette"));
    paletteSize.reset(createUniform(openGLContext, shader, "paletteSize"));
    paletteType.reset(createUniform(openGLContext, shader, "paletteType"));
    lightingType.reset(createUniform(openGLContext, shader, "lightingType"));
  }

  std::unique_ptr<juce::OpenGLShaderProgram::Uniform> texture, fade, heightmap, blocksPerPixel, width, height, Xr, Zr, Cx, Cz, grassBlockId, foliageBlockId, netherrackBlockId, waterBlockId, dimension;
  std::unique_ptr<juce::OpenGLShaderProgram::Uniform> north, northEast, east, southEast, south, southWest, west, northWest;
  std::unique_ptr<juce::OpenGLShaderProgram::Uniform> waterOpticalDensity, waterTranslucent, biomeBlend, enableBiome;
  std::unique_ptr<juce::OpenGLShaderProgram::Uniform> palette, paletteSize, paletteType, lightingType;

private:
  static juce::OpenGLShaderProgram::Uniform *createUniform(juce::OpenGLContext &openGLContext,
                                                           juce::OpenGLShaderProgram &shader,
                                                           const char *uniformName) {
    if (openGLContext.extensions.glGetUniformLocation(shader.getProgramID(), uniformName) < 0) {
      return nullptr;
    }

    return new juce::OpenGLShaderProgram::Uniform(shader, uniformName);
  }
};

} // namespace mcview
