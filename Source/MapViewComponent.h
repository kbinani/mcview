#pragma once

#include "Dimension.h"
#include "OverScroller.hpp"
#include "PinComponent.h"
#include "RegionTextureCache.h"
#include "RegionToTexture.h"
#include "TimerInstance.h"
#include "WorldData.h"
#include <deque>
#include <map>
#include <memory>
#include <set>
#include <variant>
#include <vector>

class MapViewComponent : public juce::Component, private juce::OpenGLRenderer, private juce::AsyncUpdater, private juce::Timer, public juce::ChangeListener, public RegionToTexture::Delegate {
  struct AsyncUpdateQueueUpdateCaptureButtonStatus {};
  struct AsyncUpdateQueueReleaseGarbageThreadPool {};

  using AsyncUpdateQueue = std::variant<AsyncUpdateQueueUpdateCaptureButtonStatus, AsyncUpdateQueueReleaseGarbageThreadPool>;

public:
  std::function<void()> onOpenButtonClicked;
  std::function<void()> onSettingsButtonClicked;

public:
  MapViewComponent();
  ~MapViewComponent();

  void paint(juce::Graphics &g) override;
  void resized() override;

  void newOpenGLContextCreated() override;
  void renderOpenGL() override;
  void openGLContextClosing() override;

  void mouseMagnify(juce::MouseEvent const &event, float scaleFactor) override;
  void mouseWheelMove(juce::MouseEvent const &event, juce::MouseWheelDetails const &wheel) override;
  void mouseDrag(juce::MouseEvent const &event) override;
  void mouseDown(juce::MouseEvent const &event) override;
  void mouseMove(juce::MouseEvent const &event) override;
  void mouseUp(juce::MouseEvent const &event) override;

  void changeListenerCallback(juce::ChangeBroadcaster *) override;

  void regionToTextureDidFinishJob(std::shared_ptr<RegionToTexture::Result> result) override;

  void setWorldDirectory(juce::File directory, Dimension dim);
  void queueTextureLoading(std::vector<juce::File> files, Dimension dim, bool useCache);

  void setBrowserOpened(bool opened);

  struct LookAt {
    float fX;
    float fZ;
    float fBlocksPerPixel;
  };

  void render(int const width, int const height, LookAt const lookAt, bool enableUI);
  juce::Rectangle<int> regionBoundingBox();

  void handleAsyncUpdate() override;
  void timerCallback() override;

  void setWaterOpticalDensity(float v);
  void setWaterTranslucent(bool translucent);
  void setBiomeEnable(bool enable);
  void setBiomeBlend(int blend);
  void setShowPin(bool show);

private:
  void updateShader();
  juce::Point<float> getMapCoordinateFromView(juce::Point<float> p) const;
  juce::Point<float> getViewCoordinateFromMap(juce::Point<float> p) const;
  juce::Point<float> getMapCoordinateFromView(juce::Point<float> p, LookAt lookAt) const;
  juce::Point<float> getViewCoordinateFromMap(juce::Point<float> p, LookAt lookAt) const;
  void magnify(juce::Point<float> p, float rate);
  void drawBackground();
  void triggerRepaint();
  void captureToImage();
  LookAt clampLookAt(LookAt lookAt) const;
  LookAt clampedLookAt() const;
  void setLookAt(LookAt next);
  void queueTextureLoadingImpl(juce::OpenGLContext &ctx, std::vector<juce::File> files, juce::File worldDirectory, Dimension dim, bool useCache);
  void instantiateTextures(LookAt lookAt);
  void mouseRightClicked(juce::MouseEvent const &e);
  void saveWorldData();
  void addPinComponent(std::shared_ptr<Pin>);
  void updateAllPinComponentPosition();
  void handlePinRightClicked(std::shared_ptr<Pin> const &, juce::Point<int> screenPos);
  void handlePinDoubleClicked(std::shared_ptr<Pin> const &, juce::Point<int> screenPos);
  void handlePinDrag(std::shared_ptr<Pin> const &, juce::Point<int> screenPos);
  void resetPinComponents();

  static juce::ThreadPool *CreateThreadPool();
  static float DistanceSqBetweenRegionAndLookAt(LookAt lookAt, Region region);

  struct Uniforms {
    Uniforms(juce::OpenGLContext &openGLContext, juce::OpenGLShaderProgram &shader) {
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
    }

    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> texture, fade, heightmap, blocksPerPixel, width, height, Xr, Zr, Cx, Cz, grassBlockId, foliageBlockId, netherrackBlockId, dimension;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> north, northEast, east, southEast, south, southWest, west, northWest;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> waterOpticalDensity, waterTranslucent, biomeBlend, enableBiome;

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

  struct Vertex {
    float position[2];
    float texCoord[2];
  };

  struct Attributes {
    Attributes(juce::OpenGLContext &openGLContext, juce::OpenGLShaderProgram &shader) {
      position.reset(createAttribute(openGLContext, shader, "position"));
      textureCoordIn.reset(createAttribute(openGLContext, shader, "textureCoordIn"));
    }

    void enable(juce::OpenGLContext &openGLContext) {
      using namespace juce::gl;

      if (position.get() != nullptr) {
        openGLContext.extensions.glVertexAttribPointer(position->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
        openGLContext.extensions.glEnableVertexAttribArray(position->attributeID);
      }

      if (textureCoordIn.get() != nullptr) {
        openGLContext.extensions.glVertexAttribPointer(textureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(sizeof(float) * 2));
        openGLContext.extensions.glEnableVertexAttribArray(textureCoordIn->attributeID);
      }
    }

    void disable(juce::OpenGLContext &openGLContext) {
      if (position.get() != nullptr)
        openGLContext.extensions.glDisableVertexAttribArray(position->attributeID);
      if (textureCoordIn.get() != nullptr)
        openGLContext.extensions.glDisableVertexAttribArray(textureCoordIn->attributeID);
    }

    std::unique_ptr<juce::OpenGLShaderProgram::Attribute> position, textureCoordIn;

  private:
    static juce::OpenGLShaderProgram::Attribute *createAttribute(juce::OpenGLContext &openGLContext,
                                                                 juce::OpenGLShaderProgram &shader,
                                                                 const char *attributeName) {
      if (openGLContext.extensions.glGetAttribLocation(shader.getProgramID(), attributeName) < 0)
        return nullptr;

      return new juce::OpenGLShaderProgram::Attribute(shader, attributeName);
    }
  };

  struct Buffer {
    GLuint vBuffer;
    GLuint iBuffer;

    static GLsizei const kNumPoints = 4;
  };

  class RegionUpdateChecker : public juce::Thread {
  public:
    explicit RegionUpdateChecker(MapViewComponent *component);
    void run() override;
    void setDirectory(juce::File f, Dimension dim);

  private:
    void checkUpdatedFiles(std::map<std::string, int64_t> &updated);

  private:
    juce::CriticalSection fSection;
    juce::File fDirectory;
    Dimension fDim;
    MapViewComponent *const fMapView;
  };

private:
  juce::OpenGLContext fGLContext;
  juce::File fWorldDirectory;
  WorldData fWorldData;

  std::vector<std::unique_ptr<PinComponent>> fPinComponents;
  bool fShowPin;

  Dimension fDimension;
  std::map<Region, std::shared_ptr<RegionTextureCache>> fTextures;
  std::unique_ptr<juce::OpenGLShaderProgram> fGLShader;
  std::unique_ptr<Uniforms> fGLUniforms;
  std::unique_ptr<Attributes> fGLAttributes;
  std::unique_ptr<Buffer> fGLBuffer;

  std::atomic<LookAt> fLookAt;
  std::atomic<juce::Rectangle<int>> fVisibleRegions;
  std::atomic<juce::Point<int>> fSize;

  static float const kMaxScale;
  static float const kMinScale;

  juce::Point<float> fCenterWhenDragStart;

  juce::Point<float> fMouse;

  std::unique_ptr<juce::ThreadPool> fPool;
  std::deque<std::unique_ptr<juce::ThreadPool>> fPoolTomb;
  std::deque<std::shared_ptr<RegionToTexture::Result>> fGLJobResults;

  static int constexpr kCheckeredPatternSize = 16;

  std::set<Region> fLoadingRegions;
  std::mutex fMut;

  std::unique_ptr<juce::DrawableButton> fBrowserOpenButton;
  std::unique_ptr<juce::Drawable> fBrowserOpenButtonImageOpen;
  std::unique_ptr<juce::Drawable> fBrowserOpenButtonImageClose;

  std::unique_ptr<juce::DrawableButton> fOverworld;
  std::unique_ptr<juce::Drawable> fOverworldImage;

  std::unique_ptr<juce::DrawableButton> fNether;
  std::unique_ptr<juce::Drawable> fNetherImage;

  std::unique_ptr<juce::DrawableButton> fEnd;
  std::unique_ptr<juce::Drawable> fEndImage;

  std::unique_ptr<juce::DrawableButton> fCaptureButton;
  std::unique_ptr<juce::Drawable> fCaptureButtonImage;

  std::unique_ptr<juce::DrawableButton> fSettingsButton;
  std::unique_ptr<juce::Drawable> fSettingsButtonImage;

  std::unique_ptr<juce::TooltipWindow> fTooltipWindow;

  juce::Atomic<bool> fLoadingFinished;

  OverScroller fScroller;
  std::deque<juce::MouseEvent> fLastDragPosition;
  TimerInstance fScrollerTimer;
  TimerInstance fAnimationTimer;

  juce::Atomic<float> fWaterOpticalDensity;
  juce::Atomic<bool> fWaterTranslucent;

  juce::Atomic<bool> fEnableBiome;
  juce::Atomic<int> fBiomeBlend;

  std::unique_ptr<RegionUpdateChecker> fRegionUpdateChecker;

  std::unique_ptr<juce::FileChooser> fFileChooser;
  std::deque<AsyncUpdateQueue> fAsyncUpdateQueue;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MapViewComponent)
};
