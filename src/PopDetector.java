
public class PopDetector extends UtteranceClassifier {
	private static final int    popMax    = 100;
	private static final int    popMin    = 50;
	private static final double popThresh = 1.0;
	private static final double silThresh = 0.05;
	private static final double silThreshBot = 1.5;
	
	public PopDetector(int bufSize) {
		super(bufSize);
	}

	@Override
	protected boolean match() {
		// must be right length
		if(utterLen > 3 || utterLen < 2) return false;
		// must have noise in the right band
		double popBand = bandAverage(popMin,popMax);
		if(popBand < popThresh) return false;
		// must have silence above and below
		double silBand  = bandAverage(5,popMin);
		if(silBand > silThreshBot) return false;
		double silBand2 = bandAverage(popMax,bufferSize-10);
		if(silBand2 > silThresh) return false;
		
		return true;
	}

}
