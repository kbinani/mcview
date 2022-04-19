#include "PNGWriter.h"

using namespace juce;

PNGWriter::PNGWriter(int width, int height, OutputStream &stream)
    : fStream(stream)
    , fWidth(width)
    , fHeight(height)
    , fWriteStruct(nullptr)
    , fInfoStruct(nullptr)
    , fRowData(width * 4)
{

}

PNGWriter::~PNGWriter()
{

}

void PNGWriter::writeRow(PixelARGB *row)
{

}
