#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "RegionToTexture.h"
#include "RegionTextureCache.h"
#include "OverScroller.hpp"
#include "Dimension.h"
#include <map>
#include <vector>
#include <set>
#include <deque>

class TimerInstance : public Timer
{
public:
    TimerInstance() = default;
    
    std::function<void(TimerInstance &timer)> fTimerCallback;

private:
    void timerCallback() override {
        if (!fTimerCallback) {
            return;
        }
        fTimerCallback(*this);
    }
};

class MapViewComponent : public Component, private OpenGLRenderer, private AsyncUpdater, private Timer
{
public:
    std::function<void()> onOpenButtonClicked;
    std::function<void()> onSettingsButtonClicked;
    
public:
    MapViewComponent();
    ~MapViewComponent();

    void paint(Graphics &g) override;
    void resized() override;
    
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    void mouseMagnify(MouseEvent const& event, float scaleFactor) override;
    void mouseWheelMove(MouseEvent const& event, MouseWheelDetails const& wheel) override;
    void mouseDrag(MouseEvent const& event) override;
    void mouseDown(MouseEvent const& event) override;
    void mouseMove(MouseEvent const& event) override;
    void mouseUp(MouseEvent const& event) override;

    void setWorldDirectory(File directory, Dimension dim);
    void setBrowserOpened(bool opened);

    struct LookAt {
        float fX;
        float fZ;
        float fBlocksPerPixel;
    };
    
    void render(int const width, int const height, LookAt const lookAt, bool enableUI);
    Rectangle<int> regionBoundingBox();

    void handleAsyncUpdate() override;
    void timerCallback() override;
    
    void setWaterAbsorptionCoefficient(float v);
    void setWaterTranslucent(bool translucent);
    void setBiomeEnable(bool enable);
    void setBiomeBlend(int blend);
    
private:
    void updateShader();
    Point<float> getMapCoordinateFromView(Point<float> p) const;
    Point<float> getViewCoordinateFromMap(Point<float> p) const;
    Point<float> getMapCoordinateFromView(Point<float> p, LookAt lookAt) const;
    Point<float> getViewCoordinateFromMap(Point<float> p, LookAt lookAt) const;
    void magnify(Point<float> p, float rate);
    void drawBackground();
    void triggerRepaint();
    void captureToImage();
    LookAt clampLookAt(LookAt lookAt) const;
    LookAt clampedLookAt() const;

    static ThreadPool* CreateThreadPool();
    static float DistanceSqBetweenRegionAndLookAt(LookAt lookAt, mcfile::Region const& region);

    struct Uniforms
    {
        Uniforms (OpenGLContext& openGLContext, OpenGLShaderProgram& shader)
        {
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
            north.reset(createUniform(openGLContext, shader, "north"));
            northEast.reset(createUniform(openGLContext, shader, "norhtEast"));
            east.reset(createUniform(openGLContext, shader, "east"));
            southEast.reset(createUniform(openGLContext, shader, "southEast"));
            south.reset(createUniform(openGLContext, shader, "south"));
            southWest.reset(createUniform(openGLContext, shader, "southWest"));
            west.reset(createUniform(openGLContext, shader, "west"));
            northWest.reset(createUniform(openGLContext, shader, "northWest"));
            waterAbsorptionCoefficient.reset(createUniform(openGLContext, shader, "waterAbsorptionCoefficient"));
            waterTranslucent.reset(createUniform(openGLContext, shader, "waterTranslucent"));
            biomeBlend.reset(createUniform(openGLContext, shader, "biomeBlend"));
            enableBiome.reset(createUniform(openGLContext, shader, "enableBiome"));
        }

        std::unique_ptr<OpenGLShaderProgram::Uniform> texture, fade, heightmap, blocksPerPixel, width, height, Xr, Zr, Cx, Cz, grassBlockId, foliageBlockId;
        std::unique_ptr<OpenGLShaderProgram::Uniform> north, northEast, east, southEast, south, southWest, west, northWest;
        std::unique_ptr<OpenGLShaderProgram::Uniform> waterAbsorptionCoefficient, waterTranslucent, biomeBlend, enableBiome;

    private:
        static OpenGLShaderProgram::Uniform* createUniform(OpenGLContext& openGLContext,
                                                           OpenGLShaderProgram& shader,
                                                           const char* uniformName)
        {
            if (openGLContext.extensions.glGetUniformLocation(shader.getProgramID(), uniformName) < 0) {
                return nullptr;
            }

            return new OpenGLShaderProgram::Uniform (shader, uniformName);
        }
    };

    struct Vertex
    {
        float position[2];
        float texCoord[2];
    };

    struct Attributes
    {
        Attributes (OpenGLContext& openGLContext, OpenGLShaderProgram& shader)
        {
            position.reset(createAttribute(openGLContext, shader, "position"));
            textureCoordIn.reset(createAttribute(openGLContext, shader, "textureCoordIn"));
        }

        void enable (OpenGLContext& openGLContext)
        {
            if (position.get() != nullptr)
            {
                openGLContext.extensions.glVertexAttribPointer (position->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), nullptr);
                openGLContext.extensions.glEnableVertexAttribArray (position->attributeID);
            }

            if (textureCoordIn.get() != nullptr)
            {
                openGLContext.extensions.glVertexAttribPointer (textureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 2));
                openGLContext.extensions.glEnableVertexAttribArray (textureCoordIn->attributeID);
            }
        }

        void disable (OpenGLContext& openGLContext)
        {
            if (position.get() != nullptr)        openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);
            if (textureCoordIn.get() != nullptr)  openGLContext.extensions.glDisableVertexAttribArray (textureCoordIn->attributeID);
        }

        std::unique_ptr<OpenGLShaderProgram::Attribute> position, textureCoordIn;

    private:
        static OpenGLShaderProgram::Attribute* createAttribute (OpenGLContext& openGLContext,
                                                                OpenGLShaderProgram& shader,
                                                                const char* attributeName)
        {
            if (openGLContext.extensions.glGetAttribLocation (shader.getProgramID(), attributeName) < 0)
                return nullptr;

            return new OpenGLShaderProgram::Attribute (shader, attributeName);
        }
    };

    struct Buffer {
        GLuint vBuffer;
        GLuint iBuffer;
        
        static GLsizei const kNumPoints = 4;
    };

private:
    OpenGLContext fOpenGLContext;
    File fWorldDirectory;
    Dimension fDimension;
    std::map<Region, std::shared_ptr<RegionTextureCache>> fTextures;
    std::unique_ptr<OpenGLShaderProgram> fShader;
    std::unique_ptr<Uniforms> fUniforms;
    std::unique_ptr<Attributes> fAttributes;
    std::unique_ptr<Buffer> fBuffer;

    Atomic<LookAt> fLookAt;
    Atomic<Rectangle<int>> fVisibleRegions;

    static float const kMaxScale;
    static float const kMinScale;

    Point<float> fCenterWhenDragStart;

    Point<float> fMouse;
    
    ScopedPointer<ThreadPool> fPool;
    std::vector<std::unique_ptr<RegionToTexture>> fJobs;

    static int constexpr kCheckeredPatternSize = 16;
    
    std::set<Region> fLoadingRegions;
    CriticalSection fLoadingRegionsLock;

    ScopedPointer<DrawableButton> fBrowserOpenButton;
    ScopedPointer<Drawable> fBrowserOpenButtonImageOpen;
    ScopedPointer<Drawable> fBrowserOpenButtonImageClose;

    ScopedPointer<DrawableButton> fOverworld;
    ScopedPointer<Drawable> fOverworldImage;
    
    ScopedPointer<DrawableButton> fNether;
    ScopedPointer<Drawable> fNetherImage;
    
    ScopedPointer<DrawableButton> fEnd;
    ScopedPointer<Drawable> fEndImage;
    
    ScopedPointer<DrawableButton> fCaptureButton;
    ScopedPointer<Drawable> fCaptureButtonImage;
    
    ScopedPointer<DrawableButton> fSettingsButton;
    ScopedPointer<Drawable> fSettingsButtonImage;
    
    ScopedPointer<TooltipWindow> fTooltipWindow;
    
    Atomic<bool> fLoadingFinished;
    
    OverScroller fScroller;
    std::deque<MouseEvent> fLastDragPosition;
    TimerInstance fScrollerTimer;
    
    Atomic<float> fWaterAbsorptionCoefficient;
    Atomic<bool> fWaterTranslucent;
    
    Atomic<bool> fEnableBiome;
    Atomic<int> fBiomeBlend;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MapViewComponent)
};
