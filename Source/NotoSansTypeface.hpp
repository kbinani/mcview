#pragma once

namespace mcview {

class NotoSansTypeface : public juce::CustomTypeface {
public:
  enum FontLanguage : int {
    Neutral,
    Japanese,
    TraditionalChinese,
    SimplifiedChinese,
  };

  struct Asset {
    char const *const fData;
    int const fSize;
  };

  NotoSansTypeface(juce::String fontFamily, float scale, bool bold, std::map<FontLanguage, Asset> const &assets, std::vector<juce::String> const &preferredLanguages) : fScale(scale), fAssets(assets) {
    std::vector<FontLanguage> remaining = {Japanese, SimplifiedChinese, TraditionalChinese};

    for (size_t i = 0; i < preferredLanguages.size(); i++) {
      juce::String lang = preferredLanguages[i];
      if (lang == "ja-JP") {
        if (auto found = std::find(remaining.begin(), remaining.end(), Japanese); found != remaining.end()) {
          fOrder.push_back(Japanese);
          remaining.erase(found);
        }
      } else if (lang.startsWith("zh-Hans")) {
        if (auto found = std::find(remaining.begin(), remaining.end(), SimplifiedChinese); found != remaining.end()) {
          fOrder.push_back(SimplifiedChinese);
          remaining.erase(found);
        }
      } else if (lang.startsWith("zh-Hant")) {
        if (auto found = std::find(remaining.begin(), remaining.end(), TraditionalChinese); found != remaining.end()) {
          fOrder.push_back(TraditionalChinese);
          remaining.erase(found);
        }
      }
    }
    while (!remaining.empty()) {
      fOrder.push_back(remaining[0]);
      remaining.erase(remaining.begin());
    }
    fOrder.push_back(Neutral);
    setCharacteristics(fontFamily, 0.801104963f, bold, false, L' ');
  }

  bool loadGlyphIfPossible(juce::juce_wchar c) override {
    using namespace juce;
    for (size_t i = 0; i < fOrder.size(); i++) {
      Typeface *typeface = ensureTypeface(fOrder[i]);
      if (!typeface) {
        continue;
      }

      Array<int> glyphIndexes;
      Array<float> offsets;
      typeface->getGlyphPositions(String::charToString(c), glyphIndexes, offsets);

      const int glyphIndex = glyphIndexes.getFirst();
      if (glyphIndex == 65535) {
        continue;
      }

      if (glyphIndex >= 0 && glyphIndexes.size() > 0) {
        auto glyphWidth = offsets[1];

        Path p;
        typeface->getOutlineForGlyph(glyphIndex, p);

        addGlyph(c, p, glyphWidth);
        return true;
      }
    }

    return false;
  }

  float getStringWidth(juce::String const &text) override {
    return juce::CustomTypeface::getStringWidth(text) * fScale;
  }

  void getGlyphPositions(juce::String const &text, juce::Array<int> &glyphs, juce::Array<float> &xOffsets) override {
    using namespace juce;
    CustomTypeface::getGlyphPositions(text, glyphs, xOffsets);
    for (int i = 0; i < xOffsets.size(); i++) {
      xOffsets.set(i, xOffsets[i] * fScale);
    }
  }

  bool getOutlineForGlyph(int glyphNumber, juce::Path &path) override {
    juce::Path p;
    bool ok = juce::CustomTypeface::getOutlineForGlyph(glyphNumber, p);
    if (!ok) {
      return false;
    }
    p.applyTransform(juce::AffineTransform::scale(fScale, fScale));
    path = p;
    return ok;
  }

  juce::EdgeTable *getEdgeTableForGlyph(int glyphNumber, juce::AffineTransform const &transform, float fontHeight) override {
    using namespace juce;
    AffineTransform t = transform.scaled(fScale, fScale);
    return CustomTypeface::getEdgeTableForGlyph(glyphNumber, t, fontHeight);
  }

private:
  juce::Typeface *ensureTypeface(FontLanguage language) {
    using namespace juce;
    if (fLoadedTypefaces.count(language) == 0) {
      fLoadedTypefaces[language] = nullptr;
      if (auto found = fAssets.find(language); found != fAssets.end() && found->second.fSize > 0) {
        if (auto ptr = Typeface::createSystemTypefaceFor(found->second.fData, (size_t)found->second.fSize); ptr) {
          fLoadedTypefaces[language] = ptr;
        }
      }
    }
    return fLoadedTypefaces[language].get();
  }

private:
  float const fScale;
  std::map<FontLanguage, juce::Typeface::Ptr> fLoadedTypefaces;
  std::vector<FontLanguage> fOrder;
  std::map<FontLanguage, Asset> fAssets;
};

} // namespace mcview
