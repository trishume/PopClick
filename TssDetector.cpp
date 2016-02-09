#include "TssDetector.h"

#include <iostream>
#include <numeric>
#include <algorithm>
#include <cmath>

using namespace std;

static const int kDebugHeight = 4;
static const int kPreferredBlockSize = 512;
static const int kSpectrumSize = kPreferredBlockSize/2+1;

static const size_t kMainBandLow = 40;
static const size_t kMainBandHi = 60;
static const size_t kOptionalBandHi = 160;

static const size_t kLowerBandLow = 0;
static const size_t kLowerBandHi = kMainBandLow;
static const size_t kOptionalBandLo = kMainBandHi;
static const size_t kUpperBandLo = kOptionalBandHi;
static const size_t kUpperBandHi = kSpectrumSize;

TssDetector::TssDetector(float inputSampleRate) : Plugin(inputSampleRate) {
    m_blockSize = kPreferredBlockSize;
    m_sensitivity = 8.5;
    m_maxShiftDown = 4;
    m_maxShiftUp = 2;
}

TssDetector::~TssDetector() {
}

string TssDetector::getIdentifier() const {
    return "tssdetector";
}

string TssDetector::getName() const {
    return "Tss Sound detector";
}

string TssDetector::getDescription() const {
    // Return something helpful here!
    return "Detects making a Tssss sound with your tongue and teeth";
}

string TssDetector::getMaker() const {
    // Your name here
    return "Tristan Hume";
}

int TssDetector::getPluginVersion() const {
    // Increment this each time you release a version that behaves
    // differently from the previous one
    return 1;
}

string TssDetector::getCopyright() const {
    // This function is not ideally named.  It does not necessarily
    // need to say who made the plugin -- getMaker does that -- but it
    // should indicate the terms under which it is distributed.  For
    // example, "Copyright (year). All Rights Reserved", or "GPL"
    return "MIT";
}

TssDetector::InputDomain TssDetector::getInputDomain() const {
    return FrequencyDomain;
}

size_t TssDetector::getPreferredBlockSize() const {
    return m_blockSize;
}

size_t TssDetector::getPreferredStepSize() const {
    return m_blockSize/4;
}

size_t TssDetector::getMinChannelCount() const {
    return 1;
}

size_t TssDetector::getMaxChannelCount() const {
    return 1;
}

TssDetector::ParameterList TssDetector::getParameterDescriptors() const {
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
    d.description = "The activation threshold at which a tss is registered";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 15;
    d.defaultValue = 8.5;
    d.isQuantized = false;
    list.push_back(d);
    d.identifier = "maxshiftdown";
    d.name = "Maximum Shift Down";
    d.description = "Largest number of bins in down direction can be shifted to match.";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 10;
    d.defaultValue = 4;
    d.isQuantized = true;
    d.quantizeStep = 1.0;
    list.push_back(d);
    d.identifier = "maxshiftup";
    d.name = "Maximum Shift Up";
    d.description = "Largest number of bins in down direction can be shifted to match.";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 10;
    d.defaultValue = 2;
    d.isQuantized = true;
    d.quantizeStep = 1.0;
    list.push_back(d);

    return list;
}

float TssDetector::getParameter(string identifier) const {
    if (identifier == "sensitivity") {
        return m_sensitivity; // return the ACTUAL current value of your parameter here!
    } else if(identifier == "maxshiftdown") {
        return m_maxShiftDown;
    } else if(identifier == "maxshiftup") {
        return m_maxShiftUp;
    }
    return 0;
}

void TssDetector::setParameter(string identifier, float value) {
    if (identifier == "sensitivity") {
        m_sensitivity = value;
    } else if(identifier == "maxshiftdown") {
        m_maxShiftDown = value;
    } else if(identifier == "maxshiftup") {
        m_maxShiftUp = value;
    }
}

TssDetector::ProgramList TssDetector::getPrograms() const {
    ProgramList list;

    // If you have no programs, return an empty list (or simply don't
    // implement this function or getCurrentProgram/selectProgram)

    return list;
}

string TssDetector::getCurrentProgram() const {
    return ""; // no programs
}

void TssDetector::selectProgram(string) {
}

bool TssDetector::initialise(size_t channels, size_t, size_t blockSize) {
    if (channels < getMinChannelCount() ||
    channels > getMaxChannelCount()) return false;

    // Real initialisation work goes here!
    m_blockSize = blockSize;
    return true;
}

void TssDetector::reset() {

}

TssDetector::FeatureSet TssDetector::getRemainingFeatures() {
    return FeatureSet();
}


TssDetector::OutputList TssDetector::getOutputDescriptors() const {
    OutputList list;

    OutputDescriptor d;
    d.identifier = "powerspectrum";
    d.name = "Power Spectrum";
    d.description = "Power values of the frequency spectrum bins calculated from the input signal";
    d.unit = "";
    d.hasFixedBinCount = true;
    if (m_blockSize == 0) {
        // Just so as not to return "1".  This is the bin count that
        // would result from a block size of 512, which is a likely
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

    d.identifier = "matchiness";
    d.name = "Matchiness";
    d.description = "The output of the matcher, how confident this is a match.";
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

    return list;
}

TssDetector::FeatureSet TssDetector::process(const float *const *inputBuffers, Vamp::RealTime timestamp) {
    FeatureSet fs;

    if (m_blockSize == 0) {
        cerr << "ERROR: TssDetector::process: Not initialised" << endl;
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

    float lowerBand = avgBand(spectrum.values, kLowerBandLow, kLowerBandHi);
    float mainBand = avgBand(spectrum.values, kMainBandLow, kMainBandHi);
    float optionalBand = avgBand(spectrum.values, kOptionalBandLo, kOptionalBandHi);
    float upperBand = avgBand(spectrum.values, kUpperBandLo, kUpperBandHi);

    float matchiness = mainBand / ((lowerBand+upperBand)/2.0);

    Feature matchFeat;
    matchFeat.hasTimestamp = false;
    matchFeat.values.push_back(matchiness);
    fs[2].push_back(matchFeat);

    Feature debug;
    debug.hasTimestamp = false;
    debug.values.reserve(kDebugHeight); // optional
    debug.values.push_back(lowerBand);
    debug.values.push_back(mainBand);
    debug.values.push_back(optionalBand);
    debug.values.push_back(upperBand);
    fs[1].push_back(debug);

    return fs;
}

float TssDetector::avgBand(std::vector<float> &frame, size_t low, size_t hi) {
    float sum = 0;
    for (size_t i = low; i < hi; ++i) {
        sum += frame[i];
    }
    return sum / (hi - low);
}

