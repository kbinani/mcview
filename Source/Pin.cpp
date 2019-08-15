#include "Pin.h"

static const Identifier kX("x");
static const Identifier kZ("z");
static const Identifier kMessage("message");
static const Identifier kDimension("dimension");

static const String kNether("the_nether");
static const String kEnd("the_end");
static const String kOverworld("overworld");

static Dimension DimensionFromString(String v)
{
    if (v == kNether) {
        return Dimension::TheNether;
    } else if (v == kEnd) {
        return Dimension::TheEnd;
    } else {
        return Dimension::Overworld;
    }
}

static String DimensionToString(Dimension dim)
{
    switch (dim) {
        case Dimension::TheNether:
            return kNether;
        case Dimension::TheEnd:
            return kEnd;
        default:
            return kOverworld;
    }
}

bool Pin::Parse(var pin, Pin &dest)
{
    if (!pin.hasProperty(kX) || !pin.hasProperty(kZ) || !pin.hasProperty(kMessage) || !pin.hasProperty(kDimension)) {
        return false;
    }
    var x = pin.getProperty(kX, 0);
    var z = pin.getProperty(kZ, 0);
    var message = pin.getProperty(kMessage, "");
    var dim = pin.getProperty(kDimension, "");
    if (!x.isInt() || !z.isInt() || !message.isString() || !dim.isString()) {
        return false;
    }
    dest.fX = x;
    dest.fZ = z;
    dest.fMessage = message;
    dest.fDim = DimensionFromString(dim.toString());
    return true;
}

var Pin::toVar() const
{
    std::unique_ptr<DynamicObject> obj(new DynamicObject());
    obj->setProperty(kX, fX);
    obj->setProperty(kZ, fZ);
    obj->setProperty(kMessage, fMessage);
    obj->setProperty(kDimension, DimensionToString(fDim));
    return var(obj.release());
}

