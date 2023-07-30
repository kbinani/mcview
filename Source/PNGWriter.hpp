#pragma once

namespace mcview {

class PNGWriter {
public:
  PNGWriter(int width, int height, juce::OutputStream &stream);
  ~PNGWriter();

  void writeRow(juce::PixelARGB *row);

private:
  juce::OutputStream &fStream;
  int fWidth;
  void *fWriteStruct;
  void *fInfoStruct;
  juce::HeapBlock<juce::uint8> fRowData;
};

} // namespace mcview
