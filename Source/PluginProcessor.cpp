#include "PluginProcessor.h"
#include "PluginEditor.h"
static juce::AudioProcessorValueTreeState::ParameterLayout makeParams(){
 juce::AudioProcessorValueTreeState::ParameterLayout p;
 auto add=[&](const char* id,float a,float b,float d){p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},id,juce::NormalisableRange<float>(a,b),d));};
 add("PREDELAY",0,200,20);add("DECAY",0.2f,12,3.5f);add("SIZE",0,100,70);add("DAMPING",0,100,45);
 add("LOWCUT",20,1000,100);add("HIGHCUT",1000,20000,12000);add("MIX",0,100,35);
 p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"MODE",1},"Reverb character",juce::StringArray{"Room","Hall","Plate","Chamber","Vintage","Ambient"},0));
 return p;
}
FeltRoomProcessor::FeltRoomProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),params(*this,nullptr,"PARAMETERS",makeParams()){}
void FeltRoomProcessor::prepareToPlay(double s,int maxBlock){
 sr=s;reverb.setSampleRate(s);reverb.reset();
 wet.setSize(2,juce::jmax(1,maxBlock),false,true,true);
 delay.setSize(2,juce::jmax(1,(int)std::ceil(s*0.21)+4));delay.clear();delayWrite=0;
 for(auto* f:{&lowCut[0],&lowCut[1],&highCut[0],&highCut[1]})f->reset();
 auto v=[this](const char* id){return params.getRawParameterValue(id)->load();};
 const float initLo = juce::jlimit(20.f,1000.f,v("LOWCUT"));
 const float initHi = juce::jlimit(1000.f,(float)(s*0.47),v("HIGHCUT"));
 const float initSize = v("SIZE")/100.f, initDamp = v("DAMPING")/100.f, initDecay = v("DECAY");
 const int initMode = juce::jlimit(0,5,(int)v("MODE"));
 const float sizeOffsets[]={0.f,0.13f,-0.09f,0.035f,-0.13f,0.20f};
 const float dampOffsets[]={0.f,-0.12f,0.07f,0.10f,0.23f,-0.22f};
 const float initRoomSize = juce::jlimit(0.05f,0.98f,0.17f+0.043f*initDecay+0.30f*initSize+sizeOffsets[initMode]);
 const float initDampVal = juce::jlimit(0.f,0.98f,initDamp+dampOffsets[initMode]);
 // 25ms for mix, 50ms for filters, 40ms for reverb params - smooth enough to avoid zipper but responsive
 mixSmoothed.reset(s,0.025); mixSmoothed.setCurrentAndTargetValue(v("MIX")/100.f);
 loCutSmoothed.reset(s,0.05); loCutSmoothed.setCurrentAndTargetValue(initLo);
 hiCutSmoothed.reset(s,0.05); hiCutSmoothed.setCurrentAndTargetValue(initHi);
 roomSizeSmoothed.reset(s,0.04); roomSizeSmoothed.setCurrentAndTargetValue(initRoomSize);
 dampSmoothed.reset(s,0.04); dampSmoothed.setCurrentAndTargetValue(initDampVal);
 lastLoFreq = initLo; lastHiFreq = initHi;
 // init filters to avoid first block click
 auto hpInit = juce::dsp::IIR::Coefficients<float>::makeHighPass(s,initLo);
 auto lpInit = juce::dsp::IIR::Coefficients<float>::makeLowPass(s,initHi);
 for(int c=0;c<2;++c){ *lowCut[c].coefficients = *hpInit; *highCut[c].coefficients = *lpInit; }
}
void FeltRoomProcessor::releaseResources(){
 // Keep buffers but clear delay to avoid stale tails on sample rate change
 delay.clear();
 wet.clear();
}
void FeltRoomProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){
 juce::ScopedNoDenormals nd;
 const int n=b.getNumSamples(), ch=juce::jmin(2,b.getNumChannels());if(ch==0||n==0)return;
 // FIX: dynamic resize instead of returning dry on host block-size change
 if(wet.getNumSamples() < n || wet.getNumChannels() < ch){
  wet.setSize(juce::jmax(2,ch), n, false, true, true);
 }
 // delay buffer already sized for 210ms max; ensure it exists
 if(delay.getNumSamples() == 0){
  delay.setSize(2, juce::jmax(1,(int)std::ceil(sr*0.21)+4)); delay.clear(); delayWrite=0;
 }
 auto v=[this](const char* id){return params.getRawParameterValue(id)->load();};
 const int mode=juce::jlimit(0,5,(int)v("MODE"));
 const float size=v("SIZE")/100.f, damping=v("DAMPING")/100.f,decay=v("DECAY");
 // Each character has a different damping, width and decay response. These are algorithmic variants, not convolution IRs.
 const float sizeOffsets[]={0.f,0.13f,-0.09f,0.035f,-0.13f,0.20f};
 const float dampOffsets[]={0.f,-0.12f,0.07f,0.10f,0.23f,-0.22f};
 const float widths[]={0.82f,1.f,1.f,0.72f,0.55f,1.f};
 // Smooth all continuous params to avoid zipper on automation
 const float targetRoomSize = juce::jlimit(0.05f,0.98f,0.17f+0.043f*decay+0.30f*size+sizeOffsets[mode]);
 const float targetDamp = juce::jlimit(0.f,0.98f,damping+dampOffsets[mode]);
 const float targetLo = juce::jlimit(20.f,1000.f,v("LOWCUT"));
 const float targetHi = juce::jlimit(1000.f,(float)(sr*0.47),v("HIGHCUT"));
 roomSizeSmoothed.setTargetValue(targetRoomSize);
 dampSmoothed.setTargetValue(targetDamp);
 loCutSmoothed.setTargetValue(targetLo);
 hiCutSmoothed.setTargetValue(targetHi);
 // Advance smoothing by n samples and take current smoothed value for this block
 if (n > 1){
  roomSizeSmoothed.skip(n-1);
  dampSmoothed.skip(n-1);
  loCutSmoothed.skip(n-1);
  hiCutSmoothed.skip(n-1);
 }
 float curRoomSize = roomSizeSmoothed.getNextValue();
 float curDamp = dampSmoothed.getNextValue();
 float curLo = loCutSmoothed.getNextValue();
 float curHi = hiCutSmoothed.getNextValue();

 juce::Reverb::Parameters rp;
 rp.roomSize = curRoomSize;
 rp.damping = curDamp;
 rp.width = widths[mode];
 rp.wetLevel=0.7f;rp.dryLevel=0.f;rp.freezeMode=0.f;
 reverb.setParameters(rp);

 // FIX: only update IIR coefficients when freq changed enough to avoid needless alloc + clicks
 // Threshold 0.5 Hz for lowCut, 1 Hz for highCut, plus smoothing already applied
 bool loChanged = std::abs(curLo - lastLoFreq) > 0.5f;
 bool hiChanged = std::abs(curHi - lastHiFreq) > 1.0f;
 if (loChanged || hiChanged){
  // Create new coefficients from smoothed frequencies
  auto hp = juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, curLo);
  auto lp = juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, curHi);
  for(int c=0;c<ch;++c){
   if (loChanged) *lowCut[c].coefficients = *hp;
   if (hiChanged) *highCut[c].coefficients = *lp;
  }
  if (loChanged) lastLoFreq = curLo;
  if (hiChanged) lastHiFreq = curHi;
 } else {
  // Even if not changed, ensure coefficients are still valid for first blocks
  // (already set in prepareToPlay)
 }

 for(int c=0;c<ch;++c) wet.copyFrom(c,0,b,c,0,n);

 const int length=delay.getNumSamples(),offset=juce::jlimit(0,length-2,(int)std::round(v("PREDELAY")*sr/1000.0));
 for(int i=0;i<n;++i){
  const int read=(delayWrite-offset+length)%length;
  for(int c=0;c<ch;++c){float* d=delay.getWritePointer(c);float* w=wet.getWritePointer(c);const float input=w[i];w[i]=offset==0?input:d[read];d[delayWrite]=input;}
  if(++delayWrite>=length)delayWrite=0;
 }
 if(ch==2)reverb.processStereo(wet.getWritePointer(0),wet.getWritePointer(1),n);
 else reverb.processMono(wet.getWritePointer(0),n);
 for(int c=0;c<ch;++c){auto* w=wet.getWritePointer(c);for(int i=0;i<n;++i)w[i]=highCut[c].processSample(lowCut[c].processSample(w[i]));}
 mixSmoothed.setTargetValue(v("MIX")/100.f);
 for(int i=0;i<n;++i){float m=mixSmoothed.getNextValue();for(int c=0;c<ch;++c){auto* dry=b.getWritePointer(c);dry[i]=dry[i]*(1.f-m)+wet.getSample(c,i)*m;}}
}
void FeltRoomProcessor::applyPreset(const std::array<float,8>& vals){
 const char* ids[]={"PREDELAY","DECAY","SIZE","DAMPING","LOWCUT","HIGHCUT","MIX","MODE"};
 for(int i=0;i<8;++i){
  if(auto* p=params.getParameter(ids[i])){
   // Use proper gesture for host automation undo
   p->beginChangeGesture();
   p->setValueNotifyingHost(p->convertTo0to1(vals[(size_t)i]));
   p->endChangeGesture();
  }
 }
}
void FeltRoomProcessor::getStateInformation(juce::MemoryBlock& m){if(auto x=params.copyState().createXml())copyXmlToBinary(*x,m);}
void FeltRoomProcessor::setStateInformation(const void* d,int n){auto x=getXmlFromBinary(d,n);if(x&&x->hasTagName(params.state.getType()))params.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new FeltRoomProcessor();}
juce::AudioProcessorEditor* FeltRoomProcessor::createEditor(){return new FeltRoomEditor(*this);}
