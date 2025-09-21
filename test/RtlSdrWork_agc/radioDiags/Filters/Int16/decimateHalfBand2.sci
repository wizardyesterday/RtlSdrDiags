//******************************************************************
// This program generates the required filter coefficients so that
// an input signal, that is sampled at 1024000S/s can be decimated
// to 512000 S/s.  We want to limit the bandwidth of the aliasing
// filter to 27200 Hz.  The filter coefficients are computed by
// the minimax function, and the result is an equiripple linear
// phase FIR filter.
// The filter specifications are listed below.
//
// Stopband Attenuation: 45dB.
// Pass Band: 0 <= F <= 27200 Hz.
// Transition Band: 27200 < F <= 484800 Hz.
// Stop Band: 484800 < F < 512000 Hz.
// Passband Ripple: 10^(-Attenuation/20).
// Stopband Ripple: 10^(-Attenuation/20).
//
// Note that the filter length will be automatically  calculated
// from the filter parameters.
// Chris G. 01/26/2018
//******************************************************************

//******************************************************************
// Function declarations.
//******************************************************************

//******************************************************************
//
// Name dInf
//
// Purpose: The purpose of this function is to compute the measure,
// D-infinity, that is used for computation of the required filter
// order for an equal ripple filter using the minimax algorithm.
// The origin of this algorithm is from the book, "Multirate
// Digital Signal Processing" by Ronald E. Crochiere and Lawrence
// R. Rabiner.
//
// Calling sequence: x = dInf(deltaP,deltaF)
//
// Inputs:
//
//    deltaP - The passband ripple.
//
//    deltaS - The stopband ripple.
//
// Outputs:
//
//    x - The value of D-infinity.
// 
//******************************************************************
function x = dInf(deltaP,deltaS)

// Set parameters.
a1 = 0.005309;
a2 = 0.07114;
a3 = -0.4761;
a4 = -0.00266;
a5 = -0.5941;
a6 = -0.4278;

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// Break up the computation to make things easier to
// read.
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// Compute first term.
x1 = a1 * log10(deltaP)^2 + a2 * log10(deltaP) + a3;
x1 = log10(deltaS) * x1;

// Compute second term.
x2 = a4 * log10(deltaP)^2 + a5 * log10(deltaP) + a6;
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

// Sum up the two terms.
x = x1 + x2;

endfunction

//******************************************************************
//
// Name f
//
// Purpose: The purpose of this function is to compute the measure,
// f, that is used for computation of the required filter
// order for an equal ripple filter using the minimax algorithm.
// The origin of this algorithm is from the book, "Multirate
// Digital Signal Processing" by Ronald E. Crochiere and Lawrence
// R. Rabiner.
//
// Calling sequence: x = f(deltaP,deltaF)
//
// Inputs:
//
//    deltaP - The passband ripple.
//
//    deltaS - The stopband ripple.
//
// Outputs:
//
//    x - The value of f.
// 
//******************************************************************
function x = f(deltaP,deltaS)

x = 11.012 + 0.512 * (log10(deltaP) - log10(deltaS));

endfunction

//******************************************************************
//
// Name computeFilterOrder
//
// Purpose: The purpose of this function is to compute the filter
// order for an equiripple FIR filter.
// The origin of this algorithm is from the book, "Multirate
// Digital Signal Processing" by Ronald E. Crochiere and Lawrence
// R. Rabiner.
//
// Calling sequence: n = computeFilterOrder(deltaP,
//                                          deltaS,
//                                          deltaF,
//                                          Fs)
//
// Inputs:
//
//    deltaP - The passband ripple.
//
//    deltaS - The stopband ripple.
//
//    deltaF - The bandwidth of the transition band in the
//    normalized frequency domain.
//
//    Fs - The sampling frequency in sample per second.
//
// Outputs:
//
//    n - The filter order.
// 
//******************************************************************
function n = computeFilterOrder(deltaP,deltaS,deltaF,Fs)

// Compute number of taps.
n = dInf(deltaP,deltaS) / deltaF - f(deltaP,deltaS) * deltaF + 1;

// Round up to the next highest integer.
n = ceil(n);

endfunction
//******************************************************************
// Set up parameters.
//******************************************************************
// Sample rate is 1024000 S/s.
Fsample = 1024000;

// Passband edge.
Fp = 27200;

// Stopband edge.
Fs = (Fsample / 2) - Fp;

// The desired demodulator bandwidth.
F = [0 Fp; Fs Fsample/2];

// Bandwidth of transition band.
deltaF = (Fs - Fp) / Fsample;

// Stopband attenuation is 45dB.
Attenuation = 45;

// Passband ripple
deltaP = 10^(-Attenuation/20);

// Stopband ripple.
deltaS = deltaP;

// Number of taps for our filter.
//n = computeFilterOrder(deltaP,deltaS,deltaF,Fs)
n = 3;

//******************************************************************
// Generate the FIR filter coefficients and magnitude of frequency
// response..  
//******************************************************************

//------------------------------------------------------------------
// This will be an antialiasing filter the preceeds the 4:1
// compressor.
//------------------------------------------------------------------
h = eqfir(n,F/Fsample,[1 0],[1 1]);

// Compute magnitude and frequency points.
[hm,fr] = frmag(h,1024);

//******************************************************************
// Plot magnitudes.
//******************************************************************
set("figure_style","new");

title("Antialiasing Filter - Sample Rate: 1024000 S/s  (non-decimated)");
a = gca();
a.margins = [0.225 0.1 0.125 0.2];
a.grid = [1 1];
a.x_label.text = "F, Hz";
a.y_label.text = "|H|";
plot2d(fr*Fsample,hm);

