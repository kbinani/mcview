#include "../JuceLibraryCode/JuceHeader.h"
#include "MapViewComponent.h"
#include "RegionToTexture.h"
#include "RegionTextureCache.h"
#include "OverScroller.hpp"
#include "PNGWriter.h"
#include "SettingsComponent.h"
#include "defer.h"
#include "GraphicsHelper.h"
#include <set>
#include <cassert>
#include <cmath>
#include <thread>
#include <minecraft-file.hpp>
#include <colormap/colormap.h>

float const MapViewComponent::kMaxScale = 10;
float const MapViewComponent::kMinScale = 1.0f / 32.0f;

static int const kMargin = 10;
static int const kButtonSize = 40;
static int const kFadeDurationMS = 300;

MapViewComponent::MapViewComponent()
    : fLookAt({0, 0, 5})
    , fVisibleRegions({0, 0, 0, 0})
    , fPool(CreateThreadPool())
    , fLoadingFinished(true)
    , fWaterOpticalDensity(SettingsComponent::kDefaultWaterOpticalDensity)
    , fWaterTranslucent(true)
    , fEnableBiome(true)
    , fBiomeBlend(2)
{
    if (auto* peer = getPeer()) {
        peer->setCurrentRenderingEngine (0);
    }

    fBrowserOpenButtonImageClose = Drawable::createFromImageData(BinaryData::baseline_keyboard_arrow_left_white_18dp_png,
                                                                 BinaryData::baseline_keyboard_arrow_left_white_18dp_pngSize);
    fBrowserOpenButtonImageOpen = Drawable::createFromImageData(BinaryData::baseline_keyboard_arrow_right_white_18dp_png,
                                                                BinaryData::baseline_keyboard_arrow_right_white_18dp_pngSize);
    
    fBrowserOpenButton.reset(new DrawableButton("Browser", DrawableButton::ButtonStyle::ImageOnButtonBackground));
    setBrowserOpened(true);
    fBrowserOpenButton->setSize(kButtonSize, kButtonSize);
    fBrowserOpenButton->onClick = [this]() {
        onOpenButtonClicked();
    };
    addAndMakeVisible(*fBrowserOpenButton);

    fOverworldImage = Drawable::createFromImageData(BinaryData::baseline_landscape_white_18dp_png,
                                                    BinaryData::baseline_landscape_white_18dp_pngSize);
    fOverworld.reset(new DrawableButton("", DrawableButton::ButtonStyle::ImageOnButtonBackground));
    fOverworld->setImages(fOverworldImage.get());
    fOverworld->onClick = [this]() {
        setWorldDirectory(fWorldDirectory, Dimension::Overworld);
    };
    fOverworld->setEnabled(false);
    fOverworld->setTooltip(TRANS("Overworld"));
    addAndMakeVisible(*fOverworld);
    
    fNetherImage = Drawable::createFromImageData(BinaryData::baseline_whatshot_white_18dp_png, BinaryData::baseline_whatshot_white_18dp_pngSize);
    fNether.reset(new DrawableButton("", DrawableButton::ButtonStyle::ImageOnButtonBackground));
    fNether->setImages(fNetherImage.get());
    fNether->onClick = [this]() {
        setWorldDirectory(fWorldDirectory, Dimension::TheNether);
    };
    fNether->setEnabled(false);
    fNether->setTooltip(TRANS("The Nether"));
    addAndMakeVisible(*fNether);
    
    fEndImage = Drawable::createFromImageData(BinaryData::baseline_brightness_3_white_18dp_png, BinaryData::baseline_brightness_3_white_18dp_pngSize);
    fEnd.reset(new DrawableButton("", DrawableButton::ButtonStyle::ImageOnButtonBackground));
    fEnd->setImages(fEndImage.get());
    fEnd->onClick = [this]() {
        setWorldDirectory(fWorldDirectory, Dimension::TheEnd);
    };
    fEnd->setEnabled(false);
    fEnd->setTooltip(TRANS("The End"));
    addAndMakeVisible(*fEnd);
    
    fCaptureButtonImage = Drawable::createFromImageData(BinaryData::baseline_camera_white_18dp_png,
                                                        BinaryData::baseline_camera_white_18dp_pngSize);
    fCaptureButton.reset(new DrawableButton("Capture", DrawableButton::ButtonStyle::ImageOnButtonBackground));
    fCaptureButton->setImages(fCaptureButtonImage.get());
    fCaptureButton->onClick = [this]() {
        captureToImage();
    };
    addAndMakeVisible(*fCaptureButton);
    fCaptureButton->setEnabled(false);

    fSettingsButtonImage = Drawable::createFromImageData(BinaryData::baseline_settings_white_18dp_png,
                                                         BinaryData::baseline_settings_white_18dp_pngSize);
    fSettingsButton.reset(new DrawableButton("Settings", DrawableButton::ButtonStyle::ImageOnButtonBackground));
    fSettingsButton->setImages(fSettingsButtonImage.get());
    fSettingsButton->onClick = [this]() {
        onSettingsButtonClicked();
    };
    addAndMakeVisible(*fSettingsButton);
    
    setOpaque(true);
    fOpenGLContext.setRenderer(this);
    fOpenGLContext.attachTo(*this);

    fScrollerTimer.fTimerCallback = [this](TimerInstance &timer) {
        if (!fScroller.computeScrollOffset()) {
            timer.stopTimer();
        }
        LookAt next = clampedLookAt();
        next.fX = fScroller.getCurrX() * next.fBlocksPerPixel;
        next.fZ = fScroller.getCurrY() * next.fBlocksPerPixel;
        setLookAt(next);
        triggerRepaint();
    };
    
    fTooltipWindow.reset(new TooltipWindow());
    addAndMakeVisible(*fTooltipWindow);
    
    fRegionUpdateChecker.reset(new RegionUpdateChecker(this));
    fRegionUpdateChecker->startThread();

    Desktop::getInstance().getAnimator().addChangeListener(this);

    fSize = Point<int>(600, 400);
    setSize (600, 400);
}

MapViewComponent::~MapViewComponent()
{
    Desktop::getInstance().getAnimator().removeChangeListener(this);

    fOpenGLContext.detach();
    fPool->removeAllJobs(true, -1);
    fRegionUpdateChecker->signalThreadShouldExit();
    fRegionUpdateChecker->waitForThreadToExit(-1);
}

void MapViewComponent::paint(Graphics &g)
{
    g.saveState();
    defer {
        g.restoreState();
    };

    int const width = getWidth();
    int const lineHeight = 24;
    int const coordLabelWidth = 100;
    int const coordLabelHeight = 2 * lineHeight;

    Rectangle<float> const border(width - kMargin - kButtonSize - kMargin - coordLabelWidth, kMargin, coordLabelWidth, coordLabelHeight);
    g.setColour(Colour::fromFloatRGBA(1, 1, 1, 0.8));
    g.fillRoundedRectangle(border, 6.0f);
    g.setColour(Colours::lightgrey);
    g.drawRoundedRectangle(border, 6.0f, 1.0f);
    Point<float> block = getMapCoordinateFromView(fMouse);
    int y = kMargin;
    g.setColour(Colours::black);
    Font bold(14, Font::bold);
    Font regular(14);
    Rectangle<int> line1(border.getX() + kMargin, y, border.getWidth() - 2 * kMargin, lineHeight);
    g.setFont(regular);
    g.drawText("X: ", line1, Justification::centredLeft);
    g.setFont(bold);
    g.drawFittedText(String::formatted("%d", (int)floor(block.x)), line1, Justification::centredRight, 1);
    y += lineHeight;
    Rectangle<int> line2(border.getX() + kMargin, y, border.getWidth() - 2 * kMargin, lineHeight);
    g.setFont(regular);
    g.drawText("Z: ", line2, Justification::centredLeft);
    g.setFont(bold);
    g.drawFittedText(String::formatted("%d", (int)floor(block.y)), line2, Justification::centredRight, 1);
    y += lineHeight;
}

void MapViewComponent::newOpenGLContextCreated()
{
    std::unique_ptr<Buffer> buffer(new Buffer());

    fOpenGLContext.extensions.glGenBuffers(1, &buffer->vBuffer);
	fOpenGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, buffer->vBuffer);
    std::vector<Vertex> vertices = {
        {{0, 0}, {0.0, 0.0}},
        {{1, 0}, {1.0, 0.0}},
        {{1, 1}, {1.0, 1.0}},
        {{0, 1}, {0.0, 1.0}},
    };
	fOpenGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	fOpenGLContext.extensions.glGenBuffers(1, &buffer->iBuffer);
	fOpenGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->iBuffer);
    std::vector<uint32_t> indices = { 0, 1, 2, 3 };
	fOpenGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);
    
    fBuffer.reset(buffer.release());
}

void MapViewComponent::updateShader()
{
    std::unique_ptr<OpenGLShaderProgram> newShader(new OpenGLShaderProgram(fOpenGLContext));

    newShader->addVertexShader(R"#(
        attribute vec2 textureCoordIn;
        attribute vec4 position;
        uniform float blocksPerPixel;
        uniform float Xr;
        uniform float Zr;
        uniform float width;
        uniform float height;
        uniform float Cx;
        uniform float Cz;
        varying vec2 textureCoordOut;
        void main() {
            textureCoordOut = textureCoordIn;

            float Xp = position.x;
            float Yp = position.y;
            float Xm = Xr + Xp * 512.0;
            float Zm = Zr + Yp * 512.0;
            float Xw = (Xm - Cx) / blocksPerPixel + width / 2.0;
            float Yw = (Zm - Cz) / blocksPerPixel + height / 2.0;
            float Xg = 2.0 * Xw / width - 1.0;
            float Yg = 1.0 - 2.0 * Yw / height;

            gl_Position = vec4(Xg, Yg, position.z, position.w);
        }
    )#");

    colormap::kbinani::Altitude altitude;

	std::ostringstream fragment;
    fragment << R"#(
        #version 120
        varying vec2 textureCoordOut;
        uniform sampler2D texture;
        uniform float fade;
        uniform int grassBlockId;
        uniform int foliageBlockId;
        uniform int netherrackBlockId;
        uniform float waterOpticalDensity;
        uniform bool waterTranslucent;
        uniform int biomeBlend;
        uniform bool enableBiome;
        uniform vec4 background;
    
        uniform sampler2D north;
        uniform sampler2D northEast;
        uniform sampler2D east;
        uniform sampler2D southEast;
        uniform sampler2D south;
        uniform sampler2D southWest;
        uniform sampler2D west;
        uniform sampler2D northWest;
    )#";

	fragment << altitude.getSource() << std::endl;
    
    fragment << R"#(
        struct BlockInfo {
            float height;
            float waterDepth;
            int biomeId;
            int blockId;
            int biomeRadius;
        };
    
        int imod(int x, int y) {
            return x - y * int(floor(float(x) / float(y)) + 0.1);
        }
    
        BlockInfo pixelInfo(vec4 color) {
            // h:            8bit
            // waterDepth:   7bit
            // biome:        4bit
            // block:       10bit
            // biomeRadius:  3bit
            
            /*
             AAAAAAAARRRRRRRRGGGGGGGGBBBBBBBB
             hhhhhhhhwwwwwwwbbbboooooooooorrr
             */
            
            int shift2 = 4;
            int shift3 = 8;
            int shift5 = 32;
            
            int h = int(color.a * 255.0);
            int depth = int(color.r * 255.0) / 2;
            int biome = imod(int(color.r * 255.0), 2) * shift2 + int(color.g * 255.0) / shift5;
            int block = imod(int(color.g * 255.0), 0x20) * shift5 + int(color.b * 255.0) / shift3;
            int biomeRadius = imod(int(color.b * 255.0), 8);
            BlockInfo info;
            info.height = float(h);
            info.waterDepth = float(depth) / 127.0 * 255.0;
            info.biomeId = biome;
            info.blockId = block;
            info.biomeRadius = biomeRadius;
            return info;
        }
    
        vec3 rgb2hsv(vec3 c)
        {
            vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
            vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
            
            float d = q.x - min(q.w, q.y);
            float e = 1.0e-10;
           
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }

        vec3 hsv2rgb(vec3 c)
        {
            vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        vec4 blend(vec4 bg, vec4 fg) {
            float a = fg.a + bg.a * (1.0 - fg.a);
            if (a == 0.0) {
                return vec4(0.0, 0.0, 0.0, 0.0);
            }
            float r = (fg.r * fg.a + bg.r * bg.a * (1.0 - fg.a)) / a;
            float g = (fg.g * fg.a + bg.g * bg.a * (1.0 - fg.a)) / a;
            float b = (fg.b * fg.a + bg.b * bg.a * (1.0 - fg.a)) / a;
            return vec4(r, g, b, a);
        }
    
        vec4 rgb(int r, int g, int b, int a) {
            return vec4(float(r) / 255.0, float(g) / 255.0, float(b) / 255.0, float(a) / 255.0);
        }

        vec4 netherrack_colormap(float x) {
            float h = 1.0 / 360.0;
            float s = 64.0 / 100.0;
            float vmin = 16.0 / 100.0;
            float vmax = 50.0 / 100.0;
            float v = vmin + (vmax - vmin) * clamp(1.0 - x, 0.0, 1.0);
            return vec4(hsv2rgb(vec3(h, s, v)), 1.0);
        }
    )#";
    
    fragment << "vec4 colorFromBlockId(int blockId) {" << std::endl;
    fragment << "    if (blockId == " << mcfile::blocks::minecraft::air << ") {" << std::endl;
    fragment << "        return background;" << std::endl;
    fragment << "    }" << std::endl;
    fragment << "    const vec4 mapping[" << (mcfile::blocks::minecraft::minecraft_max_block_id - 1) << "] = vec4[](" << std::endl;
    for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
        auto it = RegionToTexture::kBlockToColor.find(id);
        if (it != RegionToTexture::kBlockToColor.end()) {
            Colour c = it->second;
            GLfloat r = c.getRed() / 255.0f;
            GLfloat g = c.getGreen() / 255.0f;
            GLfloat b = c.getBlue() / 255.0f;
            fragment << "        vec4(float(" << r << "), float(" << g << "), float(" << b << "), 1.0)";
        } else {
            fragment << "        vec4(0.0, 0.0, 0.0, 0.0)";
        }
        if (id < mcfile::blocks::minecraft::minecraft_max_block_id - 1) {
            fragment << "," << std::endl;
        }
    }
    fragment << "    );" << std::endl;
    fragment << "    return mapping[blockId - 1];" << std::endl;
    fragment << "}" << std::endl;

    fragment << "vec4 waterColorFromBiome(int biome) {" << std::endl;
    for (auto it : RegionToTexture::kOceanToColor) {
        auto id = it.first;
        Colour c = it.second;
        int r = c.getRed();
        int g = c.getGreen();
        int b = c.getBlue();
        fragment << "    if (biome == " << (int)id << ") {" << std::endl;
        fragment << "        return rgb(" << r << ", " << g << ", " << b << ", 255);" << std::endl;
        fragment << "    } else" << std::endl;
    }
    fragment << "    { " << std::endl;
    auto ocean = RegionToTexture::kDefaultOceanColor;
    fragment << "        return rgb(" << (int)ocean.getRed() << ", " << (int)ocean.getGreen() << ", " << (int)ocean.getBlue() << ", 255);" << std::endl;
    fragment << "    }" << std::endl;
    fragment << "}";

    fragment << "vec4 foliageColorFromBiome(int biome) {" << std::endl;
    for (auto it : RegionToTexture::kFoliageToColor) {
        auto id = it.first;
        Colour c = it.second;
        int r = c.getRed();
        int g = c.getGreen();
        int b = c.getBlue();
        fragment << "    if (biome == " << (int)id << ") {" << std::endl;
        fragment << "        return rgb(" << r << ", " << g << ", " << b << ", 255);" << std::endl;
        fragment << "    } else" << std::endl;
    }
    fragment << "    { " << std::endl;
    auto foliage = RegionToTexture::kDefaultFoliageColor;
    fragment << "        return rgb(" << (int)foliage.getRed() << ", " << (int)foliage.getGreen() << ", " << (int)foliage.getBlue() << ", 255);" << std::endl;
    fragment << "    }" << std::endl;
    fragment << "}" << std::endl;
    
    fragment << R"#(

    vec4 waterColor() {
        vec2 center = textureCoordOut;
        vec4 sumColor = vec4(0.0, 0.0, 0.0, 0.0);
        int count = 0;
        for (int dx = -biomeBlend; dx <= biomeBlend; dx++) {
            for (int dz = -biomeBlend; dz <= biomeBlend; dz++) {
                float x = center.x + float(dx) / 512.0;
                float y = center.y + float(dz) / 512.0;
                vec4 c;
                if (x < 0.0) {
                    if (y < 0.0) {
                        c = texture2D(northWest, vec2(x + 1.0, y + 1.0));
                    } else if (y < 1.0) {
                        c = texture2D(west, vec2(x + 1.0, y));
                    } else {
                        c = texture2D(southWest, vec2(x + 1.0, y - 1.0));
                    }
                } else if (x < 1.0) {
                    if (y < 0.0) {
                        c = texture2D(north, vec2(x, y + 1.0));
                    } else if (y < 1.0) {
                        c = texture2D(texture, vec2(x, y));
                    } else {
                        c = texture2D(south, vec2(x, y - 1.0));
                    }
                } else {
                    if (y < 0.0) {
                        c = texture2D(northEast, vec2(x - 1.0, y + 1.0));
                    } else if (y < 1.0) {
                        c = texture2D(east, vec2(x - 1.0, y));
                    } else {
                        c = texture2D(southEast, vec2(x - 1.0, y - 1.0));
                    }
                }
                BlockInfo info = pixelInfo(c);
                sumColor += waterColorFromBiome(info.biomeId);
                count++;
            }
        }
        return sumColor / float(count);
    }
    
    void main() {
        float alpha = fade;

        vec4 color = texture2D(texture, textureCoordOut);
        BlockInfo info = pixelInfo(color);

        float height = info.height;
        float waterDepth = info.waterDepth;
        int biomeId = info.biomeId;
        int blockId = info.blockId;

        vec4 c;
        if (waterDepth > 0.0) {
            vec4 wc;
            if (enableBiome) {
                if (info.biomeRadius >= biomeBlend) {
                    wc = waterColorFromBiome(info.biomeId);
                } else {
                    wc = waterColor();
                }
            } else {
                wc = waterColorFromBiome(-1);
            }
            if (waterTranslucent) {
                float intensity = pow(10.0, -waterOpticalDensity * waterDepth);
                c = vec4(wc.r * intensity, wc.g * intensity, wc.b* intensity, alpha);
            } else {
                c = wc;
            }
        } else if (blockId == foliageBlockId) {
            vec4 lc = foliageColorFromBiome(enableBiome ? biomeId : -1);
            c = vec4(lc.rgb, alpha);
        } else if (blockId == grassBlockId) {
            float v = (height - 63.0) / 193.0;
            vec4 g = colormap(v);
            c = vec4(g.r, g.g, g.b, alpha);
        } else if (blockId == netherrackBlockId) {
            float v = (height - 31.0) / (127.0 - 31.0);
            vec4 cc = netherrack_colormap(v);
            c = vec4(cc.rgb, alpha);
        } else if (blockId == 0) {
            c = vec4(0.0, 0.0, 0.0, 0.0);
        } else {
            vec4 cc = colorFromBlockId(blockId);
            if (cc.a == 0.0) {
                c = cc;
            } else {
                c = vec4(cc.r, cc.g, cc.b, alpha);
            }
        }

        if (waterDepth == 0.0 || (waterDepth > 0.0 && waterTranslucent)) {
            float heightScore = 0.0; // +: bright, -: dark
            float d = 1.0 / 512.0;
            float tx = textureCoordOut.x;
            float ty = textureCoordOut.y;
            vec4 northC;
            if (ty - d < 0.0) {
                northC = texture2D(north, vec2(tx, ty - d + 1.0));
            } else {
                northC = texture2D(texture, vec2(tx, ty - d));
            }
            vec4 westC;
            if (tx - d < 0.0) {
                westC = texture2D(west, vec2(tx - d + 1.0, ty));
            } else {
                westC = texture2D(texture, vec2(tx - d, ty));
            }
            float northH = northC.a * 255.0;
            float westH = westC.a * 255.0;
            if (northH > 0.0) {
                if (northH > height) heightScore--;
                if (northH < height) heightScore++;
            }
            if (westH > 0.0) {
                if (westH > height) heightScore--;
                if (westH < height) heightScore++;
            }
        
            if (heightScore > 0.0) {
                float coeff = 1.2;
                vec3 hsv = rgb2hsv(c.rgb);
                hsv.b = hsv.b * coeff;
                c = vec4(hsv2rgb(hsv).rgb, c.a);
            } else if (heightScore < 0.0) {
                float coeff = 0.8;
                vec3 hsv = rgb2hsv(c.rgb);
                hsv.b = hsv.b * coeff;
                c = vec4(hsv2rgb(hsv).rgb, c.a);
            }
        }
    
        if (c.a == 0.0 && fade < 1.0) {
            gl_FragColor = vec4(0.0, 0.0, 0.0, 37.0 / 255.0 * (1.0 - fade));
        } else {
            if (fade < 1.0) {
                gl_FragColor = blend(c, vec4(0.0, 0.0, 0.0, 37.0 / 255.0 * (1.0 - fade)));
            } else {
                gl_FragColor = c;
            }
        }
    }
    )#";
    newShader->addFragmentShader(fragment.str());

    newShader->link();
    newShader->use();

    fUniforms.reset(new Uniforms(fOpenGLContext, *newShader));
    fAttributes.reset(new Attributes(fOpenGLContext, *newShader));

    fShader.reset(newShader.release());
}

void MapViewComponent::renderOpenGL()
{
    LookAt lookAt = clampedLookAt();
    auto desktopScale = (float)fOpenGLContext.getRenderingScale();
    Point<int> size = fSize.get();
    int const width = size.x * desktopScale;
    int const height = size.y * desktopScale;
    lookAt.fBlocksPerPixel /= desktopScale;
    render(width, height, lookAt, true);
}

template <typename T>
static T Clamp(T v, T min, T max)
{
    if (v < min) {
        return min;
    } else if (max < v) {
        return max;
    } else {
        return v;
    }
}

static float CubicEaseInOut(float t, float start, float end, float duration) {
    float b = start;
    float c = end - start;
    t  /= duration / 2.0f;
    if (t < 1.0f) {
        return c / 2.0f * t * t * t + b;
    } else {
        t = t - 2.0f;
        return c / 2.0f * (t * t * t + 2.0f) + b;
    }
}

void MapViewComponent::instantiateTextures(LookAt lookAt)
{
    bool loadingFinished = false;

    std::vector<std::pair<RegionToTexture*, float>> distances;
    for (int i = 0; i < fJobs.size(); i++) {
        auto& job = fJobs[i];
        if (fPool->contains(job.get())) {
            continue;
        }
        float distance = DistanceSqBetweenRegionAndLookAt(lookAt, job->fRegion);
        distances.emplace_back(job.get(), distance);
    }
    std::sort(distances.begin(), distances.end(), [](auto const& a, auto const& b) {
        return a.second < b.second;
    });

    int constexpr kNumLoadTexturesPerFrame = 16;
    for (int i = 0; i < kNumLoadTexturesPerFrame && i < distances.size(); i++) {
        RegionToTexture* job = distances[i].first;
        fPool->removeJob(job, false, 0);

        std::unique_ptr<RegionToTexture> j;
        for (auto it = fJobs.begin(); it != fJobs.end(); it++) {
            if (it->get() == job) {
                j.reset(it->release());
                fJobs.erase(it);
                break;
            }
        }

        assert(j);
        if (!j) {
            continue;
        }

        auto before = fTextures.find(j->fRegion);
        if (j->fPixels) {
            auto cache = std::make_shared<RegionTextureCache>(j->fRegion, j->fRegionFile.getFullPathName());
            cache->load(j->fPixels.get());
            if (before != fTextures.end()) {
                cache->fLoadTime = before->second->fLoadTime;
            }
            fTextures[j->fRegion] = cache;
        } else {
            if (before != fTextures.end()) {
                fTextures.erase(before);
            }
        }

        fLoadingRegionsLock.enter();
        auto it = fLoadingRegions.find(j->fRegion);
        if (it != fLoadingRegions.end()) {
            fLoadingRegions.erase(it);
        }
        if (fLoadingRegions.empty()) {
            fLoadingFinished = true;
            loadingFinished = true;
        }
        fLoadingRegionsLock.exit();
    }

    if (loadingFinished) {
        startTimer(kFadeDurationMS);
        triggerAsyncUpdate();
    }
}

void MapViewComponent::render(int const width, int const height, LookAt const lookAt, bool enableUI)
{
    if (enableUI) {
        OpenGLHelpers::clear(Colours::white);
    } else {
        OpenGLHelpers::clear(Colours::transparentBlack);
    }

    glViewport(0, 0, width, height);

    if (enableUI) {
        drawBackground();
    }
    
    Time const now = Time::getCurrentTime();
    
    if (fShader.get() == nullptr) {
        updateShader();
    }

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    fShader->use();
    auto const& textures = fTextures;
    
    for (auto it : textures) {
        auto cache = it.second;
        if (fUniforms->blocksPerPixel.get() != nullptr) {
            fUniforms->blocksPerPixel->set(lookAt.fBlocksPerPixel);
        }
        if (fUniforms->Xr.get() != nullptr) {
            fUniforms->Xr->set((GLfloat)cache->fRegion.first * 512);
        }
        if (fUniforms->Zr.get() != nullptr) {
            fUniforms->Zr->set((GLfloat)cache->fRegion.second * 512);
        }
        if (fUniforms->width.get() != nullptr) {
            fUniforms->width->set((GLfloat)width);
        }
        if (fUniforms->height.get() != nullptr) {
            fUniforms->height->set((GLfloat)height);
        }
        if (fUniforms->Cx.get() != nullptr) {
            fUniforms->Cx->set((GLfloat)lookAt.fX);
        }
        if (fUniforms->Cz.get() != nullptr) {
            fUniforms->Cz->set((GLfloat)lookAt.fZ);
        }
        if (fUniforms->grassBlockId.get() != nullptr) {
            fUniforms->grassBlockId->set((GLint)mcfile::blocks::minecraft::grass_block);
        }
        if (fUniforms->foliageBlockId) {
            fUniforms->foliageBlockId->set((GLint)mcfile::blocks::minecraft::oak_leaves);
        }
        if (fUniforms->netherrackBlockId) {
            fUniforms->netherrackBlockId->set((GLint)mcfile::blocks::minecraft::netherrack);
        }
        if (fUniforms->waterOpticalDensity) {
            fUniforms->waterOpticalDensity->set((GLfloat)fWaterOpticalDensity.get());
        }
        if (fUniforms->waterTranslucent) {
            fUniforms->waterTranslucent->set((GLboolean)fWaterTranslucent.get());
        }
        if (fUniforms->biomeBlend) {
            fUniforms->biomeBlend->set((GLint)fBiomeBlend.get());
        }
        if (fUniforms->enableBiome) {
            fUniforms->enableBiome->set((GLboolean)fEnableBiome.get());
        }
        if (fUniforms->background) {
            if (fDimension == Dimension::TheEnd) {
                fUniforms->background->set(13.0f / 255.0f, 10.0f / 255.0f, 18.0f / 255.0f, 1.0f);
            } else {
                fUniforms->background->set(0.0f, 0.0f, 0.0f, 0.0f);
            }
        }

        if (fUniforms->fade.get() != nullptr) {
            if (enableUI) {
                int const ms = (int)Clamp(now.toMilliseconds() - cache->fLoadTime.toMilliseconds(), 0LL, (int64)kFadeDurationMS);
                GLfloat const a = ms > kFadeDurationMS ? 1.0f : CubicEaseInOut((float)ms / (float)kFadeDurationMS, 0.0f, 1.0f, 1.0f);
                fUniforms->fade->set(a);
            } else {
                fUniforms->fade->set(1.0f);
            }
        }

        fOpenGLContext.extensions.glActiveTexture(GL_TEXTURE0);
        cache->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fUniforms->texture.get() != nullptr) {
            fUniforms->texture->set(0);
        }

        int const x = it.first.first;
        int const z = it.first.second;

        auto const& north = textures.find(MakeRegion(x, z - 1));
        if (north != textures.end()) {
            fOpenGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 1);
            north->second->fTexture->bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            if (fUniforms->north.get() != nullptr) {
                fUniforms->north->set(1);
            }
        }

        auto const& northEast = textures.find(MakeRegion(x + 1, z - 1));
        if (northEast != textures.end()) {
            fOpenGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 2);
            northEast->second->fTexture->bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            if (fUniforms->northEast.get() != nullptr) {
                fUniforms->northEast->set(2);
            }
        }

        auto const& east = textures.find(MakeRegion(x + 1, z));
        if (east != textures.end()) {
            fOpenGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 3);
            east->second->fTexture->bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            if (fUniforms->east.get() != nullptr) {
                fUniforms->east->set(3);
            }
        }

        auto const& southEast = textures.find(MakeRegion(x + 1, z + 1));
        if (southEast != textures.end()) {
            fOpenGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 4);
            southEast->second->fTexture->bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            if (fUniforms->southEast.get() != nullptr) {
                fUniforms->southEast->set(4);
            }
        }

        auto const& south = textures.find(MakeRegion(x, z + 1));
        if (south != textures.end()) {
            fOpenGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 5);
            south->second->fTexture->bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            if (fUniforms->south.get() != nullptr) {
                fUniforms->south->set(5);
            }
        }

        auto const& southWest = textures.find(MakeRegion(x - 1, z + 1));
        if (southWest != textures.end()) {
            fOpenGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 6);
            southWest->second->fTexture->bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            if (fUniforms->southWest.get() != nullptr) {
                fUniforms->southWest->set(6);
            }
        }

        auto const& west = textures.find(MakeRegion(x - 1, z));
        if (west != textures.end()) {
            fOpenGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 7);
            west->second->fTexture->bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            if (fUniforms->west.get() != nullptr) {
                fUniforms->west->set(7);
            }
        }

        auto const& northWest = textures.find(MakeRegion(x - 1, z - 1));
        if (northWest != textures.end()) {
            fOpenGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 8);
            northWest->second->fTexture->bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            if (fUniforms->northWest.get() != nullptr) {
                fUniforms->northWest->set(8);
            }
        }

        fOpenGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, fBuffer->vBuffer);
		fOpenGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fBuffer->iBuffer);
        
        fAttributes->enable(fOpenGLContext);
        glDrawElements(GL_QUADS, Buffer::kNumPoints, GL_UNSIGNED_INT, nullptr);
        fAttributes->disable(fOpenGLContext);
    }

    instantiateTextures(lookAt);
}

void MapViewComponent::drawBackground()
{
    Point<int> size = fSize.get();
    const int width = size.x;
    const int height = size.y;
    const float desktopScale = (float)fOpenGLContext.getRenderingScale();

    std::unique_ptr<LowLevelGraphicsContext> glRenderer(createOpenGLGraphicsContext(fOpenGLContext,
                                                                                    roundToInt(desktopScale * width),
                                                                                    roundToInt(desktopScale * height)));

    if (glRenderer.get() == nullptr) {
        return;
    }

    LookAt current = clampedLookAt();

    Graphics g(*glRenderer);
    g.addTransform(AffineTransform::scale(desktopScale, desktopScale));

    Point<float> mapOriginPx = getViewCoordinateFromMap({0.0f, 0.0f}, current);
    int const xoffset = int(floor(mapOriginPx.x)) % (2 * kCheckeredPatternSize);
    int const yoffset = int(floor(mapOriginPx.y)) % (2 * kCheckeredPatternSize);

    g.setColour(Colour::fromRGB(236, 236, 236));
    
    const int w = width / kCheckeredPatternSize + 5;
    const int h = height / kCheckeredPatternSize + 5;
    for (int j = 0; j < h; j++) {
        const int y = (j - 2) * kCheckeredPatternSize + yoffset;
        for (int i = (j % 2 == 0 ? 0 : 1); i < w; i += 2) {
            const int x = (i - 2) * kCheckeredPatternSize + xoffset;
            g.fillRect(x, y, kCheckeredPatternSize, kCheckeredPatternSize);
        }
    }
    g.setColour(Colour::fromRGB(245, 245, 245));
    for (int i = 0; i < w; i++) {
        const int x = (i - 2) * kCheckeredPatternSize + xoffset;
        g.drawVerticalLine(x, 0, height);
    }
    for (int j = 0; j < h; j++) {
        const int y = (j - 2) * kCheckeredPatternSize + yoffset;
        g.drawHorizontalLine(y, 0, width);
    }
    
    fLoadingRegionsLock.enter();
    std::set<Region> loadingRegions(fLoadingRegions);
    fLoadingRegionsLock.exit();
    g.setColour(Colour::fromRGBA(0, 0, 0, 37));
    for (Region region : loadingRegions) {
        int const x = region.first * 512;
        int const z = region.second * 512;
        Point<float> topLeft = getViewCoordinateFromMap(Point<float>(x, z), current);
        float const regionSize = 512.0f / current.fBlocksPerPixel;
        g.fillRect(topLeft.x, topLeft.y, regionSize, regionSize);
    }
}

void MapViewComponent::openGLContextClosing()
{
    fTextures.clear();
    fOpenGLContext.extensions.glDeleteBuffers(1, &fBuffer->vBuffer);
    fOpenGLContext.extensions.glDeleteBuffers(1, &fBuffer->iBuffer);
}

float MapViewComponent::DistanceSqBetweenRegionAndLookAt(LookAt lookAt, Region region)
{
    float const regionCenterX = region.first * 512 - 256;
    float const regionCenterZ = region.second * 512 - 256;
    float const dx = regionCenterX - lookAt.fX;
    float const dz = regionCenterZ - lookAt.fZ;
    return dx * dx + dz * dz;
}

void MapViewComponent::setWorldDirectory(File directory, Dimension dim)
{
    if (fWorldDirectory.getFullPathName() == directory.getFullPathName() && fDimension == dim) {
        return;
    }

    fRegionUpdateChecker->setDirectory(directory, dim);

    fOverworld->setEnabled(dim != Dimension::Overworld);
    fNether->setEnabled(dim != Dimension::TheNether);
    fEnd->setEnabled(dim != Dimension::TheEnd);
    
    fLoadingFinished = false;
    fCaptureButton->setEnabled(false);

    File worldDataFile = WorldData::WorldDataPath(directory);
    WorldData data = WorldData::Load(worldDataFile);
    
    fOpenGLContext.executeOnGLThread([this](OpenGLContext&) {
        std::unique_ptr<ThreadPool> prev(fPool.release());
        fPool.reset(CreateThreadPool());
        prev->removeAllJobs(true, -1);
        fJobs.clear();
        fTextures.clear();
    }, true);

    {
        ScopedLock lk(fLoadingRegionsLock);

        LookAt const lookAt = fLookAt.get();

        fLoadingRegions.clear();
        fWorldDirectory = directory;
        fDimension = dim;
        fWorldData = data;
        fPinComponents.clear();
        for (auto const& p : fWorldData.fPins) {
            addPinComponent(p);
        }

        int minX = 0;
        int maxX = 0;
        int minZ = 0;
        int maxZ = 0;

        DirectoryIterator it(DimensionDirectory(fWorldDirectory, fDimension), false, "*.mca");
        std::vector<File> files;
        while (it.next()) {
            File f = it.getFile();
            auto r = mcfile::Region::MakeRegion(f.getFullPathName().toStdString());
            if (!r) {
                continue;
            }
            minX = std::min(minX, r->fX);
            maxX = std::max(maxX, r->fX);
            minZ = std::min(minZ, r->fZ);
            maxZ = std::max(maxZ, r->fZ);
            files.push_back(f);
        }
        
        LookAt next = lookAt;
        next.fX = 0;
        next.fZ = 0;
        fVisibleRegions = Rectangle<int>(minX, minZ, maxX - minX + 1, maxZ - minZ + 1);
        setLookAt(next);

        std::sort(files.begin(), files.end(), [next](File const& a, File const& b) {
            auto rA = mcfile::Region::MakeRegion(a.getFullPathName().toStdString());
            auto rB = mcfile::Region::MakeRegion(b.getFullPathName().toStdString());
            auto distanceA = DistanceSqBetweenRegionAndLookAt(next, MakeRegion(rA->fX, rA->fZ));
            auto distanceB = DistanceSqBetweenRegionAndLookAt(next, MakeRegion(rB->fX, rB->fZ));
            return distanceA < distanceB;
        });
        
        queueTextureLoading(files, dim, true);
    }
}

void MapViewComponent::queueTextureLoading(std::vector<File> files, Dimension dim, bool useCache)
{
    if (files.empty()) {
        return;
    }

    if (OpenGLContext::getCurrentContext() == &fOpenGLContext) {
        queueTextureLoadingImpl(fOpenGLContext, files, dim, useCache);
    } else {
        fOpenGLContext.executeOnGLThread([this, files, dim, useCache](OpenGLContext &ctx) {
            queueTextureLoadingImpl(ctx, files, dim, useCache);
        }, false);
    }
}

void MapViewComponent::queueTextureLoadingImpl(OpenGLContext &ctx, std::vector<File> files, Dimension dim, bool useCache)
{
    fLoadingRegionsLock.enter();
    defer {
        fLoadingRegionsLock.exit();
    };
    fLoadingFinished = false;

    Rectangle<int> visibleRegions = fVisibleRegions.get();
    int minX = visibleRegions.getX();
    int maxX = visibleRegions.getRight();
    int minY = visibleRegions.getY();
    int maxY = visibleRegions.getBottom();

    for (File const& f : files) {
        auto r = mcfile::Region::MakeRegion(f.getFullPathName().toStdString());
        RegionToTexture* job = new RegionToTexture(f, MakeRegion(r->fX, r->fZ), dim, useCache);
        fJobs.emplace_back(job);
        fPool->addJob(job, false);
        fLoadingRegions.insert(job->fRegion);
        minX = std::min(minX, r->fX);
        maxX = std::max(maxX, r->fX + 1);
        minY = std::min(minY, r->fZ);
        maxY = std::max(maxY, r->fZ + 1);
    }
    
    fVisibleRegions = Rectangle<int>(minX, minY, maxX - minX, maxY - minY);
    
    ctx.setContinuousRepainting(true);
}

Point<float> MapViewComponent::getMapCoordinateFromView(Point<float> p, LookAt lookAt) const
{
    Point<int> size = fSize.get();
    float const width = size.x;
    float const height = size.y;
    float const bx = lookAt.fX + (p.x - width / 2) * lookAt.fBlocksPerPixel;
    float const bz = lookAt.fZ + (p.y - height / 2) * lookAt.fBlocksPerPixel;
    return Point<float>(bx, bz);
}

Point<float> MapViewComponent::getViewCoordinateFromMap(Point<float> p, LookAt lookAt) const
{
    Point<int> size = fSize.get();
    float const width = size.x;
    float const height = size.y;
    float const x = (p.x - lookAt.fX) / lookAt.fBlocksPerPixel + width / 2;
    float const y = (p.y - lookAt.fZ) / lookAt.fBlocksPerPixel + height / 2;
    return Point<float>(x, y);
}

Point<float> MapViewComponent::getMapCoordinateFromView(Point<float> p) const
{
    LookAt const current = clampedLookAt();
    return getMapCoordinateFromView(p, current);
}

Point<float> MapViewComponent::getViewCoordinateFromMap(Point<float> p) const
{
    LookAt const current = clampedLookAt();
    return getViewCoordinateFromMap(p, current);
}

void MapViewComponent::magnify(Point<float> p, float rate)
{
    LookAt const current = clampedLookAt();
    LookAt next = current;

    next.fBlocksPerPixel = (std::min)((std::max)(current.fBlocksPerPixel / rate, kMinScale), kMaxScale);

    float const width = getWidth();
    float const height = getHeight();
    Point<float> const pivot = getMapCoordinateFromView(p);
    float const dx = (p.x - width / 2);
    float const dz = (p.y - height / 2);
    next.fX = pivot.x - dx * next.fBlocksPerPixel;
    next.fZ = pivot.y - dz * next.fBlocksPerPixel;

    setLookAt(next);

    triggerRepaint();
}

void MapViewComponent::mouseMagnify(MouseEvent const& event, float scaleFactor)
{
    magnify(event.position, scaleFactor);
}

void MapViewComponent::mouseWheelMove(MouseEvent const& event, MouseWheelDetails const& wheel)
{
    float factor = 1.0f + wheel.deltaY;
    magnify(event.position, factor);
}

void MapViewComponent::mouseDrag(MouseEvent const& event)
{
    if (event.mods.isRightButtonDown()) {
        return;
    }
    LookAt const current = clampedLookAt();
    float const dx = event.getDistanceFromDragStartX() * current.fBlocksPerPixel;
    float const dy = event.getDistanceFromDragStartY() * current.fBlocksPerPixel;
    LookAt next = current;
    next.fX = fCenterWhenDragStart.x - dx;
    next.fZ = fCenterWhenDragStart.y - dy;
    setLookAt(next);

    fMouse = event.position;

    triggerRepaint();

    fLastDragPosition.push_back(event);
    if (fLastDragPosition.size() > 2) {
        fLastDragPosition.pop_front();
    }
}

void MapViewComponent::mouseDown(MouseEvent const& e)
{
    if (e.mods.isRightButtonDown()) {
        return;
    }
    LookAt const current = clampedLookAt();
    fCenterWhenDragStart = Point<float>(current.fX, current.fZ);
    
    fScrollerTimer.stopTimer();
}

void MapViewComponent::mouseMove(MouseEvent const& event)
{

    fMouse = event.position;
    triggerRepaint();
}

class TextInputDialog : public Component {
public:
    TextInputDialog()
    {
        fInputLabel.reset(new Label());
        fInputLabel->setEditable(true);
        String name
#if JUCE_WINDOWS
            = "Yu Gothic UI";
#else
            = "Hiragino Kaku Gothic Pro";
#endif
        Font font(name, 14, 0);
        fInputLabel->setFont(font);
        fInputLabel->setColour(Label::ColourIds::backgroundColourId, Colours::white);
        fInputLabel->setColour(Label::ColourIds::textColourId, Colours::black);
        fInputLabel->setColour(Label::ColourIds::textWhenEditingColourId, Colours::black);
        fInputLabel->setJustificationType(Justification::topLeft);
        addAndMakeVisible(*fInputLabel);
        fOkButton.reset(new TextButton());
        fOkButton->setButtonText("OK");
        fOkButton->onClick = [this]() {
            close(1);
        };
        addAndMakeVisible(*fOkButton);
        fCancelButton.reset(new TextButton());
        fCancelButton->setButtonText(TRANS("Cancel"));
        fCancelButton->onClick = [this]() {
            close(-1);
        };
        addAndMakeVisible(*fCancelButton);
        setSize(300, 160);
    }
    
    void resized() override
    {
        int const pad = 20;
        int const width = getWidth();
        int const height = getHeight();
        int const buttonWidth = 100;
        int const buttonHeight = 40;
        int const inputHeight = height - 3 * pad - buttonHeight;
        
        if (fInputLabel) {
            fInputLabel->setBounds(pad, pad, width - 2 * pad, inputHeight);
        }
        if (fOkButton) {
            fOkButton->setBounds(width - pad - buttonWidth - pad - buttonWidth, pad + inputHeight + pad, buttonWidth, buttonHeight);
        }
        if (fCancelButton) {
            fCancelButton->setBounds(width - pad - buttonWidth, pad + inputHeight + pad, buttonWidth, buttonHeight);
        }
    }

    static std::pair<int, String> show(Component *target, String title, String init)
    {
        std::unique_ptr<TextInputDialog> component(new TextInputDialog());
        component->fInputLabel->setText(init, dontSendNotification);
        DialogWindow::showModalDialog(title, component.get(), target, target->getLookAndFeel().findColour(TextButton::buttonColourId), true);
        return std::make_pair(component->fResultMenuId, component->fInputLabel->getText());
    }
    
private:
    void close(int result)
    {
        fResultMenuId = result;
        Component *pivot = this;
        while (pivot) {
            DialogWindow *dlg = dynamic_cast<DialogWindow *>(pivot);
            if (dlg) {
                dlg->closeButtonPressed();
                return;
            }
            pivot = pivot->getParentComponent();
        }
    }
    
private:
    String fMessage;
    String fResult;
    int fResultMenuId = -1;
    
    std::unique_ptr<Label> fInputLabel;
    std::unique_ptr<TextButton> fOkButton;
    std::unique_ptr<TextButton> fCancelButton;
};

void MapViewComponent::mouseUp(MouseEvent const& e)
{
    if (e.mods.isRightButtonDown()) {
        mouseRightClicked(e);
        return;
    }
    if (fLastDragPosition.size() != 2) {
        return;
    }

    MouseEvent p0 = fLastDragPosition.front();
    MouseEvent p1 = fLastDragPosition.back();
    fLastDragPosition.clear();
    
    float const dt = (p1.eventTime.toMilliseconds() - p0.eventTime.toMilliseconds()) / 1000.0f;
    if (dt <= 0.0f) {
        return;
    }
    float const dx = p1.x - p0.x;
    float const dz = p1.y - p0.y;
    float const vx = -dx / dt;
    float const vz = -dz / dt;
    LookAt current = clampedLookAt();

    Rectangle<int> visible = fVisibleRegions.get();
    fScroller.fling(current.fX / current.fBlocksPerPixel, current.fZ / current.fBlocksPerPixel,
                    vx, vz,
                    visible.getX() * 512 / current.fBlocksPerPixel, visible.getRight() * 512 / current.fBlocksPerPixel,
                    visible.getY() * 512 / current.fBlocksPerPixel, visible.getBottom() * 512 / current.fBlocksPerPixel);
    fScrollerTimer.stopTimer();
    fScrollerTimer.startTimerHz(50);
}

void MapViewComponent::mouseRightClicked(MouseEvent const& e)
{
    if (!fWorldDirectory.exists()) {
        return;
    }
    LookAt current = clampedLookAt();
    Dimension dim = fDimension;

    PopupMenu menu;
    menu.addItem(1, TRANS("Put a pin here"));
    Point<int> pos = e.getScreenPosition();
    int menuId = menu.showAt(Rectangle<int>(pos, pos));
    if (menuId != 1) {
        return;
    }
    auto result = TextInputDialog::show(this, TRANS("Please enter a pin name"), "");
    if (result.first != 1) {
        return;
    }
    String message = result.second;
    Point<float> pinPos = getMapCoordinateFromView(e.getPosition().toFloat(), current);
    std::shared_ptr<Pin> p = std::make_shared<Pin>();
    p->fX = floor(pinPos.x);
    p->fZ = floor(pinPos.y) + 1;
    p->fDim = dim;
    p->fMessage = message;
    addPinComponent(p);
    fWorldData.fPins.push_back(p);
    saveWorldData();
    triggerRepaint();
}

void MapViewComponent::addPinComponent(std::shared_ptr<Pin> pin)
{
    PinComponent *pinComponent = new PinComponent(pin);
    pinComponent->updatePinPosition(getViewCoordinateFromMap(pinComponent->getMapCoordinate()));
    pinComponent->onRightClick = [this](std::shared_ptr<Pin> pin, Point<int> screenPos) {
        handlePinRightClicked(pin, screenPos);
    };
    pinComponent->onDoubleClick = [this](std::shared_ptr<Pin> pin, Point<int> screenPos) {
        handlePinDoubleClicked(pin, screenPos);
    };
    addAndMakeVisible(pinComponent);
    fPinComponents.emplace_back(pinComponent);
}

void MapViewComponent::handlePinRightClicked(std::shared_ptr<Pin> const& pin, Point<int> screenPos)
{
    PopupMenu menu;
    menu.addItem(1, TRANS("Delete") + " \"" + pin->fMessage + "\"");
    menu.addItem(2, TRANS("Rename") + " \"" + pin->fMessage + "\"");
    int menuId = menu.showAt(Rectangle<int>(screenPos, screenPos));
    if (menuId == 1) {
        for (auto it = fPinComponents.begin(); it != fPinComponents.end(); it++) {
            if (!(*it)->isPresenting(pin)) {
                continue;
            }
            removeChildComponent(it->get());
            fPinComponents.erase(it);
            break;
        }
        fWorldData.fPins.erase(std::remove_if(fWorldData.fPins.begin(), fWorldData.fPins.end(), [pin](auto it) {
            return it.get() == pin.get();
        }), fWorldData.fPins.end());
        saveWorldData();
        triggerRepaint();
    } else if (menuId == 2) {
        auto result = TextInputDialog::show(this, TRANS("Please enter a pin name"), pin->fMessage);
        if (result.first != 1) {
            return;
        }
        String message = result.second;
        pin->fMessage = message;
        saveWorldData();
        triggerRepaint();
    }
}

void MapViewComponent::handlePinDoubleClicked(std::shared_ptr<Pin> const& pin, Point<int> screenPos)
{
    auto result = TextInputDialog::show(this, TRANS("Please enter a pin name"), pin->fMessage);
    if (result.first != 1) {
        return;
    }
    String message = result.second;
    pin->fMessage = message;
    saveWorldData();
    triggerRepaint();
}

void MapViewComponent::updateAllPinComponentPosition()
{
    LookAt const lookAt = clampedLookAt();
    for (auto const& pin : fPinComponents) {
        pin->updatePinPosition(getViewCoordinateFromMap(Point<float>(pin->getMapCoordinate()), lookAt));
    }
}

void MapViewComponent::saveWorldData()
{
    File f = WorldData::WorldDataPath(fWorldDirectory);
    fWorldData.save(f);
}

void MapViewComponent::changeListenerCallback(ChangeBroadcaster *source)
{
    updateAllPinComponentPosition();
}

ThreadPool* MapViewComponent::CreateThreadPool()
{
    auto const threads = (std::max)(1, (int)std::thread::hardware_concurrency() - 1);
    return new ThreadPool(threads);
}

void MapViewComponent::triggerRepaint()
{
    repaint();
    fOpenGLContext.triggerRepaint();
}

void MapViewComponent::resized()
{
    int const width = getWidth();
    fSize = Point<int>(getWidth(), getHeight());
    
    {
        int y = kMargin;
        fBrowserOpenButton->setBounds(kMargin, y, kButtonSize, kButtonSize); y += kButtonSize;
        y += kMargin;
        fOverworld->setBounds(kMargin, y, kButtonSize, kButtonSize); y += kButtonSize;
        y += kMargin;
        fNether->setBounds(kMargin, y, kButtonSize, kButtonSize); y += kButtonSize;
        y += kMargin;
        fEnd->setBounds(kMargin, y, kButtonSize, kButtonSize); y += kButtonSize;
        y += kMargin;
    }
    
    {
        fCaptureButton->setBounds(width - kButtonSize - kMargin, kMargin, kButtonSize, kButtonSize);
        fSettingsButton->setBounds(width - kButtonSize - kMargin, kMargin * 2 + kButtonSize, kButtonSize, kButtonSize);
    }
}

void MapViewComponent::setBrowserOpened(bool opened)
{
    if (opened) {
        fBrowserOpenButton->setImages(fBrowserOpenButtonImageClose.get());
    } else {
        fBrowserOpenButton->setImages(fBrowserOpenButtonImageOpen.get());
    }
}

Rectangle<int> MapViewComponent::regionBoundingBox()
{
    int minX, maxX, minZ, maxZ;
    minX = maxX = fTextures.begin()->first.first;
    minZ = maxZ = fTextures.begin()->first.second;
    for (auto const& it : fTextures) {
        auto region = it.first;
        int const x = region.first;
        int const z = region.second;
        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
        minZ = std::min(minZ, z);
        maxZ = std::max(maxZ, z);
    }
    return Rectangle<int>(minX, minZ, maxX - minX, maxZ - minZ);
}

class SavePNGProgressWindow : public ThreadWithProgressWindow {
public:
    SavePNGProgressWindow(MapViewComponent *component, OpenGLContext &openGLContext, File file)
        : ThreadWithProgressWindow(TRANS("Writing image file"), true, false)
        , fComponent(component)
        , fOpenGLContext(openGLContext)
        , fFile(file)
    {
    }
    
    void run()
    {
        Rectangle<int> bounds;
        fOpenGLContext.executeOnGLThread([this, &bounds](OpenGLContext&) {
            bounds = fComponent->regionBoundingBox();
        }, true);
        
        if (bounds.getWidth() == 0 || bounds.getHeight() == 0) {
            return;
        }
        
        int const minBlockX = bounds.getX() * 512;
        int const minBlockZ = bounds.getY() * 512;
        int const maxBlockX = (bounds.getRight() + 1) * 512 - 1;
        int const maxBlockZ = (bounds.getBottom() + 1) * 512 - 1;
        
        int const width = maxBlockX - minBlockX + 1;
        int const height = maxBlockZ - minBlockZ + 1;

        int const kMaxMemoryUsage = 32 * 1024 * 1024;
        int const row = std::max(kMaxMemoryUsage / ((int)sizeof(PixelARGB) * width), 128);
        
        std::vector<PixelARGB> pixels(width * row);
        PixelARGB *pixelsPtr = pixels.data();
        int const numFrames = (int)ceil(height / float(row));

        FileOutputStream stream(fFile);
        stream.truncate();
        stream.setPosition(0);
        PNGWriter writer(width, height, stream);

        fOpenGLContext.executeOnGLThread([this, minBlockX, maxBlockX, minBlockZ, maxBlockZ, width, height, pixelsPtr, row, numFrames, &writer](OpenGLContext& ctx) {
            int y = 0;
            for (int i = 0; i < numFrames; i++) {
                std::fill_n(pixelsPtr, width, PixelARGB());
                
                std::unique_ptr<OpenGLFrameBuffer> buffer(new OpenGLFrameBuffer());
                buffer->initialise(ctx, width, row);
                buffer->makeCurrentRenderingTarget();
                
                MapViewComponent::LookAt lookAt;
                lookAt.fX = minBlockX + width / 2.0f;
                lookAt.fZ = minBlockZ + i * row + row / 2.0f;
                lookAt.fBlocksPerPixel = 1;
                fComponent->render(width, row, lookAt, false);
                
                buffer->readPixels(pixelsPtr, Rectangle<int>(0, 0, width, row));
                buffer->releaseAsRenderingTarget();
                
                buffer->release();
                buffer.reset();
                
                for (int j = row; --j >= 0;) {
                    writer.writeRow(pixelsPtr + width * j);
                    y++;
                    setProgress(y / float(height));
                    if (y >= height) {
                        break;
                    }
                }
            }
        }, true);
        
        setProgress(1);
    }
    
private:
    MapViewComponent *fComponent;
    OpenGLContext &fOpenGLContext;
    File fFile;
};

void MapViewComponent::captureToImage()
{
    fCaptureButton->setEnabled(false);
    defer {
        fCaptureButton->setEnabled(true);
    };
    
    File file;
    {
        FileChooser dialog(TRANS("Choose file name"), File(), "*.png", true);
        if (!dialog.browseForFileToSave(true)) {
            return;
        }
        file = dialog.getResult();
    }
    
    SavePNGProgressWindow wnd(this, fOpenGLContext, file);
    wnd.runThread();
}

void MapViewComponent::handleAsyncUpdate()
{
    fCaptureButton->setEnabled(fLoadingFinished.get());
}

void MapViewComponent::timerCallback()
{
    fOpenGLContext.executeOnGLThread([this](OpenGLContext &ctx) {
        if (fPool->getNumJobs() || !fLoadingFinished.get()) {
            return;
        }
        ctx.setContinuousRepainting(false);
    }, false);
    stopTimer();
}

void MapViewComponent::setWaterOpticalDensity(float v)
{
    fWaterOpticalDensity = v;
    triggerRepaint();
}

void MapViewComponent::setWaterTranslucent(bool translucent)
{
    fWaterTranslucent = translucent;
    triggerRepaint();
}

void MapViewComponent::setBiomeEnable(bool enable)
{
    fEnableBiome = enable;
    triggerRepaint();
}

void MapViewComponent::setBiomeBlend(int blend)
{
    fBiomeBlend = blend;
    triggerRepaint();
}

MapViewComponent::LookAt MapViewComponent::clampedLookAt() const
{
    return clampLookAt(fLookAt.get());
}

MapViewComponent::LookAt MapViewComponent::clampLookAt(LookAt l) const
{
    Rectangle<int> visibleRegions = fVisibleRegions.get();

    if (visibleRegions.getWidth() == 0 && visibleRegions.getHeight() == 0) {
        return l;
    }

    float const minX = visibleRegions.getX() * 512;
    float const minZ = visibleRegions.getY() * 512;
    float const maxX = visibleRegions.getRight() * 512;
    float const maxZ = visibleRegions.getBottom() * 512;

    l.fX = std::min(std::max(l.fX, minX), maxX);
    l.fZ = std::min(std::max(l.fZ, minZ), maxZ);

    return l;
}

void MapViewComponent::setLookAt(LookAt next)
{
    fLookAt = clampLookAt(next);
    updateAllPinComponentPosition();
}

MapViewComponent::RegionUpdateChecker::RegionUpdateChecker(MapViewComponent* comp)
    : Thread("RegionUpdateChecker")
    , fDim(Dimension::Overworld)
    , fMapView(comp)
{
}

void MapViewComponent::RegionUpdateChecker::run()
{
    std::map<std::string, int64_t> updated;
    
    while (!currentThreadShouldExit()) {
        try {
            Thread::sleep(1000);
            checkUpdatedFiles(updated);
        } catch (...) {
        }
    }
}

void MapViewComponent::RegionUpdateChecker::setDirectory(File f, Dimension dim)
{
	ScopedLock lk(fSection);
	fDirectory = f;
    fDim = dim;
}

void MapViewComponent::RegionUpdateChecker::checkUpdatedFiles(std::map<std::string, int64_t> &updated)
{
    File d;
    Dimension dim;
    {
        ScopedLock lk(fSection);
        d = fDirectory;
        dim = fDim;
    }
    
    if (!d.exists()) {
        return;
    }
    
    File const root = DimensionDirectory(d, dim);
    
    std::map<std::string, int64_t> copy(std::find_if(updated.begin(), updated.end(), [root](auto it) {
        std::string s = it.first;
        String path(s);
        File f(path);
        return f.getParentDirectory().getFullPathName() == root.getFullPathName();
    }), updated.end());
    
    DirectoryIterator it(DimensionDirectory(d, dim), false, "*.mca");
    std::vector<File> files;
    while (it.next()) {
        File f = it.getFile();
        auto r = mcfile::Region::MakeRegion(f.getFullPathName().toStdString());
        if (!r) {
            continue;
        }
        Time modified = f.getLastModificationTime();
        std::string fullpath = f.getFullPathName().toStdString();
        
        auto j = copy.find(fullpath);
        if (j == copy.end()) {
            copy[fullpath] = modified.toMilliseconds();
        } else {
            if (j->second < modified.toMilliseconds()) {
                copy[fullpath] = modified.toMilliseconds();
                files.push_back(f);
            }
        }
    }
    
    copy.swap(updated);
    
    if (!files.empty()) {
        fMapView->queueTextureLoading(files, dim, false);
    }
}
