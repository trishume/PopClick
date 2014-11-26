import java.awt.Toolkit;
import java.util.Arrays;

public abstract class UtteranceClassifier {
	public static final double COMPLETE_SILENCE_LEN = 2;
	
	protected enum State {
		WAITING, UTTERANCE, SILENCE
	}
	protected State curState = State.WAITING;
	protected float[] utterSum;
	protected int utterLen;
	protected int silenceLen;
	
	int bufferSize;
	boolean hasEvent;
	
	public UtteranceClassifier(int bufSize) {
		bufferSize = bufSize;
		utterSum = new float[bufferSize];
		reset();
	}
	
	public void silence() {
		if(curState == State.WAITING) return;
		
		silenceLen += 1;
		if(curState == State.UTTERANCE) {
			curState = State.SILENCE;
		} if(curState == State.SILENCE) {
			if(silenceLen >= COMPLETE_SILENCE_LEN) {
				complete();
			}
		}
	}
	
	public void noise(float[] amplitudes) {
		if(curState == State.WAITING) {
			curState = State.UTTERANCE;
		}
		
		for (int i = 0; i < bufferSize; ++i) {
		    utterSum[i] += amplitudes[i];
		}
		utterLen += 1;
	}
	
	public boolean popEvent() {
		if(hasEvent) {
			hasEvent = false;
			return true;
		}
		return false;
	}
	
	protected void reset() {
		utterLen = 0;
		silenceLen = 0;
		curState = State.WAITING;
		Arrays.fill(utterSum, 0);
	}
	
	protected void complete() {
		if(match()) {
			System.out.println("Matched");
			Toolkit.getDefaultToolkit().beep();
			hasEvent = true;
		}
		reset();
	}
	
	// Matching
	abstract protected boolean match();
	
	protected double bandAverage(int start, int end) {
		double sum = 0.0;
		for(int i = start; i < end; ++i) {
			sum += (utterSum[i]/utterLen); // average the utterance now
		}
		
		return sum / (end-start);
	}
}
