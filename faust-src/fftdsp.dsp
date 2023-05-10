declare name "fft";
declare version "1.0";
declare author "Dominick Hing";
declare license "MIT Style STK-4.2";
declare description "FFT Faust Plugin for JackTrip, using FFT window of 512 and including postive and negative frequencies";

import("stdfaust.lib");

fftWindowSize = 4; // window is 128 samples long
process = an.rtocv(fftWindowSize) : an.fft(fftWindowSize) : an.c_select_pos_freqs(fftWindowSize) : an.c_magsq(fftWindowSize/2+1);