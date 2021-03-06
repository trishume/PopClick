#include "PopDetector.h"

#include <iostream>
#include <numeric>
#include <algorithm>
#include <cmath>

using namespace std;

#include "poptemplate.h"

static const int kDebugHeight = kBufferHeight + 4;
static const int kDtwWidth = kBufferWidth+1;
static const int kDtwSize = kDtwWidth*kDtwWidth;

PopDetector::PopDetector(float inputSampleRate) : Plugin(inputSampleRate), dtwGrid(kDtwSize) {
    m_blockSize = 512;
    m_boundThreshDiv = 10;
    m_sensitivity = 8.5;
    m_silenceThresh = 0.2;
    m_startBin = 2;
    m_dtwWidth = 3;
    m_maxShiftDown = 4;
    m_maxShiftUp = 2;

    m_curState = PopDetector::SilenceBefore;
    m_framesInState = 0;
    m_framesSinceTriggered = 0;
}

PopDetector::~PopDetector() {
}

string PopDetector::getIdentifier() const {
    return "popdetector";
}

string PopDetector::getName() const {
    return "Lip pop detector";
}

string PopDetector::getDescription() const {
    // Return something helpful here!
    return "Detects lip popping noises";
}

string PopDetector::getMaker() const {
    // Your name here
    return "Tristan Hume";
}

int PopDetector::getPluginVersion() const {
    // Increment this each time you release a version that behaves
    // differently from the previous one
    return 1;
}

string PopDetector::getCopyright() const {
    // This function is not ideally named.  It does not necessarily
    // need to say who made the plugin -- getMaker does that -- but it
    // should indicate the terms under which it is distributed.  For
    // example, "Copyright (year). All Rights Reserved", or "GPL"
    return "MIT";
}

PopDetector::InputDomain PopDetector::getInputDomain() const {
    return FrequencyDomain;
}

size_t PopDetector::getPreferredBlockSize() const {
    return m_blockSize;
}

size_t PopDetector::getPreferredStepSize() const {
    return m_blockSize/4;
}

size_t PopDetector::getMinChannelCount() const {
    return 1;
}

size_t PopDetector::getMaxChannelCount() const {
    return 1;
}

PopDetector::ParameterList PopDetector::getParameterDescriptors() const {
    ParameterList list;

    // If the plugin has no adjustable parameters, return an empty
    // list here (and there's no need to provide implementations of
    // getParameter and setParameter in that case either).

    // Note that it is your responsibility to make sure the parameters
    // start off having their default values (e.g. in the constructor
    // above).  The host needs to know the default value so it can do
    // things like provide a "reset to default" function, but it will
    // not explicitly set your parameters to their defaults for you if
    // they have not changed in the mean time.

    ParameterDescriptor d;
    d.identifier = "sensitivity";
    d.name = "Trigger threshold";
    d.description = "The activation threshold below which a pop is registered";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 15;
    d.defaultValue = 8.5;
    d.isQuantized = false;
    list.push_back(d);
    d.identifier = "bounddiv";
    d.name = "Boundary Threshold divisor";
    d.description = "Divisor of max for boundary threshold";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 100;
    d.defaultValue = 10;
    d.isQuantized = true;
    d.quantizeStep = 1.0;
    list.push_back(d);
    d.identifier = "silence";
    d.name = "Silence threshold";
    d.description = "Threshold of the average amplitude for silence";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 10;
    d.defaultValue = 0.2;
    d.isQuantized = false;
    list.push_back(d);
    d.identifier = "startbin";
    d.name = "High pass filter";
    d.description = "Ignore bins below this bin number.";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 5;
    d.defaultValue = 2;
    d.isQuantized = true;
    d.quantizeStep = 1.0;
    list.push_back(d);
    d.identifier = "dtwwidth";
    d.name = "DTW Boundary Width";
    d.description = "Band away from diagonal that the DTW can warp in";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 30;
    d.defaultValue = 3;
    d.isQuantized = true;
    d.quantizeStep = 1.0;
    list.push_back(d);
    d.identifier = "maxshiftdown";
    d.name = "Maximum Template Shift Down";
    d.description = "Largest number of bins in down direction template can be shifted to match.";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 10;
    d.defaultValue = 4;
    d.isQuantized = true;
    d.quantizeStep = 1.0;
    list.push_back(d);
    d.identifier = "maxshiftup";
    d.name = "Maximum Template Shift Up";
    d.description = "Largest number of bins in down direction template can be shifted to match.";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 10;
    d.defaultValue = 2;
    d.isQuantized = true;
    d.quantizeStep = 1.0;
    list.push_back(d);

    return list;
}

float PopDetector::getParameter(string identifier) const {
    if (identifier == "sensitivity") {
        return m_sensitivity; // return the ACTUAL current value of your parameter here!
    } else if(identifier == "bounddiv") {
        return m_boundThreshDiv;
    } else if(identifier == "silence") {
        return m_silenceThresh;
    } else if(identifier == "startbin") {
        return m_startBin;
    } else if(identifier == "dtwwidth") {
        return m_dtwWidth;
    } else if(identifier == "maxshiftdown") {
        return m_maxShiftDown;
    } else if(identifier == "maxshiftup") {
        return m_maxShiftUp;
    }
    return 0;
}

void PopDetector::setParameter(string identifier, float value) {
    if (identifier == "sensitivity") {
        m_sensitivity = value;
    } else if(identifier == "bounddiv") {
        m_boundThreshDiv = value;
    } else if(identifier == "silence") {
        m_silenceThresh = value;
    } else if(identifier == "startbin") {
        m_startBin = value;
    } else if(identifier == "dtwwidth") {
        m_dtwWidth = value;
    } else if(identifier == "maxshiftdown") {
        m_maxShiftDown = value;
    } else if(identifier == "maxshiftup") {
        m_maxShiftUp = value;
    }
}

PopDetector::ProgramList PopDetector::getPrograms() const {
    ProgramList list;

    // If you have no programs, return an empty list (or simply don't
    // implement this function or getCurrentProgram/selectProgram)

    return list;
}

string PopDetector::getCurrentProgram() const {
    return ""; // no programs
}

void PopDetector::selectProgram(string) {
}

bool PopDetector::initialise(size_t channels, size_t, size_t blockSize) {
    if (channels < getMinChannelCount() ||
    channels > getMaxChannelCount()) return false;

    // Real initialisation work goes here!
    m_blockSize = blockSize;

    buffer.clear();
    for(unsigned i = 0; i < kBufferSize; ++i) {
        buffer.push_back(0.0);
    }

    return true;
}

void PopDetector::reset() {
    // Clear buffers, reset stored values, etc
    m_framesInState = 0;
    m_curState = SilenceBefore;

    // clear buffer
    for(auto &&x : buffer) {
        x = 0.0;
    }
}

PopDetector::FeatureSet PopDetector::getRemainingFeatures() {
    return FeatureSet();
}


PopDetector::OutputList PopDetector::getOutputDescriptors() const {
    OutputList list;

    OutputDescriptor d;
    d.identifier = "powerspectrum";
    d.name = "Power Spectrum";
    d.description = "Power values of the frequency spectrum bins calculated from the input signal";
    d.unit = "";
    d.hasFixedBinCount = true;
    if (m_blockSize == 0) {
        // Just so as not to return "1".  This is the bin count that
        // would result from a block size of 1024, which is a likely
        // default -- but the host should always set the block size
        // before querying the bin count for certain.
        d.binCount = 257;
    } else {
        d.binCount = m_blockSize / 2 + 1;
    }
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::OneSamplePerStep;
    list.push_back(d);

    d.identifier = "debugspectrum";
    d.name = "Debug Spectrum";
    d.description = "Spectrum containing special debugging info";
    d.binCount = kDebugHeight;
    // all attributes are already set to the right value
    list.push_back(d);

    d.identifier = "average";
    d.name = "Average Power";
    d.description = "Average of the power spectrum for a column";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 1;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::OneSamplePerStep;
    list.push_back(d);

    d.identifier = "max";
    d.name = "Max Power";
    d.description = "Max of the power spectrum for a column";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 1;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::OneSamplePerStep;
    list.push_back(d);

    d.identifier = "diff";
    d.name = "Template difference";
    d.description = "The extent to which the template pop matches the buffer.";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 1;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::OneSamplePerStep;
    list.push_back(d);

    d.identifier = "pops";
    d.name = "Simple Pop instants";
    d.description = "Instants where a real-time recognizer could recognize a pop had occured.";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 0;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::VariableSampleRate;
    d.sampleRate = m_inputSampleRate;
    list.push_back(d);

    d.identifier = "dtwdiff";
    d.name = "DTW Template difference";
    d.description = "Template difference with dynamic time warping";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 1;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::OneSamplePerStep;
    list.push_back(d);

    d.identifier = "temppops";
    d.name = "Template Pop instants";
    d.description = "Instants where a real-time template-based recognizer could recognize a pop had occured.";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 0;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::VariableSampleRate;
    d.sampleRate = m_inputSampleRate;
    list.push_back(d);

    return list;
}

PopDetector::FeatureSet PopDetector::process(const float *const *inputBuffers, Vamp::RealTime timestamp) {
    FeatureSet fs;

    if (m_blockSize == 0) {
        cerr << "ERROR: PopDetector::process: Not initialised" << endl;
        return fs;
    }

    size_t n = m_blockSize / 2 + 1;
    const float *fbuf = inputBuffers[0];

    Feature spectrum;
    spectrum.hasTimestamp = false;
    spectrum.values.reserve(n); // optional
    for (size_t i = 0; i < n; ++i) {
        double real = fbuf[i * 2];
        double imag = fbuf[i * 2 + 1];
        spectrum.values.push_back(real * real + imag * imag);
    }
    fs[0].push_back(spectrum);

    float avg = 0;
    for (size_t i = m_startBin; i < n; ++i) {
        avg += spectrum.values[i];
    }
    avg = avg / n;

    Feature avgFeat;
    avgFeat.hasTimestamp = false;
    avgFeat.values.push_back(avg);
    fs[2].push_back(avgFeat);

    auto it = max_element(spectrum.values.begin()+m_startBin,spectrum.values.end());
    float maxAmplitude = *it;

    Feature maxAmplitudeFeat;
    maxAmplitudeFeat.hasTimestamp = false;
    maxAmplitudeFeat.values.push_back(maxAmplitude);
    fs[3].push_back(maxAmplitudeFeat);

    size_t lower = 0;
    for (size_t i = m_startBin; i < n; ++i) {
        if(spectrum.values[i] > maxAmplitude/m_boundThreshDiv) {
            lower = i;
            break;
        }
    }

    size_t upper = n;
    for (int i = n-1; i >= m_startBin; --i) {
        if(spectrum.values[i] > maxAmplitude/m_boundThreshDiv) {
            upper = i;
            break;
        }
    }

    // update buffer forward one time step
    for(unsigned i = 0; i < kBufferPrimaryHeight; ++i) {
        buffer.pop_front();
        buffer.push_back(spectrum.values[i]);
    }
    // high frequencies aren't useful so we bin them all together
    buffer.pop_front();
    float highSum = accumulate(spectrum.values.begin()+kBufferPrimaryHeight,spectrum.values.end(),0.0);
    buffer.push_back(highSum);

    auto maxIt = max_element(buffer.begin(), buffer.end());
    int maxBin = (maxIt - buffer.begin()) % kBufferHeight;
    // positive shift makes peak lower frequency, negative makes it higher
    int shift = min(m_maxShiftDown, max(kPopTemplateMaxBin - maxBin,-m_maxShiftUp));

    float minDiff = 10000000.0;
    for(int i = -m_maxShiftUp; i < m_maxShiftDown; ++i) {
        float diff = templateDiff(*maxIt, i);
        if(diff < minDiff) minDiff = diff;
    }
    m_framesSinceTriggered += 1;
    if(minDiff < m_sensitivity && m_framesSinceTriggered > 15) {
        Feature instant;
        instant.hasTimestamp = true;
        instant.timestamp = timestamp;
        fs[7].push_back(instant);
        m_framesSinceTriggered = 0;
    }

    Feature diffFeat;
    diffFeat.hasTimestamp = false;
    diffFeat.values.push_back(minDiff);
    fs[4].push_back(diffFeat);

    float dtwDiff = templateDiffDtw(m_dtwWidth, *maxIt, shift);
    Feature dtwDiffFeat;
    dtwDiffFeat.hasTimestamp = false;
    dtwDiffFeat.values.push_back(dtwDiff);
    fs[6].push_back(dtwDiffFeat);

    bool recognized = stateMachine(avg, lower, upper);
    if(recognized) {
        Feature instant;
        instant.hasTimestamp = true;
        instant.timestamp = timestamp;
        fs[5].push_back(instant);
    }

    Feature debug;
    debug.hasTimestamp = false;
    debug.values.reserve(kDebugHeight); // optional
    debug.values.push_back(minDiff);
    debug.values.push_back(dtwDiff);
    debug.values.push_back(m_curState);
    debug.values.push_back(m_framesInState);
    for (size_t i = 0; i < kBufferHeight; ++i) {
        float val = buffer[(kBufferSize-kBufferHeight)+i];
        debug.values.push_back(val);
    }
    fs[1].push_back(debug);

    return fs;
}

bool PopDetector::stateMachine(float avg, int lower, int upper) {
    int popTop = (m_curState == SilenceBefore) ? 22 : 19;
    int popHeight = 10;
    if(m_curState == SilenceBefore) {
        popHeight = 20;
    } else if(m_framesInState > 2) {
        popHeight = 6;
    }
    if(avg < m_silenceThresh) { // silence frame
        if(m_curState == SilenceAfter && m_framesInState >= 30) {
            // don't use normal transition because silence after should also add to silence before count
            m_curState = SilenceBefore;
            m_framesInState += 1;
            return true; // successful lip pop detection
        } else if(m_curState == Pop && m_framesInState >= 3) {
            transition(SilenceAfter);
        } else if(m_curState != SilenceBefore && m_curState != SilenceAfter) {
            transition(SilenceBefore);
        }
    } else if(upper < popTop && upper - lower < popHeight) { // pop frame
        if(m_curState == SilenceBefore && m_framesInState >= 40) {
            transition(Pop);
        // } else if(m_curState == Pop && m_framesInState > 8) { // too long
        //     transition(BadSound);
        } else if(m_curState != Pop) {
            transition(BadSound);
        }
    } else { // misc. bad sound
        transition(BadSound);
    }

    m_framesInState += 1;
    return false;
}

float PopDetector::templateAt(int i, int shift) {
    int bin = i % kBufferHeight;
    if(i % kBufferHeight >= kBufferPrimaryHeight) {
        return kPopTemplate[i]/kPopTemplateMax;
    }
    if(bin+shift < 0 || bin+shift >= kBufferPrimaryHeight) {
        return 0.0;
    }
    return kPopTemplate[i+shift]/kPopTemplateMax;
}

float PopDetector::diffCol(int templStart, int bufStart, float maxVal, int shift) {
    float diff = 0;
    for(unsigned i = m_startBin; i < kBufferHeight; ++i) {
        float d = templateAt(templStart+i, shift) - buffer[bufStart+i]/maxVal;
        diff += abs(d);
    }
    return diff;
}

float PopDetector::templateDiff(float maxVal, int shift) {
    float diff = 0;
    for(unsigned i = 0; i < kBufferSize; i += kBufferHeight) {
        diff += diffCol(i,i, maxVal,shift);
    }
    return diff;
}

// Dynamic Time Warping
float PopDetector::templateDiffDtw(int w, float maxVal, int shift) {
    for(auto &&x : dtwGrid) {
        x = 100000000; // basically infinity
    }
    dtwGrid[0] = 0.0;

    for(int r = 1; r < kDtwWidth; ++r) {
        for(int c = max(1,r-w); c <= min(kDtwWidth-1, r+w); ++c) {
            float cost = diffCol((r-1)*kBufferHeight,(c-1)*kBufferHeight, maxVal,shift);
            float prevMin = min(dtwGrid[(r-1)*kDtwWidth+(c-1)],
                            min(dtwGrid[(r-1)*kDtwWidth+c],
                                dtwGrid[r*kDtwWidth+(c-1)]));
            dtwGrid[r*kDtwWidth+c] = cost + prevMin;
        }
    }

    return dtwGrid[kDtwSize-1];
}

