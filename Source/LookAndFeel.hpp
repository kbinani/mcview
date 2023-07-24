#pragma once

namespace mcview {

class LookAndFeel : public juce::LookAndFeel_V4 {
public:
  LookAndFeel() {
    auto preferredLanguages = LocalizationHelper::PreferredLanguages();
    juce::String fontFamily = "NotoSans";
    float scale = 1.15f;

    std::map<NotoSansTypeface::FontLanguage, NotoSansTypeface::Asset> regularAssets = {
        {NotoSansTypeface::FontLanguage::Neutral, {.fData = FontData::NotoSansRegular_ttf, .fSize = FontData::NotoSansRegular_ttfSize}},                //
        {NotoSansTypeface::FontLanguage::Japanese, {.fData = FontData::NotoSansJPRegular_ttf, .fSize = FontData::NotoSansJPRegular_ttfSize}},           //
        {NotoSansTypeface::FontLanguage::SimplifiedChinese, {.fData = FontData::NotoSansSCRegular_otf, .fSize = FontData::NotoSansSCRegular_otfSize}},  //
        {NotoSansTypeface::FontLanguage::TraditionalChinese, {.fData = FontData::NotoSansTCRegular_otf, .fSize = FontData::NotoSansTCRegular_otfSize}}, //
    };
    auto regular = new NotoSansTypeface(fontFamily, scale, false, regularAssets, preferredLanguages);
    fNotoSansRegular = juce::ReferenceCountedObjectPtr<juce::Typeface>(regular);

    std::map<NotoSansTypeface::FontLanguage, NotoSansTypeface::Asset> boldAssets = {
        {NotoSansTypeface::FontLanguage::Neutral, {.fData = FontData::NotoSansBold_ttf, .fSize = FontData::NotoSansBold_ttfSize}},                //
        {NotoSansTypeface::FontLanguage::Japanese, {.fData = FontData::NotoSansJPBold_ttf, .fSize = FontData::NotoSansJPBold_ttfSize}},           //
        {NotoSansTypeface::FontLanguage::SimplifiedChinese, {.fData = FontData::NotoSansSCBold_otf, .fSize = FontData::NotoSansSCBold_otfSize}},  //
        {NotoSansTypeface::FontLanguage::TraditionalChinese, {.fData = FontData::NotoSansTCBold_otf, .fSize = FontData::NotoSansTCBold_otfSize}}, //
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
