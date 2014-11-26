/*
*      _______                       _____   _____ _____  
*     |__   __|                     |  __ \ / ____|  __ \ 
*        | | __ _ _ __ ___  ___  ___| |  | | (___ | |__) |
*        | |/ _` | '__/ __|/ _ \/ __| |  | |\___ \|  ___/ 
*        | | (_| | |  \__ \ (_) \__ \ |__| |____) | |     
*        |_|\__,_|_|  |___/\___/|___/_____/|_____/|_|     
*                                                         
* -------------------------------------------------------------
*
* TarsosDSP is developed by Joren Six at IPEM, University Ghent
*  
* -------------------------------------------------------------
*
*  Info: http://0110.be/tag/TarsosDSP
*  Github: https://github.com/JorenSix/TarsosDSP
*  Releases: http://0110.be/releases/TarsosDSP/
*  
*  TarsosDSP includes modified source code by various authors,
*  for credits and info, see README.
* 
*/




import java.awt.BorderLayout;
import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.util.Vector;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.Mixer;
import javax.sound.sampled.Mixer.Info;
import javax.sound.sampled.TargetDataLine;
import javax.sound.sampled.UnsupportedAudioFileException;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.border.TitledBorder;

import be.tarsos.dsp.AudioDispatcher;
import be.tarsos.dsp.AudioEvent;
import be.tarsos.dsp.AudioProcessor;
import be.tarsos.dsp.SilenceDetector;
import be.tarsos.dsp.io.jvm.AudioDispatcherFactory;
import be.tarsos.dsp.io.jvm.AudioPlayer;
import be.tarsos.dsp.io.jvm.JVMAudioInputStream;
import be.tarsos.dsp.util.fft.FFT;

public class Spectrogram extends JFrame {
	
	/**
	 * 
	 */
	private static final long serialVersionUID = 1383896180290138076L;
	private final SpectrogramPanel panel;
	private AudioDispatcher dispatcher;
	
	private static final int speedFactor = 1;
	private float sampleRate = 44100 / speedFactor;
	private int bufferSize = 1024 * 4 / speedFactor;
	private int overlap = 512 * 4 / speedFactor;
	
	private String fileName;
		
	public Spectrogram(String fileName){
		this.setLayout(new BorderLayout());
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.setTitle("Spectrogram");
		panel = new SpectrogramPanel();
		this.fileName = fileName;
		
		Vector<Info> info = Shared.getMixerInfo(false, true);
		Mixer mix = AudioSystem.getMixer(info.firstElement());
		try {
			setNewMixer(mix);
		} catch (LineUnavailableException | UnsupportedAudioFileException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		
		JPanel otherContainer = new JPanel(new BorderLayout());
		otherContainer.add(panel,BorderLayout.CENTER);
		otherContainer.setBorder(new TitledBorder("3. Utter a sound (whistling works best)"));
		
		
		this.add(otherContainer,BorderLayout.CENTER);
	}
	
	
	
	private void setNewMixer(Mixer mixer) throws LineUnavailableException, UnsupportedAudioFileException {

		if(dispatcher!= null){
			dispatcher.stop();
		}
		if(fileName == null){
			final AudioFormat format = new AudioFormat(sampleRate, 16, 1, true,
					false);
			final DataLine.Info dataLineInfo = new DataLine.Info(
					TargetDataLine.class, format);
			TargetDataLine line;
			line = (TargetDataLine) mixer.getLine(dataLineInfo);
			final int numberOfSamples = bufferSize;
			line.open(format, numberOfSamples);
			line.start();
			final AudioInputStream stream = new AudioInputStream(line);

			JVMAudioInputStream audioStream = new JVMAudioInputStream(stream);
			// create a new dispatcher
			dispatcher = new AudioDispatcher(audioStream, bufferSize,
					overlap);
		} else {
			try {
				File audioFile = new File(fileName);
				dispatcher = AudioDispatcherFactory.fromFile(audioFile, bufferSize, overlap);
				AudioFormat format = AudioSystem.getAudioFileFormat(audioFile).getFormat();
				dispatcher.addAudioProcessor(new AudioPlayer(format));
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		// add a processor, handle pitch event.
		dispatcher.addAudioProcessor(fftProcessor);

		// run the dispatcher (on a new thread).
		new Thread(dispatcher,"Audio dispatching").start();
	}
	
	SilenceDetector silenceDetect = new SilenceDetector(-90.0, false);
	PopDetector popDetect = new PopDetector(bufferSize/2);
	AudioProcessor fftProcessor = new AudioProcessor(){
		
		FFT fft = new FFT(bufferSize);
		float[] amplitudes = new float[bufferSize/2];

		@Override
		public void processingFinished() {
			// TODO Auto-generated method stub
		}

		@Override
		public boolean process(AudioEvent audioEvent) {
			float[] audioFloatBuffer = audioEvent.getFloatBuffer();
			float[] transformbuffer = new float[bufferSize*2];
			System.arraycopy(audioFloatBuffer, 0, transformbuffer, 0, audioFloatBuffer.length); 
			fft.forwardTransform(transformbuffer);
			fft.modulus(transformbuffer, amplitudes);
			
			int status = 0;
			if(silenceDetect.isSilence(audioFloatBuffer)) {
				popDetect.silence();
				status = 1;
			} else {
				popDetect.noise(amplitudes);
			}
			
			if(popDetect.popEvent()) {
				status = 2;
			}
			
			panel.drawFFT(amplitudes,status);
			panel.repaint();
			return true;
		}
		
	};
	
	public static void main(final String... strings) throws InterruptedException,
			InvocationTargetException {
		SwingUtilities.invokeAndWait(new Runnable() {
			@Override
			public void run() {
				try {
					UIManager.setLookAndFeel(UIManager
							.getSystemLookAndFeelClassName());
				} catch (Exception e) {
					// ignore failure to set default look and feel;
				}
				JFrame frame = strings.length == 0 ? new Spectrogram(null) : new Spectrogram(strings[0]) ;
				frame.pack();
				frame.setSize(640, 480);
				frame.setVisible(true);
			}
		});
}
	

}
