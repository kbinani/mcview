#pragma once

namespace mcview {

class LeftPanelHeader : public juce::Component {
public:
  LeftPanelHeader(juce::ConcertinaPanel *parent, juce::String const &title)
      : fMouseDown(false), fMouseOver(false), fParent(parent), fTitle(title) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
  }

  void mouseDown(juce::MouseEvent const &e) override {
    fMouseDown = true;
    repaint();
  }

  void mouseUp(juce::MouseEvent const &e) override {
    fMouseDown = false;
    repaint();
  }

  void mouseEnter(juce::MouseEvent const &e) override {
    fMouseOver = true;
    repaint();
  }

  void mouseExit(juce::MouseEvent const &e) override {
    fMouseOver = false;
    repaint();
  }

  void paint(juce::Graphics &g) override {
    {
      g.saveState();
      defer {
        g.restoreState();
      };
      getLookAndFeel().drawConcertinaPanelHeader(g, getBounds(), fMouseOver, fMouseDown, *fParent, *this);
    }
    {
      g.saveState();
      defer {
        g.restoreState();
      };
      g.setColour(juce::Colours::white);
      int const margin = 10;
      g.drawFittedText(fTitle, margin, 0, getWidth() - 2 * margin, getHeight(), juce::Justification::centredLeft, 1);
    }
  }

private:
  static int constexpr kButtonSize = 20;

  bool fMouseDown;
  bool fMouseOver;
  juce::ConcertinaPanel *const fParent;
  juce::String fTitle;
};

} // namespace mcview
