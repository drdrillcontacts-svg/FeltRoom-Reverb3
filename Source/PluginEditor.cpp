#include "PluginEditor.h"
#include "BinaryData.h"
namespace {
struct Preset {const char* name;std::array<float,8> p;};
const Preset factory[]={
 {"Intimate Room",{20,3.5f,70,45,100,12000,35,0}},
 {"Warm Felt",{12,2.4f,52,72,160,8500,29,0}},
 {"Clear Piano",{16,2.0f,55,20,120,18000,24,2}},
 {"Small Studio",{7,1.1f,24,48,170,14000,20,0}},
 {"Bright Plate",{24,3.1f,68,12,210,19000,34,2}},
 {"Vocal Plate",{42,2.8f,63,32,250,14000,26,2}},
 {"Vocal Chamber",{36,2.3f,55,44,230,13000,30,3}},
 {"Acoustic Guitar",{14,1.8f,46,40,180,15000,24,3}},
 {"Wide Concert Hall",{52,6.8f,96,25,90,16000,42,1}},
 {"Dark Hall",{48,7.2f,90,80,120,7800,44,1}},
 {"Vintage Tape Space",{38,4.5f,68,83,240,7000,39,4}},
 {"Ambient Cloud",{76,10.0f,100,18,140,17000,60,5}},
 {"Dreamy Keys",{65,8.0f,92,50,170,11000,52,5}},
 {"Percussion Room",{5,0.8f,30,36,280,16000,18,0}},
 {"Long Cinematic",{92,11.5f,100,42,100,12500,63,5}},
 {"Soft Strings",{33,5.5f,78,60,140,11500,35,1}}
};
}
void FeltRoomEditor::CopperLook::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                                        float pos, float start, float end, juce::Slider&)
{
    // Premium black-metal knob with a pink-to-violet illuminated value ring.
    const auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) w, (float) h).reduced(9.0f);
    const auto centre = bounds.getCentre();
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.385f;
    const float ringRadius = radius + 3.5f;
    const float angle = start + (end - start) * juce::jlimit(0.0f, 1.0f, pos);

    // Soft shadow and machined outer rim.
    g.setColour(juce::Colours::black.withAlpha(0.68f));
    g.fillEllipse(centre.x - radius - 5.0f, centre.y - radius - 2.0f,
                  (radius + 5.0f) * 2.0f, (radius + 5.0f) * 2.0f);
    g.setColour(juce::Colour(0xff51464a));
    g.drawEllipse(centre.x - ringRadius - 1.0f, centre.y - ringRadius - 1.0f,
                  (ringRadius + 1.0f) * 2.0f, (ringRadius + 1.0f) * 2.0f, 1.0f);

    // Dark unlit arc.
    juce::Path track;
    track.addCentredArc(centre.x, centre.y, ringRadius, ringRadius, 0.0f, start, end, true);
    g.setColour(juce::Colour(0xff393239));
    g.strokePath(track, juce::PathStrokeType(3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Segmented gradient arc gives the illuminated pink/purple hardware look.
    const float activeEnd = start + (end - start) * juce::jlimit(0.0f, 1.0f, pos);
    constexpr int segments = 56;
    const auto pink = juce::Colour(0xffff4f9b);
    const auto violet = juce::Colour(0xff9b6bff);
    for (int i = 0; i < segments; ++i)
    {
        const float t0 = (float) i / (float) segments;
        const float t1 = (float) (i + 1) / (float) segments;
        const float a0 = start + (activeEnd - start) * t0;
        const float a1 = start + (activeEnd - start) * t1;
        if (a0 >= activeEnd - 0.0001f) break;
        juce::Path arc;
        arc.addCentredArc(centre.x, centre.y, ringRadius, ringRadius, 0.0f, a0, a1, true);
        g.setColour(pink.interpolatedWith(violet, t0));
        g.strokePath(arc, juce::PathStrokeType(3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Subtle bloom on the active ring, kept restrained so labels remain crisp.
    if (pos > 0.001f)
    {
        juce::Path glowArc;
        glowArc.addCentredArc(centre.x, centre.y, ringRadius, ringRadius, 0.0f, start, activeEnd, true);
        g.setColour(juce::Colour(0xffff4f9b).withAlpha(0.10f));
        g.strokePath(glowArc, juce::PathStrokeType(7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Brushed, near-black knob face with a soft top-left sheen.
    auto face = juce::Rectangle<float>(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
    juce::ColourGradient faceGradient(juce::Colour(0xff29272b), face.getX(), face.getY(),
                                      juce::Colour(0xff08090b), face.getRight(), face.getBottom(), false);
    g.setGradientFill(faceGradient);
    g.fillEllipse(face);
    g.setColour(juce::Colour(0xff4a444b).withAlpha(0.9f));
    g.drawEllipse(face, 1.0f);
    g.setColour(juce::Colours::white.withAlpha(0.045f));
    g.drawEllipse(face.reduced(3.0f), 1.0f);

    // Clear ivory indicator with a small illuminated tip.
    const float pointerLength = radius * 0.70f;
    const juce::Point<float> tip(centre.x + std::sin(angle) * pointerLength,
                                 centre.y - std::cos(angle) * pointerLength);
    const juce::Point<float> base(centre.x + std::sin(angle) * (radius * 0.18f),
                                  centre.y - std::cos(angle) * (radius * 0.18f));
    g.setColour(juce::Colour(0xfff8eee8).withAlpha(0.96f));
    g.drawLine(base.x, base.y, tip.x, tip.y, 2.0f);
    g.setColour(violet.withAlpha(0.7f));
    g.fillEllipse(tip.x - 1.5f, tip.y - 1.5f, 3.0f, 3.0f);
}
FeltRoomEditor::FeltRoomEditor(FeltRoomProcessor& p):AudioProcessorEditor(p),proc(p){
 piano=juce::ImageFileFormat::loadFrom(BinaryData::piano_jpg,BinaryData::piano_jpgSize);
 const char* ids[]={"PREDELAY","DECAY","SIZE","DAMPING","LOWCUT","HIGHCUT","MIX"};
 for(int i=0;i<7;++i){
  knobs[i]=std::make_unique<juce::Slider>();
  auto& k=*knobs[i];
  k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  k.setTextBoxStyle(juce::Slider::TextBoxBelow,false,125,25);
  k.setNumDecimalPlacesToDisplay(2);
  k.setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffead8c7));
  k.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
  k.setLookAndFeel(&look);
  // Display meaningful precision and units instead of JUCE's long floating-point values.
  k.textFromValueFunction = [i](double value) -> juce::String {
   auto trimmed = [](double v, int decimals) {
    auto text = juce::String(v, decimals);
    while (text.containsChar('.') && text.endsWith("0")) text = text.dropLastCharacters(1);
    if (text.endsWith(".")) text = text.dropLastCharacters(1);
    return text;
   };
   switch (i) {
    case 0: return trimmed(value, 1) + " ms";
    case 1: return trimmed(value, 2) + " s";
    case 2: case 3: case 6: return trimmed(value, 1) + "%";
    case 4: return juce::String((int) std::round(value)) + " Hz";
    case 5: return value >= 1000.0 ? trimmed(value / 1000.0, 1) + " kHz" : trimmed(value, 0) + " Hz";
    default: return trimmed(value, 2);
   }
  };
  k.valueFromTextFunction = [i](const juce::String& text) -> double {
   auto cleaned = text.trim().toLowerCase().replace("khz", "").replace("hz", "").replace("ms", "").replace("s", "").replace("%", "").trim();
   auto value = cleaned.getDoubleValue();
   if (i == 5 && text.toLowerCase().contains("khz")) value *= 1000.0;
   return value;
  };
  addAndMakeVisible(k);
  attachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.params,ids[i],k);
 }
 for(int i=0;i<(int)std::size(factory);++i)presets.addItem(factory[i].name,i+1);
 presets.setTextWhenNothingSelected("Choose a factory preset");
 presets.onChange=[this]{int id=presets.getSelectedId();if(id>0&&id<=(int)std::size(factory))proc.applyPreset(factory[id-1].p);};addAndMakeVisible(presets);
 mode.addItemList({"Room","Hall","Plate","Chamber","Vintage","Ambient"},1);addAndMakeVisible(mode);
 modeAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.params,"MODE",mode);
 for(auto* b:{&save,&load}){b->setColour(juce::TextButton::buttonColourId,juce::Colour(0xff30271f));b->setColour(juce::TextButton::textColourOffId,juce::Colour(0xffeedac6));addAndMakeVisible(*b);}
 save.onClick=[this]{savePreset();};load.onClick=[this]{loadPreset();};
 setSize(1200,800);
}
FeltRoomEditor::~FeltRoomEditor(){for(auto& k:knobs)k->setLookAndFeel(nullptr);}
void FeltRoomEditor::savePreset(){
 chooser=std::make_unique<juce::FileChooser>("Save FeltRoom preset",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("FeltRoom-Preset.xml"),"*.xml");
 chooser->launchAsync(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::warnAboutOverwriting,[this](const juce::FileChooser& fc){auto file=fc.getResult();if(file==juce::File{})return;if(auto xml=proc.params.copyState().createXml())xml->writeTo(file.withFileExtension("xml"));});
}
void FeltRoomEditor::loadPreset(){
 chooser=std::make_unique<juce::FileChooser>("Load FeltRoom preset",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),"*.xml");
 chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[this](const juce::FileChooser& fc){auto file=fc.getResult();if(file==juce::File{})return;auto xml=juce::XmlDocument::parse(file);if(xml&&xml->hasTagName(proc.params.state.getType()))proc.params.replaceState(juce::ValueTree::fromXml(*xml));});
}
void FeltRoomEditor::paint(juce::Graphics& g){auto area=getLocalBounds();g.fillAll(juce::Colour(0xff090a0b));
 auto imageArea=area.removeFromTop(625);if(piano.isValid())g.drawImage(piano,imageArea.toFloat(),juce::RectanglePlacement::fillDestination);
 juce::ColourGradient shade(juce::Colours::black.withAlpha(0.66f),0,0,juce::Colours::black.withAlpha(0.12f),0,570,false);g.setGradientFill(shade);g.fillRect(imageArea);
 g.setColour(juce::Colour(0xfff4e9de));g.setFont(juce::Font(juce::FontOptions(44)));g.drawFittedText("F E L T R O O M",0,42,getWidth(),65,juce::Justification::centred,1);
 g.setFont(juce::Font(juce::FontOptions(13)));g.setColour(juce::Colour(0xffd2bda8));g.drawFittedText("F E L T   R E V E R B",0,108,getWidth(),28,juce::Justification::centred,1);
 g.setFont(juce::Font(juce::FontOptions(19)));g.drawText("D.A Production",28,35,250,38,juce::Justification::left);
 g.setColour(juce::Colour(0xe6090909));g.fillRect(0,540,getWidth(),260);
 const char* labels[]={"PRE-DELAY","DECAY","SIZE","DAMPING","LOW CUT","HIGH CUT","MIX"};
 g.setFont(juce::Font(juce::FontOptions(12)));g.setColour(juce::Colour(0xffdfc8ae));for(int i=0;i<7;++i){auto cx=60+i*155+60;g.drawFittedText(labels[i],cx-73,680,146,22,juce::Justification::centred,1);}
 g.setColour(juce::Colour(0xff665548));g.drawHorizontalLine(733,40.f,1160.f);
 g.setColour(juce::Colour(0xffa79583));g.drawText("PRESETS",30,747,110,27,juce::Justification::left);
 g.drawText("CHARACTER",800,747,110,27,juce::Justification::left);
 g.setColour(juce::Colour(0xff685646));g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(8),12,1);
}
void FeltRoomEditor::resized(){for(int i=0;i<7;++i)knobs[i]->setBounds(60+i*155,565,120,110);presets.setBounds(140,744,365,32);save.setBounds(520,744,70,32);load.setBounds(600,744,70,32);mode.setBounds(900,744,265,32);}
