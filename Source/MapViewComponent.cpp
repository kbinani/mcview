#include "../JuceLibraryCode/JuceHeader.h"
#include "MapViewComponent.h"
#include "RegionToTexture.h"
#include "RegionTextureCache.h"
#include "OverScroller.hpp"
#include "PNGWriter.h"
#include "SettingsComponent.h"
#include "defer.h"
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
    
    fBrowserOpenButton = new DrawableButton("Browser", DrawableButton::ButtonStyle::ImageOnButtonBackground);
    setBrowserOpened(true);
    fBrowserOpenButton->setSize(kButtonSize, kButtonSize);
    fBrowserOpenButton->onClick = [this]() {
        onOpenButtonClicked();
    };
    addAndMakeVisible(fBrowserOpenButton);

    fOverworldImage = Drawable::createFromImageData(BinaryData::baseline_landscape_white_18dp_png,
                                                    BinaryData::baseline_landscape_white_18dp_pngSize);
    fOverworld = new DrawableButton("", DrawableButton::ButtonStyle::ImageOnButtonBackground);
    fOverworld->setImages(fOverworldImage);
    fOverworld->onClick = [this]() {
        setWorldDirectory(fWorldDirectory, Dimension::Overworld);
    };
    fOverworld->setEnabled(false);
    fOverworld->setTooltip(TRANS("Overworld"));
    addAndMakeVisible(fOverworld);
    
    fNetherImage = Drawable::createFromImageData(BinaryData::baseline_whatshot_white_18dp_png, BinaryData::baseline_whatshot_white_18dp_pngSize);
    fNether = new DrawableButton("", DrawableButton::ButtonStyle::ImageOnButtonBackground);
    fNether->setImages(fNetherImage);
    fNether->onClick = [this]() {
        setWorldDirectory(fWorldDirectory, Dimension::TheNether);
    };
    fNether->setEnabled(false);
    fNether->setTooltip(TRANS("The Nether"));
    addAndMakeVisible(fNether);
    
    fEndImage = Drawable::createFromImageData(BinaryData::baseline_brightness_3_white_18dp_png, BinaryData::baseline_brightness_3_white_18dp_pngSize);
    fEnd = new DrawableButton("", DrawableButton::ButtonStyle::ImageOnButtonBackground);
    fEnd->setImages(fEndImage);
    fEnd->onClick = [this]() {
        setWorldDirectory(fWorldDirectory, Dimension::TheEnd);
    };
    fEnd->setEnabled(false);
    fEnd->setTooltip(TRANS("The End"));
    addAndMakeVisible(fEnd);
    
    fCaptureButtonImage = Drawable::createFromImageData(BinaryData::baseline_camera_white_18dp_png,
                                                        BinaryData::baseline_camera_white_18dp_pngSize);
    fCaptureButton = new DrawableButton("Capture", DrawableButton::ButtonStyle::ImageOnButtonBackground);
    fCaptureButton->setImages(fCaptureButtonImage);
    fCaptureButton->onClick = [this]() {
        captureToImage();
    };
    addAndMakeVisible(fCaptureButton);
    fCaptureButton->setEnabled(false);

    fSettingsButtonImage = Drawable::createFromImageData(BinaryData::baseline_settings_white_18dp_png,
                                                         BinaryData::baseline_settings_white_18dp_pngSize);
    fSettingsButton = new DrawableButton("Settings", DrawableButton::ButtonStyle::ImageOnButtonBackground);
    fSettingsButton->setImages(fSettingsButtonImage);
    fSettingsButton->onClick = [this]() {
        onSettingsButtonClicked();
    };
    addAndMakeVisible(fSettingsButton);
    
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
        fLookAt = clampLookAt(next);
        triggerRepaint();
    };
    
    fTooltipWindow = new TooltipWindow();
    addAndMakeVisible(fTooltipWindow);
    
    fRegionUpdateChecker = new RegionUpdateChecker(this);
    fRegionUpdateChecker->startThread();
    
    fSize = Point<int>(600, 400);
    setSize (600, 400);
}

MapViewComponent::~MapViewComponent()
{
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
    ScopedPointer<Buffer> buffer = new Buffer();

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
    ScopedPointer<OpenGLShaderProgram> newShader = new OpenGLShaderProgram(fOpenGLContext);

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
        varying vec2 textureCoordOut;
        uniform sampler2D texture;
        uniform float fade;
        uniform int grassBlockId;
        uniform int foliageBlockId;
        uniform int netherrackBlockId;
        uniform float waterOpticalDensity;
        uniform bool waterTranslucent;
        uniform int biomeBlend;
        uniform int enableBiome;
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
        };
    
        BlockInfo pixelInfo(vec4 color) {
            // h:           8bit
            // waterDepth:  8bit
            // biome:       4bit
            // block:      12bit
            int h = int(color.a * 255.0);
            int waterDepth = int(color.r * 255.0);
            int g = int(color.g * 255.0);
            int b = int(color.b * 255.0);
            int biome = g / 16;
            int block = (g - biome * 16) * 256 + b;
            BlockInfo info;
            info.height = float(h);
            info.waterDepth = float(waterDepth);
            info.biomeId = biome;
            info.blockId = block;
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
    for (auto it : RegionToTexture::kBlockToColor) {
        auto id = it.first;
        Colour c = it.second;
        GLfloat r = c.getRed() / 255.0f;
        GLfloat g = c.getGreen() / 255.0f;
        GLfloat b = c.getBlue() / 255.0f;
        fragment << "    if (blockId == " << id << ") {" << std::endl;
        fragment << "        return vec4(float(" << r << "), float(" << g << "), float(" << b << "), 1.0);" << std::endl;
        fragment << "    } else" << std::endl;
    }
    fragment << "    { " << std::endl;
    fragment << "        return vec4(0.0, 0.0, 0.0, 0.0);" << std::endl;
    fragment << "    }" << std::endl;
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
        if (enableBiome == 0) {
            return waterColorFromBiome(-1);
        }
        vec4 sumColor = vec4(0.0, 0.0, 0.0, 0.0);
        vec2 center = textureCoordOut;
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
            vec4 wc = waterColor();
            if (waterTranslucent) {
                float intensity = pow(10.0, -waterOpticalDensity * waterDepth);
                c = vec4(wc.r * intensity, wc.g * intensity, wc.b* intensity, alpha);
            } else {
                c = wc;
            }
        } else if (blockId == foliageBlockId) {
            vec4 lc = foliageColorFromBiome(enableBiome == 0 ? -1 : biomeId);
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

static float CubicEaseInOut(float t, float start, float end, float duration) {
    float b = start;
    float c = end - start;
    t  /= duration / 2.0;
    if (t < 1.0) {
        return c / 2.0 * t * t * t + b;
    } else {
        t = t - 2.0;
        return c / 2.0 * (t* t * t + 2.0) + b;
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

    {
        for (long i = (long)fJobs.size() - 1; i >= 0; i--) {
            auto& job = fJobs[i];
            if (fPool->contains(job.get())) {
                continue;
            }
            fPool->removeJob(job.get(), false, 0);
            ScopedPointer<RegionToTexture> j(job.release());
            fJobs.erase(fJobs.begin() + i);
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
            fLoadingRegionsLock.exit();
            
            break; // load only one textrue per frame
        }
        
        if (fPool->getNumJobs() == 0 && !fLoadingFinished.get()) {
            startTimer(kFadeDurationMS);
            fLoadingFinished = true;
            triggerAsyncUpdate();
        }
    }
    
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
    Rectangle<float> viewport(0, 0, width, height);
    
    for (auto it : textures) {
        auto cache = it.second;

        int const bx = cache->fRegion.first * 512;
        int const bz = cache->fRegion.second * 512;
        Point<float> topLeft = getViewCoordinateFromMap(Point<float>(bx, bz), lookAt);
        Rectangle<float> textureBoundsInView(topLeft.x, topLeft.y, 512.0f / lookAt.fBlocksPerPixel, 512.0f / lookAt.fBlocksPerPixel);
        if (!viewport.intersects(textureBoundsInView)) {
            continue;
        }
        
        if (fUniforms->blocksPerPixel.get() != nullptr) {
            fUniforms->blocksPerPixel->set(lookAt.fBlocksPerPixel);
        }
        if (fUniforms->Xr.get() != nullptr) {
            fUniforms->Xr->set((GLfloat)bx);
        }
        if (fUniforms->Zr.get() != nullptr) {
            fUniforms->Zr->set((GLfloat)bz);
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
            fUniforms->enableBiome->set((GLint)(fEnableBiome.get() ? 1 : 0));
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
                double const seconds = (now.toMilliseconds() - cache->fLoadTime.toMilliseconds()) / 1000.0;
                GLfloat const fadeSeconds = kFadeDurationMS / 1000.0f;
                GLfloat const a = seconds > fadeSeconds ? 1.0f : CubicEaseInOut(seconds / fadeSeconds, 0.0f, 1.0f, 1.0f);
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

float MapViewComponent::DistanceSqBetweenRegionAndLookAt(LookAt lookAt, mcfile::Region const& region)
{
    float const regionCenterX = region.fX * 512 - 256;
    float const regionCenterZ = region.fZ * 512 - 256;
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
    
    fOpenGLContext.executeOnGLThread([this](OpenGLContext&) {
        ScopedPointer<ThreadPool> prev(fPool.release());
        fPool = CreateThreadPool();
        prev->removeAllJobs(true, -1);
        fJobs.clear();
        fTextures.clear();
    }, true);

    {
        ScopedLock lk(fLoadingRegionsLock);

        fLoadingRegions.clear();
        fWorldDirectory = directory;
        fDimension = dim;

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
        
        LookAt const lookAt = fLookAt.get();
        LookAt next = lookAt;
        next.fX = 0;
        next.fZ = 0;
        fLookAt = next;
        fVisibleRegions = Rectangle<int>(minX, minZ, maxX - minX + 1, maxZ - minZ + 1);

        std::sort(files.begin(), files.end(), [next](File const& a, File const& b) {
            auto rA = mcfile::Region::MakeRegion(a.getFullPathName().toStdString());
            auto rB = mcfile::Region::MakeRegion(b.getFullPathName().toStdString());
            auto distanceA = DistanceSqBetweenRegionAndLookAt(next, *rA);
            auto distanceB = DistanceSqBetweenRegionAndLookAt(next, *rB);
            return distanceA < distanceB;
        });
        
        queueTextureLoading(files, true);
    }
}

void MapViewComponent::queueTextureLoading(std::vector<File> files, bool useCache)
{
    if (files.empty()) {
        return;
    }

    ScopedLock lk(fLoadingRegionsLock);
    fLoadingFinished = false;
    
    for (File const& f : files) {
        auto r = mcfile::Region::MakeRegion(f.getFullPathName().toStdString());
        RegionToTexture* job = new RegionToTexture(f, MakeRegion(r->fX, r->fZ), fDimension, useCache);
        fJobs.emplace_back(job);
        fPool->addJob(job, false);
        fLoadingRegions.insert(job->fRegion);
    }
    
    fOpenGLContext.setContinuousRepainting(true);
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

    fLookAt = clampLookAt(next);

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
    LookAt const current = clampedLookAt();
    float const dx = event.getDistanceFromDragStartX() * current.fBlocksPerPixel;
    float const dy = event.getDistanceFromDragStartY() * current.fBlocksPerPixel;
    LookAt next = current;
    next.fX = fCenterWhenDragStart.x - dx;
    next.fZ = fCenterWhenDragStart.y - dy;
    next = clampLookAt(next);
    fLookAt = next;

    fMouse = event.position;

    triggerRepaint();
    
    fLastDragPosition.push_back(event);
    if (fLastDragPosition.size() > 2) {
        fLastDragPosition.pop_front();
    }
}

void MapViewComponent::mouseDown(MouseEvent const&)
{
    LookAt const current = clampedLookAt();
    fCenterWhenDragStart = Point<float>(current.fX, current.fZ);
    
    fScrollerTimer.stopTimer();
}

void MapViewComponent::mouseMove(MouseEvent const& event)
{
    fMouse = event.position;
    triggerRepaint();
}

void MapViewComponent::mouseUp(MouseEvent const&)
{
    if (fLastDragPosition.size() != 2) {
        return;
    }
    LookAt current = clampedLookAt();

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
    
    Rectangle<int> visible = fVisibleRegions.get();
    fScroller.fling(current.fX / current.fBlocksPerPixel, current.fZ / current.fBlocksPerPixel,
                    vx, vz,
                    visible.getX() * 512 / current.fBlocksPerPixel, visible.getRight() * 512 / current.fBlocksPerPixel,
                    visible.getY() * 512 / current.fBlocksPerPixel, visible.getBottom() * 512 / current.fBlocksPerPixel);
    fScrollerTimer.stopTimer();
    fScrollerTimer.startTimerHz(50);
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
        fBrowserOpenButton->setImages(fBrowserOpenButtonImageClose);
    } else {
        fBrowserOpenButton->setImages(fBrowserOpenButtonImageOpen);
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
                
                ScopedPointer<OpenGLFrameBuffer> buffer = new OpenGLFrameBuffer();
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
    fOpenGLContext.executeOnGLThread([](OpenGLContext &ctx) {
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

MapViewComponent::RegionUpdateChecker::RegionUpdateChecker(MapViewComponent* comp)
    : Thread("RegionUpdateChecker")
    , fMapView(comp)
{
}

void MapViewComponent::RegionUpdateChecker::run()
{
    while (!currentThreadShouldExit()) {
        Thread::sleep(1000);
        File d;
        Dimension dim;
        fSection.enter();
        d = fDirectory;
        dim = fDim;
        fSection.exit();
        
        if (!d.exists()) {
            continue;
        }

        File const root = DimensionDirectory(d, dim);
        
        auto i = fUpdated.begin();
        while (i.next()) {
            File f(i.getKey());
            if (f.getParentDirectory().getFullPathName() != root.getFullPathName()) {
                fUpdated.remove(i.getKey());
            }
        }
        
        DirectoryIterator it(DimensionDirectory(d, dim), false, "*.mca");
        std::vector<File> files;
        while (it.next()) {
            File f = it.getFile();
            auto r = mcfile::Region::MakeRegion(f.getFullPathName().toStdString());
            if (!r) {
                continue;
            }
            Time modified = f.getLastModificationTime();
            String fullpath = f.getFullPathName();
            if (fUpdated.contains(fullpath)) {
                if (fUpdated[fullpath].toMilliseconds() < modified.toMilliseconds()) {
                    fUpdated.set(fullpath, modified);
                    files.push_back(f);
                }
            } else {
                fUpdated.set(fullpath, modified);
            }
        }
        
        if (!files.empty()) {
            fMapView->queueTextureLoading(files, false);
        }
    }
}

void MapViewComponent::RegionUpdateChecker::setDirectory(File f, Dimension dim)
{
    fSection.enter();
    defer {
        fSection.exit();
    };
    fDirectory = f;
    fDim = dim;
}
