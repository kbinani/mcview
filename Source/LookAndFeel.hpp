#pragma once

namespace mcview {

class LookAndFeel : public juce::LookAndFeel_V4 {
public:
  LookAndFeel() {
    auto preferredLanguages = LocalizationHelper::PreferredLanguages();
    juce::String fontFamily = "NotoSans";
    float scale = 1.15f;

    std::map<NotoSansTypeface::FontLanguage, NotoSansTypeface::Asset> regularAssets = {
        {NotoSansTypeface::FontLanguage::Neutral, {.fData = BinaryData::NotoSansRegular_ttf, .fSize = BinaryData::NotoSansRegular_ttfSize}},                //
        {NotoSansTypeface::FontLanguage::Japanese, {.fData = BinaryData::NotoSansJPRegular_ttf, .fSize = BinaryData::NotoSansJPRegular_ttfSize}},           //
        {NotoSansTypeface::FontLanguage::SimplifiedChinese, {.fData = BinaryData::NotoSansSCRegular_otf, .fSize = BinaryData::NotoSansSCRegular_otfSize}},  //
        {NotoSansTypeface::FontLanguage::TraditionalChinese, {.fData = BinaryData::NotoSansTCRegular_otf, .fSize = BinaryData::NotoSansTCRegular_otfSize}}, //
    };
    auto regular = new NotoSansTypeface(fontFamily, scale, false, regularAssets, preferredLanguages);
    fNotoSansRegular = juce::ReferenceCountedObjectPtr<juce::Typeface>(regular);

    std::map<NotoSansTypeface::FontLanguage, NotoSansTypeface::Asset> boldAssets = {
        {NotoSansTypeface::FontLanguage::Neutral, {.fData = BinaryData::NotoSansBold_ttf, .fSize = BinaryData::NotoSansBold_ttfSize}},                //
        {NotoSansTypeface::FontLanguage::Japanese, {.fData = BinaryData::NotoSansJPBold_ttf, .fSize = BinaryData::NotoSansJPBold_ttfSize}},           //
        {NotoSansTypeface::FontLanguage::SimplifiedChinese, {.fData = BinaryData::NotoSansSCBold_otf, .fSize = BinaryData::NotoSansSCBold_otfSize}},  //
        {NotoSansTypeface::FontLanguage::TraditionalChinese, {.fData = BinaryData::NotoSansTCBold_otf, .fSize = BinaryData::NotoSansTCBold_otfSize}}, //
    };
    auto bold = new NotoSansTypeface(fontFamily, scale, true, boldAssets, preferredLanguages);
    fNotoSansBold = juce::ReferenceCountedObjectPtr<juce::Typeface>(bold);
  }

  juce::Typeface::Ptr getTypefaceForFont(juce::Font const &font) override {
    if (font.getTypefaceName() == "<Sans-Serif>") {
      if (font.isBold()) {
        return fNotoSansBold;
      } else {
        return fNotoSansRegular;
      }
    } else {
      return juce::Font::getDefaultTypefaceForFont(font);
    }
  }

private:
  juce::Typeface::Ptr fNotoSansRegular;
  juce::Typeface::Ptr fNotoSansBold;
};

} // namespace mcview
