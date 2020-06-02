What's integrated in jacktrip is limiterdsp.dsp, which includes limiter.dsp.

You can use fausteditor.grame.fr to compile and download an executable.
With Faust installed, you can do it at the command line:

To make a standalone JACK app for Mac, say

faust2jaqt limiterdsp.dsp

For Linux, you want either that or

faust2jack limiterdsp.dsp

(to use GTK in place of Qt for the GUI).

Then you just run it and patch it in (using qjackctl) between your audio capture and jacktrip input.
