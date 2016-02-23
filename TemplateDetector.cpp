#include "TemplateDetector.h"

#include <iostream>
#include <numeric>
#include <algorithm>
#include <cmath>

using namespace std;

#include "templates.h"

static const int kDebugExtraHeight = 1;
static const float kDefaultLowPassWeight = 0.06;

TemplateDetector::TemplateDetector(float inputSampleRate) : Plugin(inputSampleRate) {
    m_blockSize = 512;
    m_sensitivity = 2.0;
    m_startBin = 3;
    m_maxShiftDown = 2;
    m_maxShiftUp = 3;
    m_hysterisisFactor = 1.5;
    m_lowPassWeight = kDefaultLowPassWeight;
    m_minFrames = 20;
    m_template = 0;
}

TemplateDetector::~TemplateDetector() {
}

string TemplateDetector::getIdentifier() const {
    return "templatedetector";
}

string TemplateDetector::getName() const {
    return "Template detector";
}

string TemplateDetector::getDescription() const {
    // Return something helpful here!
    return "Detects continuous sounds based on a template";
}

string TemplateDetector::getMaker() const {
    // Your name here
    return "Tristan Hume";
}

int TemplateDetector::getPluginVersion() const {
    // Increment this each time you release a version that behaves
    // differently from the previous one
    return 1;
}

string TemplateDetector::getCopyright() const {
    // This function is not ideally named.  It does not necessarily
    // need to say who made the plugin -- getMaker does that -- but it
    // should indicate the terms under which it is distributed.  For
    // example, "Copyright (year). All Rights Reserved", or "GPL"
    return "MIT";
}

TemplateDetector::InputDomain TemplateDetector::getInputDomain() const {
    return FrequencyDomain;
}

size_t TemplateDetector::getPreferredBlockSize() const {
    return m_blockSize;
}

size_t TemplateDetector::getPreferredStepSize() const {
    return m_blockSize/4;
}

size_t TemplateDetector::getMinChannelCount() const {
    return 1;
}

size_t TemplateDetector::getMaxChannelCount() const {
    return 1;
}

TemplateDetector::ParameterList TemplateDetector::getParameterDescriptors() const {
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
    d.identifier = "tempindex";
    d.name = "Template index";
    d.description = "Index of the template to be used in the templates list";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = kNumTemplates-1;
    d.defaultValue = 0;
    d.isQuantized = true;
    d.quantizeStep = 1.0;
    list.push_back(d);

    d.identifier = "sensitivity";
    d.name = "Trigger threshold";
    d.description = "The activation threshold below which a pop is registered";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 15;
    d.defaultValue = 2.0;
    d.isQuantized = false;
    list.push_back(d);

    d.identifier = "hysterisis";
    d.name = "Trigger hysterisis";
    d.description = "The factor of the trigger threshold required to untrigger";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 1;
    d.defaultValue = 1.5;
    d.isQuantized = false;
    list.push_back(d);
    d.identifier = "lowpass";
    d.name = "Low Pass Filter Weight";
    d.description = "The factor to give new samples in the weighted average";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 1;
    d.defaultValue = kDefaultLowPassWeight;
    d.isQuantized = false;
    list.push_back(d);
    d.identifier = "minframes";
    d.name = "Minimum time";
    d.description = "The minimum number of frames of matchiness to consider a match";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 100;
    d.defaultValue = 20;
    d.isQuantized = true;
    d.quantizeStep = 1.0;
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

float TemplateDetector::getParameter(string identifier) const {
    if (identifier == "sensitivity") {
        return m_sensitivity; // return the ACTUAL current value of your parameter here!
    } else if(identifier == "hysterisis") {
        return m_hysterisisFactor;
    } else if(identifier == "tempindex") {
        return m_template;
    } else if(identifier == "lowpass") {
        return m_lowPassWeight;
    } else if(identifier == "minframes") {
        return m_minFrames;
    } else if(identifier == "startbin") {
        return m_startBin;
    } else if(identifier == "maxshiftdown") {
        return m_maxShiftDown;
    } else if(identifier == "maxshiftup") {
        return m_maxShiftUp;
    }
    return 0;
}

void TemplateDetector::setParameter(string identifier, float value) {
    if (identifier == "sensitivity") {
        m_sensitivity = value;
    } else if(identifier == "hysterisis") {
        m_hysterisisFactor = value;
    } else if(identifier == "tempindex") {
        m_template = value;
    } else if(identifier == "lowpass") {
        m_lowPassWeight = value;
    } else if(identifier == "minframes") {
        m_minFrames = value;
    } else if(identifier == "startbin") {
        m_startBin = value;
    } else if(identifier == "maxshiftdown") {
        m_maxShiftDown = value;
    } else if(identifier == "maxshiftup") {
        m_maxShiftUp = value;
    }
}

TemplateDetector::ProgramList TemplateDetector::getPrograms() const {
    ProgramList list;

    // If you have no programs, return an empty list (or simply don't
    // implement this function or getCurrentProgram/selectProgram)

    return list;
}

string TemplateDetector::getCurrentProgram() const {
    return ""; // no programs
}

void TemplateDetector::selectProgram(string) {
}

bool TemplateDetector::initialise(size_t channels, size_t, size_t blockSize) {
    if (channels < getMinChannelCount() ||
    channels > getMaxChannelCount()) return false;

    // Real initialisation work goes here!
    m_blockSize = blockSize;
    lowPassBuffer.resize(m_blockSize / 2 + 1, 0.0);

    m_templateMax = bufferMax(kTemplates[m_template].data);

    buffer.clear();
    for(unsigned i = 0; i < kTemplates[m_template].size(); ++i) {
        buffer.push_back(0.0);
    }
    triggering = false;

    return true;
}

void TemplateDetector::reset() {
    // clear buffer
    for(auto &&x : buffer) {
        x = 0.0;
    }
}

TemplateDetector::FeatureSet TemplateDetector::getRemainingFeatures() {
    return FeatureSet();
}


TemplateDetector::OutputList TemplateDetector::getOutputDescriptors() const {
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
    d.binCount = kTemplates[m_template].height()+kDebugExtraHeight;
    // all attributes are already set to the right value
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

    d.identifier = "starts";
    d.name = "Trigger onsets";
    d.description = "Instants where a real-time recognizer could recognize a tss had occured.";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 0;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::VariableSampleRate;
    d.sampleRate = m_inputSampleRate;
    list.push_back(d);
    d.identifier = "stops";
    d.name = "Off instants";
    d.description = "Instants where a real-time recognizer could recognize a tss had stopped.";
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

TemplateDetector::FeatureSet TemplateDetector::process(const float *const *inputBuffers, Vamp::RealTime timestamp) {
    FeatureSet fs;

    if (m_blockSize == 0) {
        cerr << "ERROR: TemplateDetector::process: Not initialised" << endl;
        return fs;
    }

    size_t n = m_blockSize / 2 + 1;
    const float *fbuf = inputBuffers[0];

    for (size_t i = 0; i < n; ++i) {
        double real = fbuf[i * 2];
        double imag = fbuf[i * 2 + 1];
        double newVal = real * real + imag * imag;
        lowPassBuffer[i] = lowPassBuffer[i]*(1.0-m_lowPassWeight) + newVal*m_lowPassWeight;
    }

    Feature spectrum;
    spectrum.hasTimestamp = false;
    spectrum.values = lowPassBuffer;
    fs[0].push_back(spectrum);

    const TemplateInfo *tplate = &(kTemplates[m_template]);
    // update buffer forward one time step
    for(unsigned i = 0; i < tplate->primaryHeight; ++i) {
        buffer.pop_front();
        buffer.push_back(spectrum.values[i]);
    }
    // high frequencies aren't useful so we bin them all together
    buffer.pop_front();
    float highSum = accumulate(spectrum.values.begin()+tplate->primaryHeight,spectrum.values.end(),0.0);
    buffer.push_back(highSum);

    // compute max ignoring lower bins
    float maxVal = -1000.0;
    for(auto it = buffer.begin(); it != buffer.end(); ++it) {
        if((it-buffer.begin()) % tplate->height() < m_startBin) continue;
        if(*it >= maxVal) {
            maxVal = *it;
        }
    }

    // positive shift makes peak lower frequency, negative makes it higher
    float minDiff = 10000000.0;
    for(int i = -m_maxShiftUp; i < m_maxShiftDown; ++i) {
        float diff = templateDiff(maxVal, i);
        if(diff < minDiff) minDiff = diff;
    }
    Feature diffFeat;
    diffFeat.hasTimestamp = false;
    diffFeat.values.push_back(minDiff);
    fs[2].push_back(diffFeat);

    if(minDiff < m_sensitivity && !triggering) {
        Feature instant;
        instant.hasTimestamp = true;
        instant.timestamp = timestamp;
        fs[3].push_back(instant);
        triggering = true;
    } else if(minDiff >= m_sensitivity*m_hysterisisFactor && triggering) {
        Feature instant;
        instant.hasTimestamp = true;
        instant.timestamp = timestamp;
        fs[4].push_back(instant);
        triggering = false;
    }

    Feature debug;
    debug.hasTimestamp = false;
    debug.values.push_back(minDiff);
    for (size_t i = 0; i < tplate->height(); ++i) {
        float val = buffer[(tplate->size()-tplate->height())+i]/maxVal;
        debug.values.push_back(val);
    }
    fs[1].push_back(debug);

    return fs;
}

float TemplateDetector::templateAt(int i, int shift) {
    const TemplateInfo *tplate = &(kTemplates[m_template]);
    int bin = i % tplate->height();
    if(i % tplate->height() >= tplate->primaryHeight) {
        return tplate->data[i]/m_templateMax;
    }
    if(bin+shift < 0 || bin+shift >= (int)(tplate->primaryHeight)) {
        return 0.0;
    }
    return tplate->data[i+shift]/m_templateMax;
}

float TemplateDetector::diffCol(int templStart, int bufStart, float maxVal, int shift) {
    float diff = 0;
    for(unsigned i = m_startBin; i < kTemplates[m_template].height(); ++i) {
        float d = templateAt(templStart+i, shift) - buffer[bufStart+i]/maxVal;
        diff += abs(d);
    }
    return diff;
}

float TemplateDetector::templateDiff(float maxVal, int shift) {
    float diff = 0;
    for(unsigned i = 0; i < kTemplates[m_template].size(); i += kTemplates[m_template].height()) {
        diff += diffCol(i,i, maxVal,shift);
    }
    return diff;
}

float TemplateDetector::bufferMax(const float * const buf) const {
    float maxVal = -1000.0;
    const TemplateInfo *tplate = &(kTemplates[m_template]);
    for(unsigned i = 0; i < tplate->size(); ++i) {
        if(i % tplate->height() < m_startBin) continue;
        if(buf[i] >= maxVal) {
            maxVal = buf[i];
        }
    }
    return maxVal;
}
