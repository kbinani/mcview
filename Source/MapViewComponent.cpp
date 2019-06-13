#include "../JuceLibraryCode/JuceHeader.h"
#include "MapViewComponent.h"
#include <set>
#include <cassert>
#include <minecraft-file.hpp>

float const MapViewComponent::kMaxScale = 5;
float const MapViewComponent::kMinScale = 1.0f / 32.0f;

MapViewComponent::MapViewComponent()
    : fBlocksPerPixel(1.0f)
    , fCenter(Point<float>(0, 0))
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
        uniform float scaleX;
        uniform float scaleZ;
        uniform float tX;
        uniform float tZ;
        uniform float texX;
        uniform float texZ;
        varying vec2 textureCoordOut;
        void main() {
            textureCoordOut = textureCoordIn;
            float blockX = position.x * 512.0 + texX;
            float blockZ = -position.y * 512.0 + texZ;
            float x = (blockX - tX) * scaleX;
            float y = (blockZ - tZ) * scaleZ;
            gl_Position = vec4(x, -y, position.z, position.w);
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

Matrix3D<float> MapViewComponent::getProjectionMatrix() const
{
    auto w = 1.0f;
    auto h = w * getLocalBounds().toFloat().getAspectRatio(false);

    return Matrix3D<float>::fromFrustum(-w, w, -h, h, 4.0f, 30.0f);
}

void MapViewComponent::renderOpenGL()
{
    OpenGLHelpers::clear(Colours::black);

    float const width = getWidth();
    float const height = getHeight();
    auto desktopScale = (float)fOpenGLContext.getRenderingScale();
    glViewport(0, 0, roundToInt(desktopScale * width), roundToInt(desktopScale * height));

    {
        std::vector<Region> positions = { MakeRegion(0, 0) }; //, MakeRegion(-1, 0), MakeRegion(-1, -1), MakeRegion(0, -1)};
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

    float blocksPerPixel = fBlocksPerPixel.get();
    Point<float> center = fCenter.get();
    
    GLfloat scaleX = 2 / (blocksPerPixel * getWidth());
    GLfloat scaleZ = 2 / (blocksPerPixel * getHeight());
    GLfloat tX = center.x;
    GLfloat tZ = center.y;

    for (auto it : fTextures) {
        auto cache = it.second;
        if (fUniforms->scaleX.get() != nullptr) {
            fUniforms->scaleX->set(scaleX);
        }
        if (fUniforms->scaleZ.get() != nullptr) {
            fUniforms->scaleZ->set(scaleZ);
        }
        if (fUniforms->tX.get() != nullptr) {
            fUniforms->tX->set(tX);
        }
        if (fUniforms->tZ.get() != nullptr) {
            fUniforms->tZ->set(tZ);
        }
        if (fUniforms->texX.get() != nullptr) {
            fUniforms->texX->set((GLfloat)cache->fRegion.first * 512);
        }
        if (fUniforms->texZ.get() != nullptr) {
            fUniforms->texZ->set((GLfloat)cache->fRegion.second * 512);
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

void MapViewComponent::mouseMagnify(MouseEvent const& event, float scaleFactor)
{
    float const current = fBlocksPerPixel.get();
    fBlocksPerPixel.set(std::min(std::max(current / scaleFactor, kMinScale), kMaxScale));
}

void MapViewComponent::mouseWheelMove(MouseEvent const& event, MouseWheelDetails const& wheel)
{
    float factor = 1.0f + wheel.deltaY;
    float const current = fBlocksPerPixel.get();
    fBlocksPerPixel.set(std::min(std::max(current / factor, kMinScale), kMaxScale));
}

void MapViewComponent::mouseDrag(MouseEvent const& event)
{
    float const current = fBlocksPerPixel.get();
    float const dx = event.getDistanceFromDragStartX() * current;
    float const dy = event.getDistanceFromDragStartY() * current;
    fCenter.set(Point<float>(fCenterWhenDragStart.x - dx, fCenterWhenDragStart.y - dy));
}

void MapViewComponent::mouseDown(MouseEvent const& event)
{
    fCenterWhenDragStart = fCenter.get();
}
