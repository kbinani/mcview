#include "PinComponent.h"
#include "defer.h"
#include "WorldData.h"
#include "GraphicsHelper.h"

static int const pinHeadRadius = 6;
static int const pinNameFontSize = 14;
static int const stemLength = 16;
static Point<float> const pinHilightOffset(-2, -2);
static int const pinHeadHilightRadius = 3;
static Point<float> const pinHeadShadowOffset(3, 3);
static float const pinHeadShadowAlpha = 0.5f;
static float const stemThickness = 2;

static Rectangle<float> PinNameBounds(Pin pin, Font font, Point<float> pos)
{
    int const pad = 4;
    float stringWidth = font.getStringWidthFloat(pin.fMessage);
    Rectangle<float> stringBounds(pos.x + pinHeadRadius + pad, pos.y - stemLength - font.getHeight() / 2 - pad, pad + stringWidth + pad, pad + font.getHeight() + pad);
    return stringBounds;
}

PinComponent::PinComponent(std::shared_ptr<Pin> const& pin)
    : fPin(pin)
{
    int const height = stemLength + pinHeadRadius * 2;
    Point<float> pos(pinHeadRadius, height);
    Font font(pinNameFontSize);
    Rectangle<float> nameBounds = PinNameBounds(*fPin, font, pos);
    int const width = nameBounds.getRight();
    setSize(width, jmax(height, height - (int)nameBounds.getY()));
}

void PinComponent::paint(Graphics &g)
{
    static Colour const pinHeadColour = Colours::red;
    static Colour const pinHeadHilightColour = pinHeadColour.brighter().brighter();
    static Colour const stemColour = Colours::white;

    g.saveState();
    defer {
        g.restoreState();
    };

    Point<float> pos(pinHeadRadius, getHeight());
    Font font = Font(pinNameFontSize);

    g.setColour(Colours::black.withAlpha(pinHeadShadowAlpha));
    g.fillEllipse(pos.x + pinHeadShadowOffset.x - pinHeadRadius, pos.y + pinHeadShadowOffset.y - pinHeadRadius - stemLength, pinHeadRadius * 2, pinHeadRadius * 2);

    g.setColour(stemColour);
    g.drawLine(pos.x, pos.y - stemLength, pos.x, pos.y, stemThickness);

    g.setColour(pinHeadColour);
    g.fillEllipse(pos.x - pinHeadRadius, pos.y - pinHeadRadius - stemLength, pinHeadRadius * 2, pinHeadRadius * 2);

    g.setColour(pinHeadHilightColour);
    g.fillEllipse(pos.x + pinHilightOffset.x - pinHeadHilightRadius, pos.y + pinHilightOffset.y - pinHeadHilightRadius - stemLength, pinHeadHilightRadius * 2, pinHeadHilightRadius * 2);

    g.setColour(Colours::black.withAlpha(0.5f));
    auto stringBounds = PinNameBounds(*fPin, font, pos);
    g.fillRect(stringBounds);
    g.setColour(Colours::white);
    g.setFont(pinNameFontSize);
    GraphicsHelper::DrawText(g, fPin->fMessage, stringBounds, Justification::centred);
}

void PinComponent::updatePinPosition(Point<float> pos)
{
    int const width = getWidth();
    int const height = getHeight();
    setBounds(pos.x - pinHeadRadius, pos.y - height, width, height);
}
