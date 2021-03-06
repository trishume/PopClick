#ifndef _POPCLICK_H_
#define _POPCLICK_H_

#include <vamp-sdk/Plugin.h>
#include <deque>
#include <vector>

using std::string;

class PopDetector : public Vamp::Plugin {
public:
    PopDetector(float inputSampleRate);
    virtual ~PopDetector();

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
    int m_boundThreshDiv;
    float m_sensitivity;
    float m_silenceThresh;
    int m_startBin;
    int m_dtwWidth;
    int m_maxShiftDown;
    int m_maxShiftUp;

    enum State {SilenceBefore, Pop, SilenceAfter, BadSound};
    State m_curState;
    int m_framesInState;
    void transition(State s) { m_curState = s; m_framesInState = 0; }

    bool stateMachine(float avg, int lower, int upper);

    std::deque<float> buffer;
    std::vector<float> dtwGrid;
    int m_framesSinceTriggered;
    float templateAt(int i, int shift);
    float templateDiff(float maxVal, int shift);
    float templateDiffDtw(int w, float maxVal, int shift);
    float diffCol(int templStart, int bufStart, float maxVal, int shift);
};



#endif
