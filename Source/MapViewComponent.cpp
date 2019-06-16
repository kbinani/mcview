#include "../JuceLibraryCode/JuceHeader.h"
#include "MapViewComponent.h"
#include "RegionToTexture.h"
#include "RegionTextureCache.h"
#include <set>
#include <cassert>
#include <thread>
#include <minecraft-file.hpp>
#include <colormap/colormap.h>

float const MapViewComponent::kMaxScale = 10;
float const MapViewComponent::kMinScale = 1.0f / 32.0f;

MapViewComponent::MapViewComponent()
    : fLookAt({0, 0, 5})
    , fPool(CreateThreadPool())
    , fMouseDragAmount({0, 0})
{
    if (auto* peer = getPeer()) {
        peer->setCurrentRenderingEngine (0);
    }

    setOpaque(true);
    fOpenGLContext.setRenderer(this);
    fOpenGLContext.attachTo(*this);

    setSize (600, 400);
}

MapViewComponent::~MapViewComponent()
{
    fOpenGLContext.detach();
    fPool->removeAllJobs(true, -1);
}

void MapViewComponent::paint(Graphics& g)
{
    const int width = getWidth();
    const int height = getHeight();
    Point<float> block = getMapCoordinateFromView(fMouse);
    g.setColour(Colours::black);
    g.drawText(String::formatted("pixel:[%.2f, %.2f] block:[%.2f, %.2f]", fMouse.x, fMouse.y, block.x, block.y), 0, 0, width, height, Justification::topLeft);
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
    
        gl_FragColor = c;
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
    OpenGLHelpers::clear(Colours::white);

    float const width = getWidth();
    float const height = getHeight();
    auto desktopScale = (float)fOpenGLContext.getRenderingScale();
    glViewport(0, 0, roundToInt(desktopScale * width), roundToInt(desktopScale * height));

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
            fLoadingRegions.erase(j->fRegion);
            fLoadingRegionsLock.exit();

            break; // load only one textrue per frame
        }
        
        if (fPool->getNumJobs() == 0) {
            fOpenGLContext.setContinuousRepainting(false);
        }
    }
    
    drawBackground();
    
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

    LookAt const lookAt = fLookAt.get();

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
            double const seconds = (now.toMilliseconds() - cache->fLoadTime.toMilliseconds()) / 1000.0;
            GLfloat const fadeSeconds = 0.3f;
            GLfloat a = seconds > fadeSeconds ? 1.0f : seconds / fadeSeconds;
            fUniforms->fade->set(a);
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

    fOpenGLContext.executeOnGLThread([this](OpenGLContext&) {
        fTextures.clear();
    }, true);

    fLoadingRegionsLock.enter();

    fLoadingRegions.clear();
    fRegionsDirectory = directory;
    fPool = CreateThreadPool();

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
}

void MapViewComponent::mouseDown(MouseEvent const& event)
{
    LookAt const current = fLookAt.get();
    fCenterWhenDragStart = Point<float>(current.fX, current.fZ);
    fMouseDragAmountWhenDragStart = fMouseDragAmount;
}

void MapViewComponent::mouseMove(MouseEvent const& event)
{
    fMouse = event.position;
    triggerRepaint();
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
