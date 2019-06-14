#include "../JuceLibraryCode/JuceHeader.h"
#include "MapViewComponent.h"
#include <set>
#include <cassert>
#include <minecraft-file.hpp>

float const MapViewComponent::kMaxScale = 5;
float const MapViewComponent::kMinScale = 1.0f / 32.0f;

MapViewComponent::MapViewComponent()
    : fLookAt({0, 0, 1})
{
    if (auto* peer = getPeer()) {
        peer->setCurrentRenderingEngine (0);
    }

    setOpaque(true);
    fOpenGLContext.setRenderer(this);
    fOpenGLContext.attachTo(*this);
    fOpenGLContext.setContinuousRepainting(true);

    setSize (600, 400);
    
    //TODO:
    setRegionsDirectory(File("/Users/kbinani/Library/Application Support/minecraft/saves/2434/region"));
}

MapViewComponent::~MapViewComponent()
{
    fOpenGLContext.detach();
}

void MapViewComponent::paint(Graphics& g)
{
    const int width = getWidth();
    const int height = getHeight();
    Point<float> block = getMapCoordinateFromView(fMouse);
    g.setColour(Colours::white);
    g.drawText(String::formatted("pixel:[%.2f, %.2f] block:[%.2f, %.2f]", fMouse.x, fMouse.y, block.x, block.y), 0, 0, width, height, Justification::topLeft);
}

void MapViewComponent::newOpenGLContextCreated()
{
    ScopedPointer<Buffer> buffer = new Buffer();

    fOpenGLContext.extensions.glGenBuffers(1, &buffer->vBuffer);
	fOpenGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, buffer->vBuffer);
    std::vector<Vertex> vertices = {
        {{0, 0}, {0.0, 1.0}},
        {{1, 0}, {1.0, 1.0}},
        {{1, 1}, {1.0, 0.0}},
        {{0, 1}, {0.0, 0.0}},
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

    newShader->addFragmentShader(R"#(
        varying vec2 textureCoordOut;
        uniform sampler2D texture;
        uniform sampler2D heightmap;
        uniform float fade;
        void main() {
            vec4 color = texture2D(texture, textureCoordOut);
            float height = texture2D(heightmap, textureCoordOut).a;
//            float alpha = height * fade;
            float alpha = fade;
            gl_FragColor = vec4(color.r * alpha, color.g * alpha, color.b * alpha, 1.0);
        }
    )#");

    newShader->link();
    newShader->use();

    fUniforms.reset(new Uniforms(fOpenGLContext, *newShader));
    fAttributes.reset(new Attributes(fOpenGLContext, *newShader));

    fShader.reset(newShader.release());
}

void MapViewComponent::renderOpenGL()
{
    OpenGLHelpers::clear(Colours::black);

    float const width = getWidth();
    float const height = getHeight();
    auto desktopScale = (float)fOpenGLContext.getRenderingScale();
    glViewport(0, 0, roundToInt(desktopScale * width), roundToInt(desktopScale * height));

    {
        std::vector<Region> positions = { MakeRegion(0, 0), MakeRegion(-1, 0), MakeRegion(-1, -1), MakeRegion(0, -1)};
        for (auto pos : positions) {
            auto texture = fTextures.find(pos);
            if (texture == fTextures.end()) {
                File f = fRegionsDirectory.getChildFile(String::formatted("r.%d.%d.mca", pos.first, pos.second));
                auto cache = std::make_shared<RegionTextureCache>(pos, f.getFullPathName());
                cache->loadIfNeeded();
                fTextures.insert(std::make_pair(pos, cache));
            }
        }
    }
    Time const now = Time::getCurrentTime();

    if (fShader.get() == nullptr) {
        updateShader();
    }

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

        fOpenGLContext.extensions.glActiveTexture(GL_TEXTURE0);
        cache->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        if (fUniforms->texture.get() != nullptr) {
            fUniforms->texture->set(0);
        }

        fOpenGLContext.extensions.glActiveTexture(GL_TEXTURE1);
        cache->fHeightmap->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        if (fUniforms->heightmap.get() != nullptr) {
            fUniforms->heightmap->set(1);
        }

        if (fUniforms->fade.get() != nullptr) {
            double const seconds = (now.toMilliseconds() - cache->fLoadTime.toMilliseconds()) / 1000.0;
            GLfloat const fadeSeconds = 0.6f;
            GLfloat a = seconds > fadeSeconds ? 1.0f : seconds / fadeSeconds;
            fUniforms->fade->set(a);
        }

		fOpenGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, fBuffer->vBuffer);
		fOpenGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fBuffer->iBuffer);
        
        fAttributes->enable(fOpenGLContext);
        glDrawElements(GL_QUADS, Buffer::kNumPoints, GL_UNSIGNED_INT, nullptr);
        fAttributes->disable(fOpenGLContext);
    }

#if 0
    DirectoryIterator it(fRegionsDirectory, false, "*.mca");
    std::set<String> existing;

    int minX = 1, maxX = 0, minZ = 1, maxZ = 0;
    for (auto it = fTextures.begin(); it != fTextures.end(); it++) {
        Region region = it->first;
        String name = RegionFileName(region);
        existing.insert(name);
        minX = std::min(minX, region.first);
        maxX = std::max(maxX, region.first);
        minZ = std::min(minZ, region.second);
        maxZ = std::max(maxZ, region.second);
    }

    while (it.next()) {
        File f = it.getFile();
        if (existing.find(f.getFileName()) != existing.end()) {
            continue;
        }
        auto r = mcfile::Region::MakeRegion(f.getFullPathName().toStdString());
        if (!r) {
            continue;
        }
        auto region = MakeRegion(r->fX, r->fZ);
        auto cache = std::make_shared<RegionTextureCache>(region, f.getFullPathName());
        cache->loadIfNeeded();
        fTextures.insert(std::make_pair(region, cache));

        minX = std::min(minX, region.first);
        maxX = std::max(maxX, region.first);
        minZ = std::min(minZ, region.second);
        maxZ = std::max(maxZ, region.second);
    }
#endif
}

void MapViewComponent::openGLContextClosing()
{
    fTextures.clear();
	fOpenGLContext.extensions.glDeleteBuffers(1, &fBuffer->vBuffer);
	fOpenGLContext.extensions.glDeleteBuffers(1, &fBuffer->iBuffer);
}

void MapViewComponent::setRegionsDirectory(File directory)
{
    if (fRegionsDirectory.getFullPathName() == directory.getFullPathName()) {
        return;
    }
    fOpenGLContext.detach();
    fTextures.clear();
    fRegionsDirectory = directory;
    fOpenGLContext.attachTo(*this);
}

Point<float> MapViewComponent::getMapCoordinateFromView(Point<float> p) const
{
    LookAt const current = fLookAt.get();
    float const width = getWidth();
    float const height = getHeight();
    float const bx = current.fX + (p.x - width / 2) * current.fBlocksPerPixel;
    float const bz = current.fZ + (p.y - height / 2) * current.fBlocksPerPixel;
    return Point<float>(bx, bz);
}

void MapViewComponent::magnify(Point<float> p, float rate)
{
    LookAt const current = fLookAt.get();
    LookAt next = current;

    next.fBlocksPerPixel = std::min(std::max(current.fBlocksPerPixel / rate, kMinScale), kMaxScale);

    float const width = getWidth();
    float const height = getHeight();
    Point<float> const pivot = getMapCoordinateFromView(p);
    float const dx = (p.x - width / 2);
    float const dz = (p.y - height / 2);
    next.fX = pivot.x - dx * next.fBlocksPerPixel;
    next.fZ = pivot.y - dz * next.fBlocksPerPixel;

    fLookAt.set(next);
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
}

void MapViewComponent::mouseDown(MouseEvent const& event)
{
    LookAt const current = fLookAt.get();
    fCenterWhenDragStart = Point<float>(current.fX, current.fZ);
}

void MapViewComponent::mouseMove(MouseEvent const& event)
{
    fMouse = event.position;
    repaint();
}
