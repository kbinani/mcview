#pragma once

static const juce::Identifier kX = juce::Identifier("x");
static const juce::Identifier kZ("z");
static const juce::Identifier kMessage("message");
static const juce::Identifier kDimension("dimension");

static const juce::String kNether("the_nether");
static const juce::String kEnd("the_end");
static const juce::String kOverworld("overworld");

class Pin {
public:
  static bool Parse(juce::var pin, Pin &dest) {
    if (!pin.hasProperty(kX) || !pin.hasProperty(kZ) || !pin.hasProperty(kMessage) || !pin.hasProperty(kDimension)) {
      return false;
    }
    juce::var x = pin.getProperty(kX, 0);
    juce::var z = pin.getProperty(kZ, 0);
    juce::var message = pin.getProperty(kMessage, "");
    juce::var dim = pin.getProperty(kDimension, "");
    if (!x.isInt() || !z.isInt() || !message.isString() || !dim.isString()) {
      return false;
    }
    dest.fX = x;
    dest.fZ = z;
    dest.fMessage = message;
    dest.fDim = DimensionFromString(dim.toString());
    return true;
  }
  juce::var toVar() const {
    std::unique_ptr<juce::DynamicObject> obj(new juce::DynamicObject());
    obj->setProperty(kX, fX);
    obj->setProperty(kZ, fZ);
    obj->setProperty(kMessage, fMessage);
    obj->setProperty(kDimension, DimensionToString(fDim));
    return juce::var(obj.release());
  }

private:
  static Dimension DimensionFromString(juce::String v) {
    if (v == kNether) {
      return Dimension::TheNether;
    } else if (v == kEnd) {
      return Dimension::TheEnd;
    } else {
      return Dimension::Overworld;
    }
  }

  static juce::String DimensionToString(Dimension dim) {
    switch (dim) {
    case Dimension::TheNether:
      return kNether;
    case Dimension::TheEnd:
      return kEnd;
    default:
      return kOverworld;
    }
  }

public:
  int fX;
  int fZ;
  juce::String fMessage;
  Dimension fDim;
};
