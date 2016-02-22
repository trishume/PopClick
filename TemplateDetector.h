#ifndef _TEMPLATEDETECTOR_H_
#define _TEMPLATEDETECTOR_H_

#include <vamp-sdk/Plugin.h>
#include <deque>
#include <vector>

using std::string;

class TemplateDetector : public Vamp::Plugin {
public:
    TemplateDetector(float inputSampleRate);
    virtual ~TemplateDetector();

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
    int m_minFrames;
    unsigned m_startBin;
    float m_hysterisisFactor;
    float m_lowPassWeight;
    int m_maxShiftDown;
    int m_maxShiftUp;
    int m_template;

    std::vector<float> lowPassBuffer;

    float m_templateMax;
    float bufferMax(const float * const buf) const;

    std::deque<float> buffer;
    int m_framesSinceTriggered;
    float templateAt(int i, int shift);
    float templateDiff(float maxVal, int shift);
    float diffCol(int templStart, int bufStart, float maxVal, int shift);
};



#endif
