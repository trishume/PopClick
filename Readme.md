# Lip Popping Recognizer

This project implements a simple algorithm for recognizing when the user
makes a popping noise with their lips. The plan is to use this in concert
with my [eye tracker](https://theeyetribe.com/) as a way to click without
using my hands.

It is currently being rewritten as a [VAMP plugin](http://vamp-plugins.org/) in C++.

I plan on expanding this to recognize other mouth noises. This is an unsolved
problem in audio recognition as it has very different goals from other domains like speech recognition:

1. Noises are much easier to distinguish from each other. You can look at a spectrograph and identify if something is the noise you want, not so for a spoken word.
2. Goal is to be resource efficient and real-time. Generally speech recognizers don't do this because good results require heavy processing.
3. Highly reliable, very low false positives only a few false negatives. This is only possible because of the ease of distinguishing.
