#include "../JuceLibraryCode/JuceHeader.h"
#include "MapViewComponent.h"
#include "RegionToTexture.h"
#include "RegionTextureCache.h"
#include "OverScroller.hpp"
#include "PNGWriter.h"
#include <set>
#include <cassert>
#include <cmath>
#include <thread>
#include <minecraft-file.hpp>
#include <colormap/colormap.h>

float const MapViewComponent::kMaxScale = 10;
float const MapViewComponent::kMinScale = 1.0f / 32.0f;

static int const kButtonSize = 40;
static int const kFadeDurationMS = 300;

MapViewComponent::MapViewComponent()
    : fLookAt({0, 0, 5})
    , fPool(CreateThreadPool())
    , fMouseDragAmount({0, 0})
    , fLoadingFinished(true)
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

    fCaptureButtonImage = Drawable::createFromImageData(BinaryData::baseline_camera_white_18dp_png,
                                                        BinaryData::baseline_camera_white_18dp_pngSize);
    fCaptureButton = new DrawableButton("Capture", DrawableButton::ButtonStyle::ImageOnButtonBackground);
    fCaptureButton->setImages(fCaptureButtonImage);
    fCaptureButton->onClick = [this]() {
        captureToImage();
    };
    addAndMakeVisible(fCaptureButton);
    fCaptureButton->setEnabled(false);

    setOpaque(true);
    fOpenGLContext.setRenderer(this);
    fOpenGLContext.attachTo(*this);

    fScrollerTimer.fTimerCallback = [this](TimerInstance &timer) {
        if (!fScroller.computeScrollOffset()) {
            timer.stopTimer();
        }
        LookAt next = fLookAt.get();
        next.fX = fScroller.getCurrX() * next.fBlocksPerPixel;
        next.fZ = fScroller.getCurrY() * next.fBlocksPerPixel;
        fLookAt = next;
        fOpenGLContext.triggerRepaint();
    };
    
    setSize (600, 400);
}

MapViewComponent::~MapViewComponent()
{
    fOpenGLContext.detach();
    fPool->removeAllJobs(true, -1);
}

void MapViewComponent::paint(Graphics& g)
{
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
	fragment << "varying vec2 textureCoordOut;" << std::endl;
	fragment << "uniform sampler2D texture;" << std::endl;
	fragment << "uniform float fade;" << std::endl;
	fragment << "uniform int grassBlockId;" << std::endl;
    
    fragment << "uniform sampler2D north;" << std::endl;
    fragment << "uniform sampler2D west;" << std::endl;

	fragment << altitude.getSource() << std::endl;
    
    fragment << "vec4 colorFromBlockId(int blockId) {" << std::endl;
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

    fragment << R"#(
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
    )#";
    
	fragment << "void main() {" << std::endl;

	fragment << R"#(
        vec4 waterColor = vec4(69.0 / 255.0, 91.0 / 255.0, 211.0 / 255.0, 1.0);
        float waterDiffusion = 0.02;

        float alpha = fade;

        vec4 color = texture2D(texture, textureCoordOut);

        float height = color.a * 255.0;
        float waterDepth = color.r * 255.0;
        int hi = int(color.g * 255.0);
        int lo = int(color.b * 255.0);
        int blockId = hi * 256 + lo;

        vec4 c;
        if (waterDepth > 0.0) {
            float intensity = pow(10.0, -waterDiffusion * waterDepth);
            c = vec4(waterColor.r * intensity, waterColor.g * intensity, waterColor.b * intensity, alpha);
        } else if (blockId == grassBlockId) {
            float v = (height - 63.0) / 193.0;
            vec4 g = colormap(v);
            c = vec4(g.r, g.g, g.b, alpha);
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
    LookAt lookAt = fLookAt.get();
    auto desktopScale = (float)fOpenGLContext.getRenderingScale();
    int const width = getWidth() * desktopScale;
    int const height = getHeight() * desktopScale;
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
            RegionToTexture* j = job.release();
            fJobs.erase(fJobs.begin() + i);
            auto cache = std::make_shared<RegionTextureCache>(j->fRegion, j->fRegionFile.getFullPathName());
            cache->load(j->fPixels);
            fTextures.insert(std::make_pair(j->fRegion, cache));
            delete j;
            
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

    for (auto it : fTextures) {
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

        auto north = fTextures.find(MakeRegion(x, z - 1));
        if (north != fTextures.end()) {
            fOpenGLContext.extensions.glActiveTexture(GL_TEXTURE1);
            north->second->fTexture->bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            if (fUniforms->north.get() != nullptr) {
                fUniforms->north->set(1);
            }
        }

        auto west = fTextures.find(MakeRegion(x - 1, z));
        if (west != fTextures.end()) {
            fOpenGLContext.extensions.glActiveTexture(GL_TEXTURE2);
            west->second->fTexture->bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            if (fUniforms->west.get() != nullptr) {
                fUniforms->west->set(2);
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
    const int width = getWidth();
    const int height = getHeight();
    const float desktopScale = (float)fOpenGLContext.getRenderingScale();

    std::unique_ptr<LowLevelGraphicsContext> glRenderer(createOpenGLGraphicsContext(fOpenGLContext,
                                                                                    roundToInt(desktopScale * width),
                                                                                    roundToInt(desktopScale * height)));

    if (glRenderer.get() == nullptr) {
        return;
    }

    Graphics g(*glRenderer);
    g.addTransform(AffineTransform::scale(desktopScale, desktopScale));

    g.setColour(Colour::fromRGB(236, 236, 236));
    int const xoffset = fMouseDragAmount.x % (2 * kCheckeredPatternSize);
    int const yoffset = fMouseDragAmount.y % (2 * kCheckeredPatternSize);

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
    
    LookAt current = fLookAt.get();
    
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

void MapViewComponent::setRegionsDirectory(File directory)
{
    if (fRegionsDirectory.getFullPathName() == directory.getFullPathName()) {
        return;
    }

    fLoadingFinished = false;
    fCaptureButton->setEnabled(false);
    
    fOpenGLContext.executeOnGLThread([this](OpenGLContext&) {
        ScopedPointer<ThreadPool> prev(fPool.release());
        fPool = CreateThreadPool();
        prev->removeAllJobs(true, -1);
        fJobs.clear();
        fTextures.clear();
    }, true);

    fLoadingRegionsLock.enter();

    fLoadingRegions.clear();
    fRegionsDirectory = directory;

    DirectoryIterator it(fRegionsDirectory, false, "*.mca");
    std::vector<File> files;
    while (it.next()) {
        File f = it.getFile();
        auto r = mcfile::Region::MakeRegion(f.getFullPathName().toStdString());
        if (!r) {
            continue;
        }
        files.push_back(f);
    }
    
    LookAt lookAt = fLookAt.get();
    std::sort(files.begin(), files.end(), [lookAt](File const& a, File const& b) {
        auto rA = mcfile::Region::MakeRegion(a.getFullPathName().toStdString());
        auto rB = mcfile::Region::MakeRegion(b.getFullPathName().toStdString());
        auto distanceA = DistanceSqBetweenRegionAndLookAt(lookAt, *rA);
        auto distanceB = DistanceSqBetweenRegionAndLookAt(lookAt, *rB);
        return distanceA < distanceB;
    });
    
    for (File const& f : files) {
        auto r = mcfile::Region::MakeRegion(f.getFullPathName().toStdString());
        RegionToTexture* job = new RegionToTexture(f, MakeRegion(r->fX, r->fZ));
        fJobs.emplace_back(job);
        fPool->addJob(job, false);
        fLoadingRegions.insert(job->fRegion);
    }
    
    fLoadingRegionsLock.exit();
    
    if (fPool->getNumJobs() > 0) {
        fOpenGLContext.setContinuousRepainting(true);
    }
}

Point<float> MapViewComponent::getMapCoordinateFromView(Point<float> p, LookAt lookAt) const
{
    float const width = getWidth();
    float const height = getHeight();
    float const bx = lookAt.fX + (p.x - width / 2) * lookAt.fBlocksPerPixel;
    float const bz = lookAt.fZ + (p.y - height / 2) * lookAt.fBlocksPerPixel;
    return Point<float>(bx, bz);
}

Point<float> MapViewComponent::getViewCoordinateFromMap(Point<float> p, LookAt lookAt) const
{
    float const width = getWidth();
    float const height = getHeight();
    float const x = (p.x - lookAt.fX) / lookAt.fBlocksPerPixel + width / 2;
    float const y = (p.y - lookAt.fZ) / lookAt.fBlocksPerPixel + height / 2;
    return Point<float>(x, y);
}

Point<float> MapViewComponent::getMapCoordinateFromView(Point<float> p) const
{
    LookAt const current = fLookAt.get();
    return getMapCoordinateFromView(p, current);
}

Point<float> MapViewComponent::getViewCoordinateFromMap(Point<float> p) const
{
    LookAt const current = fLookAt.get();
    return getViewCoordinateFromMap(p, current);
}

void MapViewComponent::magnify(Point<float> p, float rate)
{
    LookAt const current = fLookAt.get();
    LookAt next = current;

    next.fBlocksPerPixel = (std::min)((std::max)(current.fBlocksPerPixel / rate, kMinScale), kMaxScale);

    float const width = getWidth();
    float const height = getHeight();
    Point<float> const pivot = getMapCoordinateFromView(p);
    float const dx = (p.x - width / 2);
    float const dz = (p.y - height / 2);
    next.fX = pivot.x - dx * next.fBlocksPerPixel;
    next.fZ = pivot.y - dz * next.fBlocksPerPixel;

    fLookAt.set(next);

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
    LookAt const current = fLookAt.get();
    float const dx = event.getDistanceFromDragStartX() * current.fBlocksPerPixel;
    float const dy = event.getDistanceFromDragStartY() * current.fBlocksPerPixel;
    LookAt next = current;
    next.fX = fCenterWhenDragStart.x - dx;
    next.fZ = fCenterWhenDragStart.y - dy;
    fLookAt.set(next);
    
    fMouseDragAmount.x = (fMouseDragAmountWhenDragStart.x + event.getDistanceFromDragStartX()) % (2 * kCheckeredPatternSize);
    fMouseDragAmount.y = (fMouseDragAmountWhenDragStart.y + event.getDistanceFromDragStartY()) % (2 * kCheckeredPatternSize);

    triggerRepaint();
    
    fLastDragPosition.push_back(event);
    if (fLastDragPosition.size() > 2) {
        fLastDragPosition.pop_front();
    }
}

void MapViewComponent::mouseDown(MouseEvent const& event)
{
    LookAt const current = fLookAt.get();
    fCenterWhenDragStart = Point<float>(current.fX, current.fZ);
    fMouseDragAmountWhenDragStart = fMouseDragAmount;
    
    fScrollerTimer.stopTimer();
}

void MapViewComponent::mouseMove(MouseEvent const& event)
{
    fMouse = event.position;
    triggerRepaint();
}

void MapViewComponent::mouseUp(MouseEvent const& event)
{
    if (fLastDragPosition.size() != 2) {
        return;
    }
    LookAt current = fLookAt.get();

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
    
    int const minX = current.fX / current.fBlocksPerPixel - getWidth();
    int const maxX = current.fX / current.fBlocksPerPixel + getWidth();
    int const minZ = current.fZ / current.fBlocksPerPixel - getHeight();
    int const maxZ = current.fZ / current.fBlocksPerPixel + getHeight();
    fScroller.fling(current.fX / current.fBlocksPerPixel, current.fZ / current.fBlocksPerPixel, vx, vz, minX, maxX, minZ, maxZ);
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
    int const margin = 10;
    int const width = getWidth();
    
    if (fBrowserOpenButton) {
        fBrowserOpenButton->setBounds(margin, margin, kButtonSize, kButtonSize);
    }
    if (fCaptureButton) {
        fCaptureButton->setBounds(width - kButtonSize - margin, margin, kButtonSize, kButtonSize);
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
        : ThreadWithProgressWindow("Writing image file", true, false)
        , fComponent(component)
        , fOpenGLContext(openGLContext)
        , fFile(file)
    {
    }
    
    void run()
    {
        Rectangle<int> bounds;
        fOpenGLContext.executeOnGLThread([this, &bounds](OpenGLContext& ctx) {
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
                
                for (int i = row; --i >= 0;) {
                    writer.writeRow(pixelsPtr + width * i);
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

    File file;
    {
        FileChooser dialog("Choose file name", File(), "*.png", true);
        if (!dialog.browseForFileToSave(true)) {
            return;
        }
        file = dialog.getResult();
    }
    
    SavePNGProgressWindow wnd(this, fOpenGLContext, file);
    wnd.runThread();
    
    fCaptureButton->setEnabled(true);
}

void MapViewComponent::handleAsyncUpdate()
{
    fCaptureButton->setEnabled(fLoadingFinished.get());
}

void MapViewComponent::timerCallback()
{
    fOpenGLContext.setContinuousRepainting(false);
    stopTimer();
}
