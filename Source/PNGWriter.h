#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class PNGWriter {
public:
    PNGWriter(int width, int height, OutputStream &stream);
    ~PNGWriter();

    void writeRow(PixelARGB *row);

private:
    OutputStream &fStream;
    int fWidth;
    int fHeight;
    void *fWriteStruct;
    void *fInfoStruct;
    HeapBlock<uint8> fRowData;
};
