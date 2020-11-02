% Octave script to run after
% faust2octave tlimiter.dsp
figure();
plot(db(max(0.01,abs(faustout(1:1000,:)))),'linewidth',2);
grid('on');
axis('tight');
xlabel('Time (samples)');
ylabel('Amplitude (dB)');
legend('input', 'cmpr-lad-mono','lim-lad-mono','FB-linear', 'FB-log', 'Env');
title('Sine-response for limiters in <faustlibraries>/compressors.lib','fontsize',12);
print -dpdf 'limiter-sine-test.pdf'
