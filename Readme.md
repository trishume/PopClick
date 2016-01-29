# Lip Popping Recognizer

This project implements a simple algorithm for recognizing when the user
makes a popping noise with their lips. The plan is to use this in concert
with my [eye tracker](https://theeyetribe.com/) as a way to click without
using my hands.

It is currently being rewritten as a [Vamp plugin](http://vamp-plugins.org/) in C++.
I develop the Vamp plugin in Sublime Text, compile it to a dylib and then load it in [Sonic Visualizer](http://www.sonicvisualiser.org/)
which is a great tool for developing audio recognition algorithms.

I plan on expanding this to recognize other mouth noises. This is an unsolved
problem in audio recognition as it has very different goals from other domains like speech recognition:

1. Noises are much easier to distinguish from each other. You can look at a spectrograph and identify if something is the noise you want, not so for a spoken word.
2. Goal is to be resource efficient and real-time. Generally speech recognizers don't do this because good results require heavy processing.
3. Highly reliable, very low false positives only a few false negatives. This is only possible because of the ease of distinguishing.

## Sonic Visualizer Screenshot

![Screenshot](http://i.imgur.com/2UsEBmQ.png)

The top row is a spectrograph, the middle is a debug visualization of the algorithm state, the bottom row is the waveform and some parameters.
The red lines on the bottom row are when a lip pop can be recognized (if it was real time), the three lip pops and three non-pops are correctly classified.

This is the layout I use for debugging the algorithm, [Sonic Visualizer](http://www.sonicvisualiser.org/) allows me to inspect values and scroll and zoom around the test audio file.

## Building

Run `make -f Makefile.osx` on osx, change the `.osx` to the correct Makefile if you are on another platform.
