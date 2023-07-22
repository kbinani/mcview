#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

class PNGWriter {
public:
  PNGWriter(int width, int height, juce::OutputStream &stream);
  ~PNGWriter();

  void writeRow(juce::PixelARGB *row);

private:
  juce::OutputStream &fStream;
  int fWidth;
  int fHeight;
  void *fWriteStruct;
  void *fInfoStruct;
  juce::HeapBlock<juce::uint8> fRowData;
};
