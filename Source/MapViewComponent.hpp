#pragma once

namespace mcview {

class MapViewComponent
    : public juce::Component,
      private juce::OpenGLRenderer,
      private juce::AsyncUpdater,
      private juce::Timer,
      public juce::ChangeListener,
      public TexturePackThreadPool::Delegate,
      public SavePNGProgressWindow ::Delegate,
      public RegionUpdateChecker::Delegate,
      public TextInputDialog<PinEdit>::Delegate {
  struct AsyncUpdateQueueReleaseGarbageThreadPool {};

  using AsyncUpdateQueue = std::variant<AsyncUpdateQueueReleaseGarbageThreadPool>;

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
      : fLookAt({0, 0, 5}),
        fWaterOpticalDensity(Settings::kDefaultWaterOpticalDensity),
        fWaterTranslucent(true),
        fEnableBiome(true),
        fBiomeBlend(2),
        fPaletteType(PaletteType::mcview),
        fLightingType(LightingType::topLeft) {
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
      setWorldDirectory(fWorldDirectory, Dimension::Overworld, fEdition);
    };
    fOverworld->setEnabled(false);
    fOverworld->setTooltip(TRANS("Overworld"));
    addAndMakeVisible(*fOverworld);

    fNetherImage = Drawable::createFromImageData(BinaryData::baseline_whatshot_white_18dp_png, BinaryData::baseline_whatshot_white_18dp_pngSize);
    fNether.reset(new DrawableButton("", DrawableButton::ButtonStyle::ImageOnButtonBackground));
    fNether->setImages(fNetherImage.get());
    fNether->onClick = [this]() {
      setWorldDirectory(fWorldDirectory, Dimension::TheNether, fEdition);
    };
    fNether->setEnabled(false);
    fNether->setTooltip(TRANS("The Nether"));
    addAndMakeVisible(*fNether);

    fEndImage = Drawable::createFromImageData(BinaryData::baseline_brightness_3_white_18dp_png, BinaryData::baseline_brightness_3_white_18dp_pngSize);
    fEnd.reset(new DrawableButton("", DrawableButton::ButtonStyle::ImageOnButtonBackground));
    fEnd->setImages(fEndImage.get());
    fEnd->onClick = [this]() {
      setWorldDirectory(fWorldDirectory, Dimension::TheEnd, fEdition);
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

  ~MapViewComponent() override {
    juce::Desktop::getInstance().getAnimator().removeChangeListener(this);

    fGLContext.detach();
    if (fPool) {
      fPool->abandon(true);
    }
    for (auto &pool : fPoolTrashBin) {
      pool->abandon(true);
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
            g.drawFittedText(String::formatted("r.%d.%d.mca", x, z),
                             tl.x, tl.y,
                             br.x - tl.x, br.y - tl.y,
                             Justification::centred, 1);
          }
        }
      }
    }

    juce::Rectangle<float> const border(width - kMargin - kButtonSize - kMargin - coordLabelWidth, kMargin, coordLabelWidth, coordLabelHeight);
    g.setColour(Colour::fromFloatRGBA(1, 1, 1, 0.8));
    g.fillRoundedRectangle(border, 6.0f);
    g.setColour(Colours::lightgrey);
    g.drawRoundedRectangle(border, 6.0f, 1.0f);
    Point<float> block = getMapCoordinateFromView(fMouse);
    int y = kMargin;
    g.setColour(Colours::black);
    Font bold(14, Font::bold);
    Font regular(14);
    juce::Rectangle<int> line1(border.getX() + kMargin, y, border.getWidth() - 2 * kMargin, lineHeight);
    g.setFont(regular);
    g.drawText("X: ", line1, Justification::centredLeft);
    g.setFont(bold);
    g.drawFittedText(String::formatted("%d", (int)floor(block.x)), line1, Justification::centredRight, 1);
    y += lineHeight;
    juce::Rectangle<int> line2(border.getX() + kMargin, y, border.getWidth() - 2 * kMargin, lineHeight);
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

    std::unique_ptr<GLBuffer> buffer(new GLBuffer());

    fGLContext.extensions.glGenBuffers(1, &buffer->vBuffer);
    fGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, buffer->vBuffer);
    std::vector<GLVertex> vertices = {
        {{0, 0}, {0.0, 0.0}},
        {{1, 0}, {1.0, 0.0}},
        {{1, 1}, {1.0, 1.0}},
        {{0, 1}, {0.0, 1.0}},
    };
    fGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, sizeof(GLVertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

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
    fGLPalette.reset();
    fGLPaletteJava.reset();
    fGLPaletteBedrock.reset();
    fGLContext.extensions.glDeleteBuffers(1, &fGLBuffer->vBuffer);
    fGLContext.extensions.glDeleteBuffers(1, &fGLBuffer->iBuffer);
  }

  void savePNGProgressWindowRender(int const width, int const height, LookAt const lookAt) override {
    return render(width, height, lookAt, false);
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

    VisibleRegions visible = fVisibleRegions.load();
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

  void texturePackThreadPoolDidFinishJob(TexturePackThreadPool *pool, std::shared_ptr<TexturePackJob::Result> result) override {
    {
      std::lock_guard<std::mutex> lock(fMut);
      if (pool != fPool.get()) {
        return;
      }
      fGLJobResults.push_back(result);
      fAsyncUpdateQueue.push_back(AsyncUpdateQueueReleaseGarbageThreadPool{});
    }
    triggerAsyncUpdate();
  }

  void regionUpdateCheckerDidDetectRegionFileUpdate(std::vector<juce::File> files, Dimension dimension) override {
    enqueueTextureLoadingJava(files, dimension, false);
  }

  void setWorldDirectory(juce::File directory, Dimension dim, Edition edition) {
    using namespace juce;
    if (fWorldDirectory.getFullPathName() == directory.getFullPathName() && fDimension == dim) {
      return;
    }

    fRegionUpdateChecker->setDirectory(directory, dim);

    fOverworld->setEnabled(dim != Dimension::Overworld);
    fNether->setEnabled(dim != Dimension::TheNether);
    fEnd->setEnabled(dim != Dimension::TheEnd);

    fCaptureButton->setEnabled(false);

    File worldDataFile = WorldData::WorldDataPath(directory);
    WorldData data = WorldData::Load(worldDataFile);

    std::shared_ptr<leveldb::DB> db;
    if (fPool) {
      fPool->abandon(false);
      if (auto pool = dynamic_cast<BedrockTexturePackThreadPool *>(fPool.get()); pool && pool->fWorldDirectory == directory && edition == Edition::Bedrock) {
        db = pool->fDb;
      }
      fPoolTrashBin.push_back(std::move(fPool));
    }
    if (edition == Edition::Bedrock) {
      if (!db) {
        leveldb::DB *ptr = nullptr;
        if (auto st = leveldb::DB::Open({}, PathFromFile(directory) / "db", &ptr); st.ok()) {
          db.reset(ptr);
        }
      }
      fPool.reset(new BedrockTexturePackThreadPool(directory, dim, db, this));
    } else {
      fPool.reset(new JavaTexturePackThreadPool(directory, dim, this));
    }

    auto garbageTextures = std::make_shared<std::map<Region, std::unique_ptr<RegionTextureCache>>>();
    garbageTextures->swap(fTextures);

    {
      std::lock_guard<std::mutex> lk(fMut);

      LookAt const lookAt = fLookAt.load();

      fLoadingRegions.clear();
      fWorldDirectory = directory;
      fDimension = dim;
      fWorldData = data;
      fEdition = edition;
      for (auto &it : *garbageTextures) {
        fTextureTrashBin.push_back(std::move(it.second));
      }
      fVisibleRegions = VisibleRegions();
      resetPinComponents();

      VisibleRegions vr;

      if (edition == Edition::Bedrock) {
        if (db) {
          LookAt nextLookAt = lookAt;
          nextLookAt.fX = 0;
          nextLookAt.fZ = 0;
          int minRx, minRz, maxRx, maxRz;
          viewportRegions(nextLookAt, &minRx, &minRz, &maxRx, &maxRz);
          VisibleRegions vr;

          for (int rz = minRz; rz <= maxRz; rz++) {
            for (int rx = minRx; rx <= maxRx; rx++) {
              bool exists = false;
              for (int cz = rz * 32; cz < rz * 32 + 32; cz++) {
                for (int cx = rx * 32; cx < rx * 32 + 32; cx++) {
                  if (mcfile::be::Chunk::Exists(db.get(), cx, cz, DimensionFromDimension(dim))) {
                    exists = true;
                    break;
                  }
                }
                if (exists) {
                  break;
                }
              }
              if (!exists) {
                continue;
              }
              auto region = MakeRegion(rx, rz);
              vr.add(rx, rz);
              fTextures[region] = std::make_unique<RegionTextureCache>(directory, dim, region);
              fPool->addTexturePackJob(region, true);
              fLoadingRegions.insert(region);
            }
          }
          fVisibleRegions = vr;
          setLookAt(clampLookAt(nextLookAt));
          startLoadingTimer();

          mcfile::be::Chunk::ForAll(db.get(), DimensionFromDimension(dim), [this, directory, dim, &vr](int cx, int cz) {
            int rx = mcfile::Coordinate::RegionFromChunk(cx);
            int rz = mcfile::Coordinate::RegionFromChunk(cz);
            auto region = MakeRegion(rx, rz);
            if (fTextures.count(region) > 0) {
              return;
            }
            vr.add(rx, rz);
            fTextures[region] = std::make_unique<RegionTextureCache>(directory, dim, region);
          });
          fVisibleRegions = vr;
          setLookAt(clampedLookAt());
        } else {
          // TODO: error dialog
        }
      } else {
        LookAt nextLookAt = lookAt;
        nextLookAt.fX = 0;
        nextLookAt.fZ = 0;
        int minRx, minRz, maxRx, maxRz;
        viewportRegions(nextLookAt, &minRx, &minRz, &maxRx, &maxRz);

        File dir = DimensionDirectory(fWorldDirectory, fDimension);
        std::vector<File> files;
        for (int rz = minRz; rz <= maxRz; rz++) {
          for (int rx = minRx; rx <= maxRx; rx++) {
            auto mca = dir.getChildFile(mcfile::je::Region::GetDefaultRegionFileName(rx, rz));
            if (!mca.existsAsFile()) {
              continue;
            }
            auto r = mcfile::je::Region::MakeRegion(PathFromFile(mca));
            if (!r) {
              continue;
            }
            files.push_back(mca);
          }
        }
        unsafeEnqueueTextureLoadingJava(files, dim, true);
        setLookAt(clampLookAt(nextLookAt));

        VisibleRegions vr = fVisibleRegions;
        RangedDirectoryIterator it(DimensionDirectory(fWorldDirectory, fDimension), false, "*.mca");
        for (DirectoryEntry entry : it) {
          File f = entry.getFile();
          auto r = mcfile::je::Region::MakeRegion(PathFromFile(f));
          if (!r) {
            continue;
          }
          vr.add(r->fX, r->fZ);
          auto region = MakeRegion(r->fX, r->fZ);
          if (auto found = fTextures.find(region); found == fTextures.end()) {
            fTextures[region] = std::make_unique<RegionTextureCache>(fWorldDirectory, fDimension, region);
          }
        }
        fVisibleRegions = vr;
        setLookAt(clampedLookAt());
      }
    }
  }

  void enqueueTextureLoadingJava(std::vector<juce::File> files, Dimension dim, bool useCache) {
    std::lock_guard<std::mutex> lock(fMut);
    unsafeEnqueueTextureLoadingJava(files, dim, useCache);
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
    PaletteType palette = fPaletteType.get();
    LightingType lighting = fLightingType.get();

    if (!fGLShader) {
      updateShader();
    }
    OpenGLTexture *paletteTexture = nullptr;
    if (palette == PaletteType::java) {
      if (!fGLPaletteJava) {
        LoadPalette(fGLPaletteJava, Palette::JavaColorFromId);
      }
      paletteTexture = fGLPaletteJava.get();
    } else if (palette == PaletteType::bedrock) {
      if (!fGLPaletteBedrock) {
        LoadPalette(fGLPaletteBedrock, Palette::BedrockColorFromId);
      }
      paletteTexture = fGLPaletteBedrock.get();
    } else {
      if (!fGLPalette) {
        LoadPalette(fGLPalette, Palette::ColorFromId);
      }
      paletteTexture = fGLPalette.get();
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    fGLShader->use();
    auto const &textures = fTextures;

    for (auto &it : textures) {
      auto &cache = it.second;
      if (!cache->fTexture) {
        continue;
      }
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
      if (fGLUniforms->waterBlockId) {
        fGLUniforms->waterBlockId->set((GLint)mcfile::blocks::minecraft::water);
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

      if (auto const &north = textures.find(MakeRegion(x, z - 1)); north != textures.end() && north->second->fTexture) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 1);
        north->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->north) {
          fGLUniforms->north->set(1);
        }
      }

      if (auto const &northEast = textures.find(MakeRegion(x + 1, z - 1)); northEast != textures.end() && northEast->second->fTexture) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 2);
        northEast->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->northEast) {
          fGLUniforms->northEast->set(2);
        }
      }

      if (auto const &east = textures.find(MakeRegion(x + 1, z)); east != textures.end() && east->second->fTexture) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 3);
        east->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->east) {
          fGLUniforms->east->set(3);
        }
      }

      if (auto const &southEast = textures.find(MakeRegion(x + 1, z + 1)); southEast != textures.end() && southEast->second->fTexture) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 4);
        southEast->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->southEast) {
          fGLUniforms->southEast->set(4);
        }
      }

      if (auto const &south = textures.find(MakeRegion(x, z + 1)); south != textures.end() && south->second->fTexture) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 5);
        south->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->south) {
          fGLUniforms->south->set(5);
        }
      }

      if (auto const &southWest = textures.find(MakeRegion(x - 1, z + 1)); southWest != textures.end() && southWest->second->fTexture) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 6);
        southWest->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->southWest) {
          fGLUniforms->southWest->set(6);
        }
      }

      if (auto const &west = textures.find(MakeRegion(x - 1, z)); west != textures.end() && west->second->fTexture) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 7);
        west->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->west) {
          fGLUniforms->west->set(7);
        }
      }

      if (auto const &northWest = textures.find(MakeRegion(x - 1, z - 1)); northWest != textures.end() && northWest->second->fTexture) {
        fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 8);
        northWest->second->fTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        if (fGLUniforms->northWest) {
          fGLUniforms->northWest->set(8);
        }
      }

      fGLContext.extensions.glActiveTexture(GL_TEXTURE0 + 9);
      paletteTexture->bind();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      if (fGLUniforms->palette) {
        fGLUniforms->palette->set(9);
      }
      if (fGLUniforms->paletteSize) {
        fGLUniforms->paletteSize->set((GLint)paletteTexture->getWidth());
      }
      if (fGLUniforms->lightingType) {
        fGLUniforms->lightingType->set(static_cast<GLint>(lighting));
      }
      if (fGLUniforms->paletteType) {
        GLint pt = 0;
        switch (palette) {
        case PaletteType::java:
          pt = 1;
          break;
        case PaletteType::bedrock:
          pt = 2;
          break;
        case PaletteType::mcview:
        default:
          pt = 0;
          break;
        }
        fGLUniforms->paletteType->set(pt);
      }

      fGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, fGLBuffer->vBuffer);
      fGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fGLBuffer->iBuffer);

      fGLAttributes->enable(fGLContext);
      glDrawElements(GL_QUADS, GLBuffer::kNumPoints, GL_UNSIGNED_INT, nullptr);
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
      if (std::holds_alternative<AsyncUpdateQueueReleaseGarbageThreadPool>(q)) {
        for (int i = (int)fPoolTrashBin.size() - 1; i >= 0; i--) {
          if (fPoolTrashBin[i]->getNumJobs() == 0) {
            fPoolTrashBin.erase(fPoolTrashBin.begin() + i);
          }
        }
      }
    }
  }

  void timerCallback() override {
    fGLContext.triggerRepaint();
  }

  void setWaterOpticalDensity(float v) {
    if (v == fWaterOpticalDensity.get()) {
      return;
    }
    fWaterOpticalDensity = v;
    triggerRepaint();
  }

  void setWaterTranslucent(bool translucent) {
    if (translucent == fWaterTranslucent.get()) {
      return;
    }
    fWaterTranslucent = translucent;
    triggerRepaint();
  }

  void setBiomeEnable(bool enable) {
    if (enable == fEnableBiome.get()) {
      return;
    }
    fEnableBiome = enable;
    triggerRepaint();
  }

  void setBiomeBlend(int blend) {
    if (blend == fBiomeBlend.get()) {
      return;
    }
    fBiomeBlend = blend;
    triggerRepaint();
  }

  void setShowPin(bool show) {
    if (show == fShowPin) {
      return;
    }
    fShowPin = show;
    resetPinComponents();
    triggerRepaint();
  }

  void setPaletteType(PaletteType palette) {
    if (palette == fPaletteType.get()) {
      return;
    }
    fPaletteType = palette;
    triggerRepaint();
  }

  void setLightingType(LightingType type) {
    if (type == fLightingType.get()) {
      return;
    }
    fLightingType = type;
    triggerRepaint();
  }

private:
  static void LoadPalette(std::unique_ptr<juce::OpenGLTexture> &texture, std::function<std::optional<juce::Colour>(mcfile::blocks::BlockId)> converter) {
    using namespace juce;
    int count = (int)mcfile::blocks::minecraft::minecraft_max_block_id - 1;
    int k = (int)ceil(log2(count) / 2);
    int size = 2 << k;
    std::unique_ptr<PixelARGB[]> data(new PixelARGB[size * size]);
    std::fill_n(data.get(), size * size, PixelARGB(0, 0, 0, 0));
    for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
      if (auto color = converter(id); color) {
        data[id - 1] = color->getPixelARGB();
      }
    }
    texture.reset(new juce::OpenGLTexture);
    texture->loadARGB(data.get(), size, size);
  }

  void updateShader() {
    using namespace std;
    using namespace juce;
    std::unique_ptr<juce::OpenGLShaderProgram> newShader(new juce::OpenGLShaderProgram(fGLContext));

    if (!newShader->addVertexShader(String::fromUTF8(BinaryData::tile_vert, BinaryData::tile_vertSize))) {
      juce::Logger::outputDebugString("addVertexShader failed");
      juce::Logger::outputDebugString(newShader->getLastError());
    }

    colormap::kbinani::Altitude altitude;

    std::ostringstream fragment;
    fragment << String::fromUTF8(BinaryData::color_frag, BinaryData::color_fragSize);
    fragment << altitude.getSource() << std::endl;

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
    {
      Colour c(Palette::kDefaultOceanColor);
      fragment << "        return rgb(" << (int)c.getRed() << ", " << (int)c.getGreen() << ", " << (int)c.getBlue() << ", 255);" << std::endl;
    }
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

    String fragmentShaderTemplate = fragment.str();
    String fragmentShader = fragmentShaderTemplate.replace("#{airBlockId}", String(mcfile::blocks::minecraft::air));

#if 0
    int lineNumber = 0;
    for (auto const &line : mcfile::String::Split(fragmentShader.toStdString(), '\n')) {
      juce::Logger::outputDebugString("#" + String(lineNumber++) + line);
    }
#endif
    if (!newShader->addFragmentShader(fragmentShader)) {
      juce::Logger::outputDebugString("addFragmentShader failed");
      juce::Logger::outputDebugString(newShader->getLastError());
    }

    if (!newShader->link()) {
      juce::Logger::outputDebugString("link failed");
      juce::Logger::outputDebugString(newShader->getLastError());
    }
    newShader->use();

    fGLUniforms.reset(new GLUniforms(fGLContext, *newShader));
    fGLAttributes.reset(new GLAttributes(fGLContext, *newShader));

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

  static juce::Point<float> GetMapCoordinateFromView(juce::Point<float> p, juce::Point<int> size, LookAt lookAt) {
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
      juce::Rectangle<int> bounds(minX, minZ, maxX - minX, maxZ - minZ);

      SavePNGProgressWindow wnd(this, fGLContext, file, bounds);
      wnd.run();
    });
  }

  LookAt clampLookAt(LookAt l) const {
    VisibleRegions visibleRegions = fVisibleRegions.load();

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
    auto c = clampLookAt(next);
    fLookAt = c;
    if (fPool) {
      fPool->fLookAt = c;
    }
    updateAllPinComponentPosition();
  }

  void unsafeEnqueueTextureLoadingJava(std::vector<juce::File> files, Dimension dim, bool useCache) {
    using namespace juce;
    if (files.empty()) {
      return;
    }

    VisibleRegions visibleRegions = fVisibleRegions.load();

    for (File const &f : files) {
      auto r = mcfile::je::Region::MakeRegion(PathFromFile(f));
      auto region = MakeRegion(r->fX, r->fZ);
      fPool->addTexturePackJob(region, useCache);
      fLoadingRegions.insert(region);
      visibleRegions.add(r->fX, r->fZ);
      if (auto found = fTextures.find(region); found == fTextures.end()) {
        fTextures[region] = std::make_unique<RegionTextureCache>(fWorldDirectory, fDimension, region);
      }
    }

    fVisibleRegions = visibleRegions;
    startLoadingTimer();
  }

  void startLoadingTimer() {
    if (!isTimerRunning()) {
      startTimer(16); // 60fps
    }
  }

  void viewportRegions(LookAt lookAt, int *minRx, int *minRz, int *maxRx, int *maxRz) {
    auto size = fSize.load();
    auto topLeft = GetMapCoordinateFromView(juce::Point<float>(0, 0), size, lookAt);
    auto rightBottom = GetMapCoordinateFromView(juce::Point<float>(size.x, size.y), size, lookAt);
    *minRx = mcfile::Coordinate::RegionFromBlock((int)floor(topLeft.x)) - 1;
    *minRz = mcfile::Coordinate::RegionFromBlock((int)floor(topLeft.y)) - 1;
    *maxRx = mcfile::Coordinate::RegionFromBlock((int)ceil(rightBottom.x)) + 2;
    *maxRz = mcfile::Coordinate::RegionFromBlock((int)ceil(rightBottom.y)) + 2;
  }

  void instantiateTextures(LookAt lookAt) {
    fTextureTrashBin.clear();

    bool loadingFinished = false;
    juce::File worldDirectory;
    Dimension dimension;
    std::vector<std::shared_ptr<TexturePackJob::Result>> remove;
    std::vector<std::pair<std::shared_ptr<TexturePackJob::Result>, float>> distances;
    int minRx, minRz, maxRx, maxRz;
    viewportRegions(lookAt, &minRx, &minRz, &maxRx, &maxRz);
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
        if (result->fRegion.first < minRx || maxRx < result->fRegion.first || result->fRegion.second < minRz || maxRz < result->fRegion.second) {
          fLoadingRegions.erase(result->fRegion);
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
        auto cache = std::make_unique<RegionTextureCache>(j->fWorldDirectory, j->fDimension, j->fRegion);
        cache->load(j->fPixels.get());
        if (before != fTextures.end()) {
          cache->fLoadTime = before->second->fLoadTime;
        }
        fTextures[j->fRegion] = std::move(cache);
      } else {
        if (before != fTextures.end()) {
          fTextures.erase(before);
        }
      }

      std::lock_guard<std::mutex> lock(fMut);
      auto it = fLoadingRegions.find(j->fRegion);
      if (it != fLoadingRegions.end()) {
        fLoadingRegions.erase(it);
        if (fLoadingRegions.empty()) {
          loadingFinished = true;
        }
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
    for (auto const &it : fTextures) {
      auto [rx, rz] = it.first;
      if (rx < minRx || maxRx < rx || rz < minRz || maxRz < rz) {
        if (it.second->fTexture) {
          it.second->fTexture.reset();
        }
      }
    }
    if (fPool) {
      int queued = 0;
      for (int rx = minRx; rx <= maxRx; rx++) {
        for (int rz = minRz; rz <= maxRz; rz++) {
          auto region = MakeRegion(rx, rz);
          if (auto found = fTextures.find(region); found != fTextures.end() && !found->second->fTexture) {
            {
              std::lock_guard<std::mutex> lock(fMut);
              if (fLoadingRegions.count(region) > 0) {
                continue;
              }
              fLoadingRegions.insert(region);
            }
            fPool->addTexturePackJob(MakeRegion(rx, rz), true);
            queued++;
          }
        }
      }
      if (queued > 0) {
        loadingFinished = false;
        startLoadingTimer();
      }
    }
    if (loadingFinished) {
      callAfterDelay(kFadeDurationMS, [this]() {
        std::lock_guard<std::mutex> lock(fMut);
        if (fLoadingRegions.empty()) {
          stopTimer();
        }
      });
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
    menu.showMenuAsync(PopupMenu::Options().withTargetScreenArea(juce::Rectangle<int>(pos, pos)), [this, e, dim, current](int menuId) {
      if (menuId != 1) {
        return;
      }
      Point<float> pinPos = getMapCoordinateFromView(e.getPosition().toFloat(), current);
      PinEdit edit;
      edit.fNew = true;
      auto pin = std::make_shared<Pin>();
      pin->fX = floor(pinPos.x);
      pin->fZ = floor(pinPos.y) + 1;
      pin->fDim = dim;
      edit.fPin = pin;
      TextInputDialog<PinEdit>::showAsync(this, TRANS("Please enter a pin name"), "", edit, this);
    });
  }

  void textInputDialogDidClickOkButton(juce::String input, PinEdit edit) override {
    using namespace juce;
    edit.fPin->fMessage = input;
    if (edit.fNew) {
      addPinComponent(edit.fPin);
      fWorldData.fPins.push_back(edit.fPin);
      saveWorldData();
      triggerRepaint();
    } else {
      updatePinComponent(edit.fPin);
      saveWorldData();
      triggerRepaint();
    }
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

  void updatePinComponent(std::shared_ptr<Pin> const &pin) {
    for (auto const &component : fPinComponents) {
      if (component->isPresenting(pin)) {
        component->updateSize();
        break;
      }
    }
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
    menu.showMenuAsync(PopupMenu::Options().withTargetScreenArea(juce::Rectangle<int>(screenPos, screenPos)), [this, pin](int menuId) {
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
        PinEdit edit;
        edit.fNew = false;
        edit.fPin = pin;
        TextInputDialog<PinEdit>::showAsync(this, TRANS("Please enter a pin name"), pin->fMessage, edit, this);
      }
    });
  }

  void handlePinDoubleClicked(std::shared_ptr<Pin> const &pin, juce::Point<int> screenPos) {
    using namespace juce;
    PinEdit edit;
    edit.fNew = false;
    edit.fPin = pin;
    TextInputDialog<PinEdit>::showAsync(this, TRANS("Please enter a pin name"), pin->fMessage, edit, this);
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

private:
  juce::OpenGLContext fGLContext;
  juce::File fWorldDirectory;
  WorldData fWorldData;
  Dimension fDimension;
  Edition fEdition;

  std::vector<std::unique_ptr<PinComponent>> fPinComponents;
  bool fShowPin;

  std::map<Region, std::unique_ptr<RegionTextureCache>> fTextures;
  std::deque<std::unique_ptr<RegionTextureCache>> fTextureTrashBin;
  std::unique_ptr<juce::OpenGLShaderProgram> fGLShader;
  std::unique_ptr<GLUniforms> fGLUniforms;
  std::unique_ptr<GLAttributes> fGLAttributes;
  std::unique_ptr<GLBuffer> fGLBuffer;
  std::unique_ptr<juce::OpenGLTexture> fGLPalette;
  std::unique_ptr<juce::OpenGLTexture> fGLPaletteJava;
  std::unique_ptr<juce::OpenGLTexture> fGLPaletteBedrock;

  std::atomic<LookAt> fLookAt;
  std::atomic<VisibleRegions> fVisibleRegions;
  std::atomic<juce::Point<int>> fSize;

  juce::Point<float> fCenterWhenDragStart;

  juce::Point<float> fMouse;

  std::unique_ptr<TexturePackThreadPool> fPool;
  std::deque<std::unique_ptr<TexturePackThreadPool>> fPoolTrashBin;
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

  OverScroller fScroller;
  std::deque<juce::MouseEvent> fLastDragPosition;
  TimerInstance fScrollerTimer;
  TimerInstance fAnimationTimer;

  juce::Atomic<float> fWaterOpticalDensity;
  juce::Atomic<bool> fWaterTranslucent;
  juce::Atomic<bool> fEnableBiome;
  juce::Atomic<int> fBiomeBlend;
  juce::Atomic<PaletteType> fPaletteType;
  juce::Atomic<LightingType> fLightingType;

  std::unique_ptr<RegionUpdateChecker> fRegionUpdateChecker;

  std::unique_ptr<juce::FileChooser> fFileChooser;
  std::deque<AsyncUpdateQueue> fAsyncUpdateQueue;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MapViewComponent)
};

} // namespace mcview
