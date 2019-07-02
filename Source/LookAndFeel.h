#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "GraphicsHelper.h"
#include "DirectoryBrowserComponent.h"

namespace mcview
{

class LookAndFeel : public juce::LookAndFeel_V4
{
    void drawButtonText(Graphics &g, TextButton &button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        LookAndFeel_V4::drawButtonText(g, button, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    }
    
    void drawLabel(Graphics &g, Label &label) override
    {
        g.fillAll (label.findColour (Label::backgroundColourId));
        
        if (! label.isBeingEdited())
        {
            auto alpha = label.isEnabled() ? 1.0f : 0.5f;
            const Font font (getLabelFont (label));
            
            g.setColour (label.findColour (Label::textColourId).withMultipliedAlpha (alpha));
            g.setFont (font);
            
            auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());
            
            GraphicsHelper::DrawFittedText(g, label.getText(), textArea, label.getJustificationType(),
                                           jmax (1, (int) (textArea.getHeight() / font.getHeight())),
                                           label.getMinimumHorizontalScale());
            
            g.setColour (label.findColour (Label::outlineColourId).withMultipliedAlpha (alpha));
        }
        else if (label.isEnabled())
        {
            g.setColour (label.findColour (Label::outlineColourId));
        }
        
        g.drawRect (label.getLocalBounds());
    }
    
    void drawGroupComponentOutline (Graphics &g, int width, int height, const String &text, const Justification &position, GroupComponent &group) override
    {
        const float textH = 15.0f;
        const float indent = 3.0f;
        const float textEdgeGap = 4.0f;
        auto cs = 5.0f;
        
        Font f (textH);
        
        Path p;
        auto x = indent;
        auto y = f.getAscent() - 3.0f;
        auto w = jmax (0.0f, width - x * 2.0f);
        auto h = jmax (0.0f, height - y  - indent);
        cs = jmin (cs, w * 0.5f, h * 0.5f);
        auto cs2 = 2.0f * cs;
        
        auto textW = text.isEmpty() ? 0 : jlimit (0.0f, jmax (0.0f, w - cs2 - textEdgeGap * 2), f.getStringWidth (text) + textEdgeGap * 2.0f);
        auto textX = cs + textEdgeGap;
        
        if (position.testFlags (Justification::horizontallyCentred))
            textX = cs + (w - cs2 - textW) * 0.5f;
        else if (position.testFlags (Justification::right))
            textX = w - cs - textW - textEdgeGap;
        
        p.startNewSubPath (x + textX + textW, y);
        p.lineTo (x + w - cs, y);
        
        p.addArc (x + w - cs2, y, cs2, cs2, 0, MathConstants<float>::halfPi);
        p.lineTo (x + w, y + h - cs);
        
        p.addArc (x + w - cs2, y + h - cs2, cs2, cs2, MathConstants<float>::halfPi, MathConstants<float>::pi);
        p.lineTo (x + cs, y + h);
        
        p.addArc (x, y + h - cs2, cs2, cs2, MathConstants<float>::pi, MathConstants<float>::pi * 1.5f);
        p.lineTo (x, y + cs);
        
        p.addArc (x, y, cs2, cs2, MathConstants<float>::pi * 1.5f, MathConstants<float>::twoPi);
        p.lineTo (x + textX, y);
        
        auto alpha = group.isEnabled() ? 1.0f : 0.5f;
        
        g.setColour (group.findColour (GroupComponent::outlineColourId)
                     .withMultipliedAlpha (alpha));
        
        g.strokePath (p, PathStrokeType (2.0f));
        
        g.setColour (group.findColour (GroupComponent::textColourId)
                     .withMultipliedAlpha (alpha));
        g.setFont (f);
        GraphicsHelper::DrawText (g,text,
                                  roundToInt (x + textX), 0,
                                  roundToInt (textW),
                                  roundToInt (textH),
                                  Justification::centred, true);
    }
    
    void drawToggleButton(Graphics &g, ToggleButton &button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto fontSize = jmin (15.0f, button.getHeight() * 0.75f);
        auto tickWidth = fontSize * 1.1f;
        
        drawTickBox (g, button, 4.0f, (button.getHeight() - tickWidth) * 0.5f,
                     tickWidth, tickWidth,
                     button.getToggleState(),
                     button.isEnabled(),
                     shouldDrawButtonAsHighlighted,
                     shouldDrawButtonAsDown);
        
        g.setColour (button.findColour (ToggleButton::textColourId));
        g.setFont (fontSize);
        
        if (! button.isEnabled())
            g.setOpacity (0.5f);
        
        GraphicsHelper::DrawFittedText(g, button.getButtonText(),
                                       button.getLocalBounds().withTrimmedLeft (roundToInt (tickWidth) + 10)
                                       .withTrimmedRight (2),
                                       Justification::centredLeft, 10);
    }
};
    
}
