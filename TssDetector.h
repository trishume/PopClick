#ifndef _TSSDETECTOR_H_
#define _TSSDETECTOR_H_

#include <vamp-sdk/Plugin.h>
#include <vector>

using std::string;

class TssDetector : public Vamp::Plugin {
public:
    TssDetector(float inputSampleRate);
    virtual ~TssDetector();

    string getIdentifier() const;
    string getName() const;
    string getDescription() const;
    string getMaker() const;
    int getPluginVersion() const;
    string getCopyright() const;

    InputDomain getInputDomain() const;
    size_t getPreferredBlockSize() const;
    size_t getPreferredStepSize() const;
    size_t getMinChannelCount() const;
    size_t getMaxChannelCount() const;

    ParameterList getParameterDescriptors() const;
    float getParameter(string identifier) const;
    void setParameter(string identifier, float value);

    ProgramList getPrograms() const;
    string getCurrentProgram() const;
    void selectProgram(string name);

    OutputList getOutputDescriptors() const;

    bool initialise(size_t channels, size_t stepSize, size_t blockSize);
    void reset();

    FeatureSet process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp);

    FeatureSet getRemainingFeatures();

protected:
    // plugin-specific data and methods go here
    int m_blockSize;
    float m_sensitivity;
    int m_maxShiftDown;
    int m_maxShiftUp;

    float avgBand(std::vector<float> &frame, size_t low, size_t hi);
};



#endif
