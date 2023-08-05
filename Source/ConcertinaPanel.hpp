#pragma once

namespace mcview {

class ConcertinaPanel : public juce::Component {
  class PanelHeader : public juce::Component {
  public:
    PanelHeader() {
      setInterceptsMouseClicks(true, false);
      setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    void paint(juce::Graphics &g) override {
      if (!fHeader) {
        getLookAndFeel().drawConcertinaPanelHeader(g, getBounds(), fMouseOver, fMouseDown, fDummy, *this);
      }
    }

    void resized() override {
      if (fHeader) {
        fHeader->setBounds(getLocalBounds());
      }
    }

    void mouseDrag(juce::MouseEvent const &e) override {
      if (fOnDrag) {
        fOnDrag(this, e, fMouseDownOffset);
      }
    }

    void mouseDown(juce::MouseEvent const &e) override {
      fMouseDownOffset = e.getPosition();
      if (fHeader) {
        fHeader->mouseDown(e.getEventRelativeTo(fHeader));
      }
      fMouseDown = true;
      repaint();
    }

    void mouseUp(juce::MouseEvent const &e) override {
      if (fOnUp) {
        fOnUp();
      }
      if (fHeader) {
        fHeader->mouseUp(e.getEventRelativeTo(fHeader));
      }
      fMouseDown = false;
      repaint();
    }

    void mouseEnter(juce::MouseEvent const &e) override {
      if (fHeader) {
        fHeader->mouseEnter(e.getEventRelativeTo(fHeader));
      }
      fMouseOver = true;
      repaint();
    }

    void mouseExit(juce::MouseEvent const &e) override {
      if (fHeader) {
        fHeader->mouseExit(e.getEventRelativeTo(fHeader));
      }
      fMouseOver = false;
      repaint();
    }

    void setHeaderComponent(juce::Component *component, bool takeOwnership) {
      if (fHeader) {
        removeChildComponent(fHeader);
        fHeaderOwned.reset();
      }
      fHeader = component;
      fHeader->setInterceptsMouseClicks(false, true);
      if (takeOwnership) {
        fHeaderOwned.reset(component);
      }
      addAndMakeVisible(fHeader);
      resized();
    }

  public:
    std::function<void(PanelHeader *header, juce::MouseEvent const &e, juce::Point<int> mouseDownLocalOffset)> fOnDrag;
    std::function<void()> fOnUp;

  private:
    juce::Component *fHeader = nullptr;
    std::unique_ptr<juce::Component> fHeaderOwned;
    juce::ConcertinaPanel fDummy;
    bool fMouseDown = false;
    bool fMouseOver = false;
    juce::Point<int> fMouseDownOffset;
  };

  class Panel : public juce::Component {
  public:
    Panel(juce::Component *content, bool takeOwnership) : fContent(content) {
      fHeader.reset(new PanelHeader);
      fHeader->fOnDrag = [this](PanelHeader *header, juce::MouseEvent const &e, juce::Point<int> mouseDownLocalOffset) {
        if (fOnDrag) {
          fOnDrag(header, e, mouseDownLocalOffset);
        }
      };
      fHeader->fOnUp = [this]() {
        if (fOnUp) {
          fOnUp();
        }
      };
      addAndMakeVisible(*fHeader);

      if (takeOwnership) {
        fOwnership.reset(content);
      }
      fHeaderHeight = 32;
      fBodyHeight = content->getHeight();
      addAndMakeVisible(content);
    }

    void resized() override {
      auto bounds = getLocalBounds();
      fHeader->setBounds(bounds.removeFromTop(fHeaderHeight));
      fContent->setBounds(bounds);
    }

  public:
    std::unique_ptr<PanelHeader> fHeader;
    juce::Component *const fContent;
    int fHeaderHeight = 32;
    int fBodyHeight = 0;
    std::function<void(PanelHeader *header, juce::MouseEvent const &e, juce::Point<int> mouseDownLocalOffset)> fOnDrag;
    std::function<void()> fOnUp;

  private:
    std::unique_ptr<juce::Component> fOwnership;
  };

public:
  void resized() override {
    updatePanelSizes(-1);
  }

  void addPanel(int insertIndex, juce::Component *component, bool takeOwnership) {
    auto panel = std::make_shared<Panel>(component, takeOwnership);
    panel->fHeaderHeight = 32;
    panel->fBodyHeight = component->getHeight();
    panel->fOnDrag = [this](PanelHeader *header, juce::MouseEvent const &e, juce::Point<int> mouseDownLocalOffset) {
      onDragPanelHeader(header, e, mouseDownLocalOffset);
    };
    panel->fOnUp = [this]() {
      fHeightShareOnDragStart.clear();
    };
    fPanels.insert(fPanels.begin() + insertIndex, panel);
    addAndMakeVisible(*panel);
  }

  void removePanel(juce::Component *panelComponent) {
    for (size_t i = 0; i < fPanels.size(); i++) {
      if (fPanels[i]->fContent == panelComponent) {
        removeChildComponent(fPanels[i].get());
        fPanels.erase(fPanels.begin() + i);
        return;
      }
    }
  }

  int getNumPanels() const noexcept {
    return (int)fPanels.size();
  }

  juce::Component *getPanel(int index) const noexcept {
    return fPanels[index]->fContent;
  }

  bool setPanelSize(juce::Component *panelComponent, int newHeightIncludingHeaderHeight, bool animate) {
    int index = -1;
    for (int i = 0; i < (int)fPanels.size(); i++) {
      if (fPanels[i]->fContent == panelComponent) {
        index = i;
        break;
      }
    }
    if (index < 0) {
      return false;
    }
    int prev = fPanels[index]->fBodyHeight;
    fPanels[index]->fBodyHeight = std::max(newHeightIncludingHeaderHeight - fPanels[index]->fHeaderHeight, 0);
    updatePanelSizes(index);
    return prev != fPanels[index]->fBodyHeight;
  }

  void setPanelHeaderSize(juce::Component *panelComponent, int headerSize) {
    int index = -1;
    for (int i = 0; i < (int)fPanels.size(); i++) {
      if (fPanels[i]->fContent == panelComponent) {
        index = i;
        break;
      }
    }
    if (index < 0) {
      return;
    }
    fPanels[index]->fHeaderHeight = std::max(headerSize, 0);
    updatePanelSizes(index);
  }

  void setCustomPanelHeader(juce::Component *panelComponent, juce::Component *customHeaderComponent, bool takeOwnership) {
    if (!panelComponent || !customHeaderComponent) {
      return;
    }
    int index = -1;
    for (int i = 0; i < (int)fPanels.size(); i++) {
      if (fPanels[i]->fContent == panelComponent) {
        index = i;
        break;
      }
    }
    if (index < 0) {
      if (takeOwnership) {
        delete customHeaderComponent;
      }
      return;
    }
    fPanels[index]->fHeader->setHeaderComponent(customHeaderComponent, takeOwnership);
    resized();
  }

private:
  void onDragPanelHeader(PanelHeader *header, juce::MouseEvent const &e, juce::Point<int> mouseDownLocalOffset) {
    int index = -1;
    for (int i = 0; i < (int)fPanels.size(); i++) {
      if (fPanels[i]->fHeader.get() == header) {
        index = i;
        break;
      }
    }
    if (index < 0) {
      return;
    }
    if (index == 0) {
      return;
    }
    if (fHeightShareOnDragStart.empty()) {
      for (auto const &panel : fPanels) {
        fHeightShareOnDragStart.push_back(panel->fBodyHeight + panel->fHeaderHeight);
      }
    }
    int headerY = e.getScreenY() - getScreenY() - mouseDownLocalOffset.y;
    int minY = 0;
    for (int i = 0; i < index; i++) {
      minY += fPanels[i]->fHeaderHeight;
    }
    int maxY = getHeight();
    for (int i = index; i < (int)fPanels.size(); i++) {
      maxY -= fPanels[i]->fHeaderHeight;
    }
    int clampedHeaderY = std::clamp(headerY, minY, maxY);
    int remainingTop = clampedHeaderY;
    float sumHeightShare = 0;
    for (int i = index - 1; i >= 0; i--) {
      sumHeightShare += fHeightShareOnDragStart[i];
    }
    for (int i = index - 1; i >= 0; i--) {
      int headerHeight = fPanels[i]->fHeaderHeight;
      if (i == 0) {
        fPanels[i]->fBodyHeight = std::max(remainingTop - headerHeight, 0);
      } else {
        int newHeight = std::max((int)round(clampedHeaderY * fHeightShareOnDragStart[i] / sumHeightShare) - headerHeight, 0);
        fPanels[i]->fBodyHeight = newHeight;
        remainingTop -= newHeight + headerHeight;
      }
    }
    int remainingBottom = getHeight() - clampedHeaderY;
    sumHeightShare = 0;
    for (int i = index; i < (int)fPanels.size(); i++) {
      sumHeightShare += fHeightShareOnDragStart[i];
    }
    for (int i = index; i < (int)fPanels.size(); i++) {
      int headerHeight = fPanels[i]->fHeaderHeight;
      if (i == (int)fPanels.size() - 1) {
        fPanels[i]->fBodyHeight = std::max(remainingBottom - headerHeight, 0);
      } else {
        int newHeight = std::max((int)round((getHeight() - clampedHeaderY) * fHeightShareOnDragStart[i] / sumHeightShare) - headerHeight, 0);
        fPanels[i]->fBodyHeight = newHeight;
        remainingBottom -= newHeight + headerHeight;
      }
    }
    updatePanelSizes(-1);
  }

  void updatePanelSizes(int priorityPanelIndex) {
    auto bounds = getLocalBounds();
    int availableHeight = bounds.getHeight();

    int availableHeightExcludingPriorityPanel = 0;
    for (int i = 0; i < (int)fPanels.size(); i++) {
      if (i != priorityPanelIndex) {
        availableHeightExcludingPriorityPanel += fPanels[i]->fBodyHeight + fPanels[i]->fHeaderHeight;
      }
    }
    int priorityPanelHeight = 0;
    if (priorityPanelIndex >= 0) {
      priorityPanelHeight = std::clamp(fPanels[priorityPanelIndex]->fBodyHeight + fPanels[priorityPanelIndex]->fHeaderHeight, 0, availableHeight);
    }
    int remainingHeight = availableHeight - priorityPanelHeight;

    for (int i = 0; i < (int)fPanels.size(); i++) {
      int headerHeight = fPanels[i]->fHeaderHeight;
      if (i == (int)fPanels.size() - 1) {
        fPanels[i]->setBounds(bounds);
        fPanels[i]->fBodyHeight = std::max(bounds.getHeight() - headerHeight, 0);
      } else if (i == priorityPanelIndex) {
        fPanels[i]->setBounds(bounds.removeFromTop(priorityPanelHeight));
        fPanels[i]->fBodyHeight = std::max(priorityPanelHeight - headerHeight, 0);
      } else {
        int newHeight = std::max((int)round((fPanels[i]->fBodyHeight + headerHeight) / (double)availableHeightExcludingPriorityPanel * remainingHeight), 0);
        fPanels[i]->setBounds(bounds.removeFromTop(newHeight));
        fPanels[i]->fBodyHeight = std::max(newHeight - headerHeight, 0);
      }
    }
  }

private:
  std::vector<std::shared_ptr<Panel>> fPanels;
  std::vector<float> fHeightShareOnDragStart;
};

} // namespace mcview
