#pragma once

class MapViewComponent
    : public juce::Component,
      private juce::OpenGLRenderer,
      private juce::AsyncUpdater,
      private juce::Timer,
      public juce::ChangeListener,
      public TexturePackJob::Delegate,
      public SavePNGProgressWindow ::Delegate {
  struct AsyncUpdateQueueUpdateCaptureButtonStatus {};
  struct AsyncUpdateQueueReleaseGarbageThreadPool {};

  using AsyncUpdateQueue = std::variant<AsyncUpdateQueueUpdateCaptureButtonStatus, AsyncUpdateQueueReleaseGarbageThreadPool>;

  static float constexpr kMaxScale = 10;
  static float constexpr kMinScale = 1.0f / 32.0f;
  static int constexpr kCheckeredPatternSize = 16;

  static int constexpr kMargin = 10;
  static int constexpr kButtonSize = 40;
  static int constexpr kFadeDurationMS = 300;
  static int constexpr kScrollUpdateHz = 50;

public:
  std::function<void()> onOpenButtonClicked;
  std::function<void()> onSettingsButtonClicked;

public:
  MapViewComponent()
      : fLookAt({0, 0, 5}), fVisibleRegions({0, 0, 0, 0}), fPool(CreateThreadPool()), fLoadingFinished(true), fWaterOpticalDensity(Settings::kDefaultWaterOpticalDensity), fWaterTranslucent(true), fEnableBiome(true), fBiomeBlend(2) {
    using namespace juce;

    if (auto *peer = getPeer()) {
      peer->setCurrentRenderingEngine(0);
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
    fGLContext.setRenderer(this);
    fGLContext.attachTo(*this);

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

    fAnimationTimer.fTimerCallback = [this](TimerInstance &timer) {
      updateAllPinComponentPosition();
    };

    fTooltipWindow.reset(new TooltipWindow());
    addAndMakeVisible(*fTooltipWindow);

    fRegionUpdateChecker.reset(new RegionUpdateChecker(this));
    fRegionUpdateChecker->startThread();

    Desktop::getInstance().getAnimator().addChangeListener(this);

    fSize = Point<int>(600, 400);
    setSize(600, 400);
  }

  ~MapViewComponent() {
    juce::Desktop::getInstance().getAnimator().removeChangeListener(this);

    fGLContext.detach();
    fPool->removeAllJobs(true, -1);
    for (auto &pool : fPoolTomb) {
      pool->removeAllJobs(true, -1);
    }
    fRegionUpdateChecker->signalThreadShouldExit();
    fRegionUpdateChecker->waitForThreadToExit(-1);
  }

  void paint(juce::Graphics &g) override {
    using namespace juce;
    g.saveState();
    defer {
      g.restoreState();
    };

    int const width = getWidth();
    int const height = getHeight();
    int const lineHeight = 24;
    int const coordLabelWidth = 100;
    int const coordLabelHeight = 2 * lineHeight;

    LookAt const lookAt = fLookAt.load();

    Point<float> const topLeft = getMapCoordinateFromView(Point<float>(0, 0), lookAt);
    Point<float> const bottomRight = getMapCoordinateFromView(Point<float>(width, height), lookAt);
    int const minRegionX = mcfile::Coordinate::RegionFromBlock((int)floor(topLeft.x)) - 1;
    int const maxRegionX = mcfile::Coordinate::RegionFromBlock((int)ceil(bottomRight.x)) + 1;
    int const minRegionZ = mcfile::Coordinate::RegionFromBlock((int)floor(topLeft.y)) - 1;
    int const maxRegionZ = mcfile::Coordinate::RegionFromBlock((int)ceil(bottomRight.y)) + 1;
    int const numRegionsOnDisplay = (maxRegionX - minRegionX + 1) * (maxRegionZ - minRegionZ + 1);

    if (KeyPress::isKeyCurrentlyDown(KeyPress::F1Key)) {
      g.setColour(Colours::black);
      float const thickness = 1;
      // vertical lines
      for (int x = minRegionX; x <= maxRegionX; x++) {
        int const bx = x * 512;
        int const minBz = minRegionZ * 512;
        int const maxBz = maxRegionZ * 512;
        Point<float> const top = getViewCoordinateFromMap(Point<float>(bx, minBz), lookAt);
        Point<float> const bottom = getViewCoordinateFromMap(Point<float>(bx, maxBz), lookAt);
        g.drawLine(top.x, top.y, bottom.x, bottom.y, thickness);
      }
      for (int z = minRegionZ; z <= maxRegionZ; z++) {
        int const bz = z * 512;
        int const minBx = minRegionX * 512;
        int const maxBx = maxRegionX * 512;
        Point<float> const left = getViewCoordinateFromMap(Point<float>(minBx, bz), lookAt);
        Point<float> const right = getViewCoordinateFromMap(Point<float>(maxBx, bz), lookAt);
        g.drawLine(left.x, left.y, right.x, right.y, thickness);
      }
      if (numRegionsOnDisplay < 64) {
        g.setFont(20);
        for (int x = minRegionX; x <= maxRegionX; x++) {
          for (int z = minRegionZ; z <= maxRegionZ; z++) {
            Point<float> const tl = getViewCoordinateFromMap(Point<float>(x * 512, z * 512), lookAt);
            Point<float> const br = getViewCoordinateFromMap(Point<float>((x + 1) * 512, (z + 1) * 512), lookAt);
            GraphicsHelper::DrawFittedText(g,
                                           String::formatted("r.%d.%d.mca", x, z),
                                           tl.x, tl.y,
                                           br.x - tl.x, br.y - tl.y,
                                           Justification::centred, 1);
          }
        }
      }
    }

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

  void resized() override {
    int const width = getWidth();
    fSize = juce::Point<int>(getWidth(), getHeight());

    {
      int y = kMargin;
      fBrowserOpenButton->setBounds(kMargin, y, kButtonSize, kButtonSize);
      y += kButtonSize;
      y += kMargin;
      fOverworld->setBounds(kMargin, y, kButtonSize, kButtonSize);
      y += kButtonSize;
      y += kMargin;
      fNether->setBounds(kMargin, y, kButtonSize, kButtonSize);
      y += kButtonSize;
      y += kMargin;
      fEnd->setBounds(kMargin, y, kButtonSize, kButtonSize);
      y += kButtonSize;
      y += kMargin;
    }

    {
      fCaptureButton->setBounds(width - kButtonSize - kMargin, kMargin, kButtonSize, kButtonSize);
      fSettingsButton->setBounds(width - kButtonSize - kMargin, kMargin * 2 + kButtonSize, kButtonSize, kButtonSize);
    }
  }

  void newOpenGLContextCreated() override {
    using namespace juce::gl;

    std::unique_ptr<Buffer> buffer(new Buffer());

    fGLContext.extensions.glGenBuffers(1, &buffer->vBuffer);
    fGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, buffer->vBuffer);
    std::vector<Vertex> vertices = {
        {{0, 0}, {0.0, 0.0}},
        {{1, 0}, {1.0, 0.0}},
        {{1, 1}, {1.0, 1.0}},
        {{0, 1}, {0.0, 1.0}},
    };
    fGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    fGLContext.extensions.glGenBuffers(1, &buffer->iBuffer);
    fGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->iBuffer);
    std::vector<uint32_t> indices = {0, 1, 2, 3};
    fGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);

    fGLBuffer.reset(buffer.release());
  }

  void renderOpenGL() override {
    LookAt lookAt = clampedLookAt();
    auto desktopScale = (float)fGLContext.getRenderingScale();
    juce::Point<int> size = fSize.load();
    int const width = size.x * desktopScale;
    int const height = size.y * desktopScale;
    lookAt.fBlocksPerPixel /= desktopScale;
    render(width, height, lookAt, true);
  }

  void openGLContextClosing() override {
    fTextures.clear();
    fGLContext.extensions.glDeleteBuffers(1, &fGLBuffer->vBuffer);
    fGLContext.extensions.glDeleteBuffers(1, &fGLBuffer->iBuffer);
  }

  void savePNGProgressWindowRender(int const width, int const height, LookAt const lookAt) override {
    return render(width, height, lookAt, false);
  }

  juce::Rectangle<int> savePNGProgressWindowRegionBoundingBox() override {
    int minX, maxX, minZ, maxZ;
    minX = maxX = fTextures.begin()->first.first;
    minZ = maxZ = fTextures.begin()->first.second;
    for (auto const &it : fTextures) {
      auto region = it.first;
      int const x = region.first;
      int const z = region.second;
      minX = std::min(minX, x);
      maxX = std::max(maxX, x);
      minZ = std::min(minZ, z);
      maxZ = std::max(maxZ, z);
    }
    return juce::Rectangle<int>(minX, minZ, maxX - minX, maxZ - minZ);
  }

  void mouseMagnify(juce::MouseEvent const &event, float scaleFactor) override {
    magnify(event.position, scaleFactor);
  }

  void mouseWheelMove(juce::MouseEvent const &event, juce::MouseWheelDetails const &wheel) override {
    float factor = 1.0f + wheel.deltaY;
    magnify(event.position, factor);
  }

  void mouseDrag(juce::MouseEvent const &event) override {
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

  void mouseDown(juce::MouseEvent const &e) override {
    if (e.mods.isRightButtonDown()) {
      return;
    }
    LookAt const current = clampedLookAt();
    fCenterWhenDragStart = juce::Point<float>(current.fX, current.fZ);

    fScrollerTimer.stopTimer();
  }

  void mouseMove(juce::MouseEvent const &e) override {
    fMouse = e.position;
    triggerRepaint();
  }

  void mouseUp(juce::MouseEvent const &e) override {
    using namespace juce;
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

    Rectangle<int> visible = fVisibleRegions.load();
    fScroller.fling(current.fX / current.fBlocksPerPixel, current.fZ / current.fBlocksPerPixel,
                    vx, vz,
                    visible.getX() * 512 / current.fBlocksPerPixel, visible.getRight() * 512 / current.fBlocksPerPixel,
                    visible.getY() * 512 / current.fBlocksPerPixel, visible.getBottom() * 512 / current.fBlocksPerPixel);
    fScrollerTimer.stopTimer();
    fScrollerTimer.startTimerHz(kScrollUpdateHz);
  }

  void changeListenerCallback(juce::ChangeBroadcaster *source) override {
    auto animator = dynamic_cast<juce::ComponentAnimator *>(source);
    if (!animator) {
      return;
    }
    updateAllPinComponentPosition();
    if (animator->isAnimating()) {
      fAnimationTimer.startTimerHz(kScrollUpdateHz);
    } else {
      fAnimationTimer.stopTimer();
    }
  }

  void regionToTextureDidFinishJob(std::shared_ptr<TexturePackJob::Result> result) override {
    {
      std::lock_guard<std::mutex> lock(fMut);
      fGLJobResults.push_back(result);
      fAsyncUpdateQueue.push_back(AsyncUpdateQueueReleaseGarbageThreadPool{});
    }
    triggerAsyncUpdate();
  }

  void setWorldDirectory(juce::File directory, Dimension dim) {
    using namespace juce;
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

    if (fPool) {
      fPool->removeAllJobs(true, 0);
      fPoolTomb.push_back(std::move(fPool));
    }
    fPool.reset(CreateThreadPool());

    auto garbageTextures = std::make_shared<std::map<Region, std::shared_ptr<RegionTextureCache>>>();
    garbageTextures->swap(fTextures);
    fGLContext.executeOnGLThread([this, garbageTextures](OpenGLContext &) {
      garbageTextures->clear();
    },
                                 true);

    {
      std::lock_guard<std::mutex> lk(fMut);

      LookAt const lookAt = fLookAt.load();

      fLoadingRegions.clear();
      fWorldDirectory = directory;
      fDimension = dim;
      fWorldData = data;
      resetPinComponents();

      int minX = 0;
      int maxX = 0;
      int minZ = 0;
      int maxZ = 0;

      RangedDirectoryIterator it(DimensionDirectory(fWorldDirectory, fDimension), false, "*.mca");
      std::vector<File> files;
      for (DirectoryEntry entry : it) {
        File f = entry.getFile();
        auto r = mcfile::je::Region::MakeRegion(PathFromFile(f));
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

      std::sort(files.begin(), files.end(), [next](File const &a, File const &b) {
        auto rA = mcfile::je::Region::MakeRegion(PathFromFile(a));
        auto rB = mcfile::je::Region::MakeRegion(PathFromFile(b));
        auto distanceA = DistanceSqBetweenRegionAndLookAt(next, MakeRegion(rA->fX, rA->fZ));
        auto distanceB = DistanceSqBetweenRegionAndLookAt(next, MakeRegion(rB->fX, rB->fZ));
        return distanceA < distanceB;
      });

      queueTextureLoading(files, dim, true);
    }
  }

  void queueTextureLoading(std::vector<juce::File> files, Dimension dim, bool useCache) {
    if (files.empty()) {
      return;
    }

    if (juce::OpenGLContext::getCurrentContext() == &fGLContext) {
      queueTextureLoadingImpl(fGLContext, files, fWorldDirectory, dim, useCache);
    } else {
      juce::File worldDirectory = fWorldDirectory;
      fGLContext.executeOnGLThread([this, files, worldDirectory, dim, useCache](juce::OpenGLContext &ctx) {
        queueTextureLoadingImpl(ctx, files, worldDirectory, dim, useCache);
      },
                                   false);
    }
  }

  void setBrowserOpened(bool opened) {
    if (opened) {
      fBrowserOpenButton->setImages(fBrowserOpenButtonImageClose.get());
    } else {
      fBrowserOpenButton->setImages(fBrowserOpenButtonImageOpen.get());
    }
  }

  void render(int const width, int const height, LookAt const lookAt, bool enableUI) {
    using namespace juce;
    using namespace juce::gl;

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

    if (fGLShader.get() == nullptr) {
      updateShader();
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    fGLShader->use();
    auto const &textures = fTextures;

    for (auto it : textures) {
      auto cache = it.second;
      if (fGLUniforms->blocksPerPixel.get() != nullptr) {
        fGLUniforms->blocksPerPixel->set(lookAt.fBlocksPerPixel);
      }
      if (fGLUniforms->Xr.get() != nullptr) {
        fGLUniforms->Xr->set((GLfloat)cache->fRegion.first * 512);
      }
      if (fGLUniforms->Zr.get() != nullptr) {
        fGLUniforms->Zr->set((GLfloat)cache->fRegion.second * 512);
      }
      if (fGLUniforms->width.get() != nullptr) {
        fGLUniforms->width->set((GLfloat)width);
      }
      if (fGLUniforms->height.get() != nullptr) {
        fGLUniforms->height->set((GLfloat)height);
      }
      if (fGLUniforms->Cx.get() != nullptr) {
        fGLUniforms->Cx->set((GLfloat)lookAt.fX);
      }
      if (fGLUniforms->Cz.get() != nullptr) {
        fGLUniforms->Cz->set((GLfloat)lookAt.fZ);
      }
      if (fGLUniforms->grassBlockId.get() != nullptr) {
        fGLUniforms->grassBlockId->set((GLint)mcfile::blocks::minecraft::grass_block);
      }
      if (fGLUniforms->foliageBlockId) {
        fGLUniforms->foliageBlockId->set((GLint)mcfile::blocks::minecraft::oak_leaves);
      }
      if (fGLUniforms->netherrackBlockId) {
        fGLUniforms->netherrackBlockId->set((GLint)mcfile::blocks::minecraft::netherrack);
      }
      if (fGLUniforms->waterOpticalDensity) {
        fGLUniforms->waterOpticalDensity->set((GLfloat)fWaterOpticalDensity.get());
      }
      if (fGLUniforms->waterTranslucent) {
        fGLUniforms->waterTranslucent->set((GLboolean)fWaterTranslucent.get());
      }
      if (fGLUniforms->biomeBlend) {
        fGLUniforms->biomeBlend->set((GLint)fBiomeBlend.get());
      }
      if (fGLUniforms->enableBiome) {
        fGLUniforms->enableBiome->set((GLboolean)fEnableBiome.get());
      }
      if (fGLUniforms->dimension) {
        fGLUniforms->dimension->set((GLint)fDimension);
      }

      if (fGLUniforms->fade.get() != nullptr) {
        if (enableUI) {
          int const ms = (int)Clamp(now.toMilliseconds() - cache->fLoadTime.toMilliseconds(), 0LL, (int64)kFadeDurationMS);
          GLfloat const a = ms > kFadeDurationMS ? 1.0f : CubicEaseInOut((float)ms / (float)kFadeDurationMS, 0.0f, 1.0f, 1.0f);
          fGLUniforms->fade->set(a);
        } else {
          fGLUniforms->fade->set(1.0f);
        }
      }

      fGLContext.extensions.glActiveTexture(GL_TEXTURE0);
      cache->fTexture->bind();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      if (fGLUniforms->texture.get() != nullptr) {
        fGLUniforms->texture->set(0);
      }

      int const x = it.first.first;
      int const z = it.first.second;

      auto const &north = textures.find(MakeRegion(x, z - 1));
      if (north != textures.end()) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 1);
        north->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->north.get() != nullptr) {
          fGLUniforms->north->set(1);
        }
      }

      auto const &northEast = textures.find(MakeRegion(x + 1, z - 1));
      if (northEast != textures.end()) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 2);
        northEast->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->northEast.get() != nullptr) {
          fGLUniforms->northEast->set(2);
        }
      }

      auto const &east = textures.find(MakeRegion(x + 1, z));
      if (east != textures.end()) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 3);
        east->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->east.get() != nullptr) {
          fGLUniforms->east->set(3);
        }
      }

      auto const &southEast = textures.find(MakeRegion(x + 1, z + 1));
      if (southEast != textures.end()) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 4);
        southEast->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->southEast.get() != nullptr) {
          fGLUniforms->southEast->set(4);
        }
      }

      auto const &south = textures.find(MakeRegion(x, z + 1));
      if (south != textures.end()) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 5);
        south->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->south.get() != nullptr) {
          fGLUniforms->south->set(5);
        }
      }

      auto const &southWest = textures.find(MakeRegion(x - 1, z + 1));
      if (southWest != textures.end()) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 6);
        southWest->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->southWest.get() != nullptr) {
          fGLUniforms->southWest->set(6);
        }
      }

      auto const &west = textures.find(MakeRegion(x - 1, z));
      if (west != textures.end()) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 7);
        west->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->west.get() != nullptr) {
          fGLUniforms->west->set(7);
        }
      }

      auto const &northWest = textures.find(MakeRegion(x - 1, z - 1));
      if (northWest != textures.end()) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 8);
        northWest->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->northWest.get() != nullptr) {
          fGLUniforms->northWest->set(8);
        }
      }

      fGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, fGLBuffer->vBuffer);
      fGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fGLBuffer->iBuffer);

      fGLAttributes->enable(fGLContext);
      glDrawElements(GL_QUADS, Buffer::kNumPoints, GL_UNSIGNED_INT, nullptr);
      fGLAttributes->disable(fGLContext);
    }

    instantiateTextures(lookAt);
  }

  void handleAsyncUpdate() override {
    std::deque<AsyncUpdateQueue> queue;
    {
      std::lock_guard<std::mutex> lock(fMut);
      queue.swap(fAsyncUpdateQueue);
    }
    for (auto const &q : queue) {
      if (std::holds_alternative<AsyncUpdateQueueUpdateCaptureButtonStatus>(q)) {
        fCaptureButton->setEnabled(fLoadingFinished.get());
      } else if (std::holds_alternative<AsyncUpdateQueueReleaseGarbageThreadPool>(q)) {
        for (int i = (int)fPoolTomb.size() - 1; i >= 0; i--) {
          auto &pool = fPoolTomb[i];
          if (pool->getNumJobs() == 0) {
            pool.reset();
            fPoolTomb.erase(fPoolTomb.begin() + i);
          }
        }
      }
    }
  }

  void timerCallback() override {
    fGLContext.triggerRepaint();
  }

  void setWaterOpticalDensity(float v) {
    fWaterOpticalDensity = v;
    triggerRepaint();
  }

  void setWaterTranslucent(bool translucent) {
    fWaterTranslucent = translucent;
    triggerRepaint();
  }

  void setBiomeEnable(bool enable) {
    fEnableBiome = enable;
    triggerRepaint();
  }

  void setBiomeBlend(int blend) {
    fBiomeBlend = blend;
    triggerRepaint();
  }

  void setShowPin(bool show) {
    fShowPin = show;
    resetPinComponents();
    triggerRepaint();
  }

private:
  void updateShader() {
    using namespace juce;
    std::unique_ptr<juce::OpenGLShaderProgram> newShader(new juce::OpenGLShaderProgram(fGLContext));

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
        #extension GL_EXT_gpu_shader4 : enable
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
        uniform float width;
        uniform float height;
        uniform int dimension;

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

        float altitudeFromColor(vec4 color) {
            int a = int(color.a * 255.0);
            int r = int(color.r * 255.0);
            int h = (a << 1) + (0x1 & (r >> 7));
            return float(h) - 64.0;
        }
    
        BlockInfo pixelInfo(vec4 color) {
            // h:            9bit
            // waterDepth:   7bit
            // biome:        3bit
            // block:       10bit
            // biomeRadius:  3bit
            
            /*
             AAAAAAAARRRRRRRRGGGGGGGGBBBBBBBB
             hhhhhhhhhwwwwwwwbbboooooooooorrr : v3
             hhhhhhhhwwwwwwwbbbboooooooooorrr : v2
             */

            int a = int(color.a * 255.0);
            int r = int(color.r * 255.0);
            int g = int(color.g * 255.0);
            int b = int(color.b * 255.0);
             
            float h = altitudeFromColor(color);
            int depth = 0x7f & r;
            int biome = g >> 5;
            int block = ((0x1f & g) << 5) + ((0xf8 & b) >> 3);
            int biomeRadius = 0x7 & b;
            BlockInfo info;
            info.height = h;
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

        float rand(float n) {
            return fract(sin(n) * 43758.5453123);
        }

        float noise(vec2 co){
            return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
        }

        vec4 voidColor() {
            float s1 = (rand(gl_FragCoord.x) + rand(textureCoordOut.x)) * 0.5 * width;
            float s2 = (rand(gl_FragCoord.y) + rand(textureCoordOut.y)) * 0.5 * height;
            float x = noise(vec2(s1, s2));
            float y = noise(vec2(s2, s1));
            float bm1 = sqrt(-2 * log(x)) * cos(2 * 3.1415926 * y);
            float bm2 = sqrt(-2 * log(y)) * sin(2 * 3.1415926 * x);

            // H: average=0.733309; sigma=0.00610866; minmax=[0.666667, 0.766667]
            // S: average=0.448486; sigma=0.0703466; minmax=[0.0810811, 0.6]
            // B: average=0.0958848; sigma=0.0170297; minmax=[0.0117647, 0.145098]

            float h = 0.733309;

            float sAvg = 0.448486;
            float sSigma = 0.0703466;
            float sMin = 0.0810811;
            float sMax = 0.6;
            float s = clamp(sAvg + bm1 * sSigma, sMin, sMax);

            float vAvg = 0.0958848;
            float vSigma = 0.0170297;
            float vMin = 0.0117647;
            float vMax = 0.145098;

            float v = clamp(vAvg + bm2 * vSigma, vMin, vMax);
            vec3 c = hsv2rgb(vec3(h, s, v));
            return vec4(c.rgb, 1);
        }
    )#";

    fragment << "vec4 colorFromBlockId(int blockId) {" << std::endl;
    fragment << "    if (blockId == #{airBlockId}) {" << std::endl;
    fragment << "        return vec4(0.0, 0.0, 0.0, 0.0);" << std::endl;
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
        bool isVoid = false;

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
            if (dimension == 1) {
                c = voidColor();
                isVoid = true;
            } else {
                c = vec4(0.0, 0.0, 0.0, 0.0);
            }
        } else if (blockId == #{airBlockId} && dimension == 1) {
            c = voidColor();
            isVoid = true;
        } else {
            vec4 cc = colorFromBlockId(blockId);
            if (cc.a == 0.0) {
                c = cc;
            } else {
                c = vec4(cc.r, cc.g, cc.b, alpha);
            }
        }

        if (!isVoid && (waterDepth == 0.0 || (waterDepth > 0.0 && waterTranslucent))) {
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
            float northH = altitudeFromColor(northC);
            float westH = altitudeFromColor(westC);
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

    String fragmentShaderTemplate = fragment.str();
    String fragmentShader = fragmentShaderTemplate.replace("#{airBlockId}", String(mcfile::blocks::minecraft::air));
#if 0
    int lineNumber = 0;
    for (auto const& line : mcfile::String::Split(fragmentShader.toStdString(), '\n')) {
        std::cout << "#" << (lineNumber++) << line << std::endl;
    }
#endif
    newShader->addFragmentShader(fragmentShader);

    newShader->link();
    newShader->use();

    fGLUniforms.reset(new Uniforms(fGLContext, *newShader));
    fGLAttributes.reset(new Attributes(fGLContext, *newShader));

    fGLShader.reset(newShader.release());
  }

  juce::Point<float> getMapCoordinateFromView(juce::Point<float> p) const {
    LookAt const current = clampedLookAt();
    return getMapCoordinateFromView(p, current);
  }

  juce::Point<float> getViewCoordinateFromMap(juce::Point<float> p) const {
    LookAt const current = clampedLookAt();
    return getViewCoordinateFromMap(p, current);
  }

  juce::Point<float> getMapCoordinateFromView(juce::Point<float> p, LookAt lookAt) const {
    juce::Point<int> size = fSize.load();
    float const width = size.x;
    float const height = size.y;
    float const bx = lookAt.fX + (p.x - width / 2) * lookAt.fBlocksPerPixel;
    float const bz = lookAt.fZ + (p.y - height / 2) * lookAt.fBlocksPerPixel;
    return juce::Point<float>(bx, bz);
  }

  juce::Point<float> getViewCoordinateFromMap(juce::Point<float> p, LookAt lookAt) const {
    juce::Point<int> size = fSize.load();
    float const width = size.x;
    float const height = size.y;
    float const x = (p.x - lookAt.fX) / lookAt.fBlocksPerPixel + width / 2;
    float const y = (p.y - lookAt.fZ) / lookAt.fBlocksPerPixel + height / 2;
    return juce::Point<float>(x, y);
  }

  void magnify(juce::Point<float> p, float rate) {
    LookAt const current = clampedLookAt();
    LookAt next = current;

    next.fBlocksPerPixel = (std::min)((std::max)(current.fBlocksPerPixel / rate, kMinScale), kMaxScale);

    float const width = getWidth();
    float const height = getHeight();
    juce::Point<float> const pivot = getMapCoordinateFromView(p);
    float const dx = (p.x - width / 2);
    float const dz = (p.y - height / 2);
    next.fX = pivot.x - dx * next.fBlocksPerPixel;
    next.fZ = pivot.y - dz * next.fBlocksPerPixel;

    setLookAt(next);

    triggerRepaint();
  }

  void drawBackground() {
    using namespace juce;
    Point<int> size = fSize.load();
    const int width = size.x;
    const int height = size.y;
    const float desktopScale = (float)fGLContext.getRenderingScale();

    std::unique_ptr<LowLevelGraphicsContext> glRenderer(createOpenGLGraphicsContext(fGLContext,
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

    std::set<Region> loadingRegions;
    {
      std::lock_guard<std::mutex> lock(fMut);
      loadingRegions = fLoadingRegions;
    }
    g.setColour(Colour::fromRGBA(0, 0, 0, 37));
    for (Region region : loadingRegions) {
      int const x = region.first * 512;
      int const z = region.second * 512;
      Point<float> topLeft = getViewCoordinateFromMap(Point<float>(x, z), current);
      float const regionSize = 512.0f / current.fBlocksPerPixel;
      g.fillRect(topLeft.x, topLeft.y, regionSize, regionSize);
    }
  }

  void triggerRepaint() {
    repaint();
    fGLContext.triggerRepaint();
  }

  void captureToImage() {
    using namespace juce;
    fCaptureButton->setEnabled(false);

    fFileChooser.reset(new FileChooser(TRANS("Choose file name"), File(), "*.png", true));
    fFileChooser->launchAsync(FileBrowserComponent::FileChooserFlags::saveMode, [this](FileChooser const &chooser) {
      defer {
        fCaptureButton->setEnabled(true);
      };
      File file = chooser.getResult();
      if (file == File()) {
        return;
      }
      SavePNGProgressWindow wnd(this, fGLContext, file);
      wnd.run();
    });
  }

  LookAt clampLookAt(LookAt l) const {
    juce::Rectangle<int> visibleRegions = fVisibleRegions.load();

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

  LookAt clampedLookAt() const {
    return clampLookAt(fLookAt.load());
  }

  void setLookAt(LookAt next) {
    fLookAt = clampLookAt(next);
    updateAllPinComponentPosition();
  }

  void queueTextureLoadingImpl(juce::OpenGLContext &ctx, std::vector<juce::File> files, juce::File worldDirectory, Dimension dim, bool useCache) {
    using namespace juce;
    std::lock_guard<std::mutex> lock(fMut);
    fLoadingFinished = false;

    Rectangle<int> visibleRegions = fVisibleRegions.load();
    int minX = visibleRegions.getX();
    int maxX = visibleRegions.getRight();
    int minY = visibleRegions.getY();
    int maxY = visibleRegions.getBottom();

    for (File const &f : files) {
      auto r = mcfile::je::Region::MakeRegion(PathFromFile(f));
      auto region = MakeRegion(r->fX, r->fZ);
      auto *job = new TexturePackJob(worldDirectory, f, region, dim, useCache, this);
      fPool->addJob(job, true);
      fLoadingRegions.insert(region);
      minX = std::min(minX, r->fX);
      maxX = std::max(maxX, r->fX + 1);
      minY = std::min(minY, r->fZ);
      maxY = std::max(maxY, r->fZ + 1);
    }

    fVisibleRegions = Rectangle<int>(minX, minY, maxX - minX, maxY - minY);
    if (!isTimerRunning()) {
      startTimer(16); // 60fps
    }
  }

  void instantiateTextures(LookAt lookAt) {
    bool loadingFinished = false;
    juce::File worldDirectory;
    Dimension dimension;
    std::vector<std::shared_ptr<TexturePackJob::Result>> remove;
    std::vector<std::pair<std::shared_ptr<TexturePackJob::Result>, float>> distances;
    {
      std::lock_guard<std::mutex> lock(fMut);
      worldDirectory = fWorldDirectory;
      dimension = fDimension;

      for (size_t i = 0; i < fGLJobResults.size(); i++) {
        auto const &result = fGLJobResults[i];
        if (result->fWorldDirectory != worldDirectory) {
          remove.push_back(result);
          continue;
        }
        if (result->fDimension != dimension) {
          remove.push_back(result);
          continue;
        }
        float distance = DistanceSqBetweenRegionAndLookAt(lookAt, result->fRegion);
        distances.push_back(std::make_pair(result, distance));
      }
    }
    std::sort(distances.begin(), distances.end(), [](auto const &a, auto const &b) {
      return a.second < b.second;
    });

    int constexpr kNumLoadTexturesPerFrame = 16;
    for (int i = 0; i < kNumLoadTexturesPerFrame && i < distances.size(); i++) {
      std::shared_ptr<TexturePackJob::Result> j = distances[i].first;
      remove.push_back(j);

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

      std::lock_guard<std::mutex> lock(fMut);
      auto it = fLoadingRegions.find(j->fRegion);
      if (it != fLoadingRegions.end()) {
        fLoadingRegions.erase(it);
      }
      if (fLoadingRegions.empty()) {
        fLoadingFinished = true;
        loadingFinished = true;
      }
    }
    {
      std::lock_guard<std::mutex> lock(fMut);
      for (auto const &it : remove) {
        for (size_t i = 0; i < fGLJobResults.size(); i++) {
          if (it.get() == fGLJobResults[i].get()) {
            fGLJobResults.erase(fGLJobResults.begin() + i);
            break;
          }
        }
      }
    }

    if (loadingFinished) {
      callAfterDelay(kFadeDurationMS, [this]() {
        stopTimer();
      });
      {
        std::lock_guard<std::mutex> lock(fMut);
        fAsyncUpdateQueue.push_back(AsyncUpdateQueueUpdateCaptureButtonStatus{});
      }
      triggerAsyncUpdate();
    }
  }

  void mouseRightClicked(juce::MouseEvent const &e) {
    using namespace juce;
    if (!fWorldDirectory.exists()) {
      return;
    }
    if (!fShowPin) {
      return;
    }
    LookAt current = clampedLookAt();
    Dimension dim = fDimension;

    PopupMenu menu;
    menu.addItem(1, TRANS("Put a pin here"));
    Point<int> pos = e.getScreenPosition();
    menu.showMenuAsync(PopupMenu::Options().withTargetScreenArea(Rectangle<int>(pos, pos)), [this, e, dim, current](int menuId) {
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
    });
  }

  void saveWorldData() {
    juce::File f = WorldData::WorldDataPath(fWorldDirectory);
    fWorldData.save(f);
  }

  void addPinComponent(std::shared_ptr<Pin> pin) {
    using namespace juce;
    PinComponent *pinComponent = new PinComponent(pin);
    pinComponent->updatePinPosition(getViewCoordinateFromMap(pinComponent->getMapCoordinate()));
    pinComponent->onRightClick = [this](std::shared_ptr<Pin> pin, Point<int> screenPos) {
      handlePinRightClicked(pin, screenPos);
    };
    pinComponent->onDoubleClick = [this](std::shared_ptr<Pin> pin, Point<int> screenPos) {
      handlePinDoubleClicked(pin, screenPos);
    };
    pinComponent->onDrag = [this](std::shared_ptr<Pin> pin, Point<int> screenPos) {
      handlePinDrag(pin, screenPos);
    };
    pinComponent->onDragEnd = [this](std::shared_ptr<Pin> pin) {
      saveWorldData();
    };
    addAndMakeVisible(pinComponent);
    fPinComponents.emplace_back(pinComponent);
  }

  void updateAllPinComponentPosition() {
    using namespace juce;
    LookAt const lookAt = clampedLookAt();
    for (auto const &pin : fPinComponents) {
      Point<float> pos = pin->getMapCoordinate();
      if (fDimension == Dimension::TheNether) {
        if (pin->getDimension() != Dimension::TheNether) {
          pos = Point<float>(pos.x / 8, pos.y / 8);
          pin->updatePinPosition(getViewCoordinateFromMap(pos, lookAt));
        } else {
          pin->updatePinPosition(getViewCoordinateFromMap(pos, lookAt));
        }
      } else if (fDimension == Dimension::Overworld) {
        if (pin->getDimension() != Dimension::Overworld) {
          pos = Point<float>(pos.x * 8, pos.y * 8);
          pin->updatePinPosition(getViewCoordinateFromMap(pos, lookAt));
        } else {
          pin->updatePinPosition(getViewCoordinateFromMap(pos, lookAt));
        }
      } else {
        pin->updatePinPosition(getViewCoordinateFromMap(pos, lookAt));
      }
    }
  }

  void handlePinRightClicked(std::shared_ptr<Pin> const &pin, juce::Point<int> screenPos) {
    using namespace juce;
    PopupMenu menu;
    menu.addItem(1, TRANS("Delete") + " \"" + pin->fMessage + "\"");
    menu.addItem(2, TRANS("Rename") + " \"" + pin->fMessage + "\"");
    menu.showMenuAsync(PopupMenu::Options().withTargetScreenArea(Rectangle<int>(screenPos, screenPos)), [this, pin](int menuId) {
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
                               }),
                               fWorldData.fPins.end());
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
    });
  }

  void handlePinDoubleClicked(std::shared_ptr<Pin> const &pin, juce::Point<int> screenPos) {
    using namespace juce;
    auto result = TextInputDialog::show(this, TRANS("Please enter a pin name"), pin->fMessage);
    if (result.first != 1) {
      return;
    }
    String message = result.second;
    pin->fMessage = message;
    saveWorldData();
    triggerRepaint();
  }

  void handlePinDrag(std::shared_ptr<Pin> const &pin, juce::Point<int> screenPos) {
    using namespace juce;
    Point<int> p = getScreenPosition();
    LookAt lookAt = fLookAt.load();
    Point<float> viewPos(screenPos.x - p.x, screenPos.y - p.y);
    Point<float> mapPos = getMapCoordinateFromView(viewPos, lookAt);
    pin->fX = floor(mapPos.x);
    pin->fZ = floor(mapPos.y) + 1;
    for (auto const &comp : fPinComponents) {
      if (!comp->isPresenting(pin)) {
        continue;
      }
      comp->updatePinPosition(getViewCoordinateFromMap(comp->getMapCoordinate(), lookAt));
      break;
    }
  }

  void resetPinComponents() {
    fPinComponents.clear();
    if (!fShowPin) {
      return;
    }
    for (auto const &p : fWorldData.fPins) {
      if (fDimension == Dimension::TheEnd) {
        if (p->fDim != fDimension) {
          continue;
        }
      } else {
        if (p->fDim == Dimension::TheEnd) {
          continue;
        }
      }
      addPinComponent(p);
    }
  }

  static juce::ThreadPool *CreateThreadPool() {
    auto const threads = (std::max)(1, (int)std::thread::hardware_concurrency() - 1);
    return new juce::ThreadPool(threads);
  }

  static float DistanceSqBetweenRegionAndLookAt(LookAt lookAt, Region region) {
    float const regionCenterX = region.first * 512 - 256;
    float const regionCenterZ = region.second * 512 - 256;
    float const dx = regionCenterX - lookAt.fX;
    float const dz = regionCenterZ - lookAt.fZ;
    return dx * dx + dz * dz;
  }

  template <typename T>
  static T Clamp(T v, T min, T max) {
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
    t /= duration / 2.0f;
    if (t < 1.0f) {
      return c / 2.0f * t * t * t + b;
    } else {
      t = t - 2.0f;
      return c / 2.0f * (t * t * t + 2.0f) + b;
    }
  }

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
    RegionUpdateChecker(MapViewComponent *comp)
        : Thread("RegionUpdateChecker"), fDim(Dimension::Overworld), fMapView(comp) {
    }

    void run() override {
      std::map<std::string, int64_t> updated;

      while (!currentThreadShouldExit()) {
        try {
          Thread::sleep(1000);
          checkUpdatedFiles(updated);
        } catch (...) {
        }
      }
    }

    void setDirectory(juce::File f, Dimension dim) {
      juce::ScopedLock lk(fSection);
      fDirectory = f;
      fDim = dim;
    }

  private:
    void checkUpdatedFiles(std::map<std::string, int64_t> &updated) {
      using namespace juce;
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
                                          }),
                                          updated.end());

      RangedDirectoryIterator it(DimensionDirectory(d, dim), false, "*.mca");
      std::vector<File> files;
      for (DirectoryEntry entry : it) {
        File f = entry.getFile();
        auto r = mcfile::je::Region::MakeRegion(PathFromFile(f));
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

  juce::Point<float> fCenterWhenDragStart;

  juce::Point<float> fMouse;

  std::unique_ptr<juce::ThreadPool> fPool;
  std::deque<std::unique_ptr<juce::ThreadPool>> fPoolTomb;
  std::deque<std::shared_ptr<TexturePackJob::Result>> fGLJobResults;

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
