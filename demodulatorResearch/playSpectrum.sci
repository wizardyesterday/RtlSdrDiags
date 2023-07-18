//******************************************************************
// File name: playSignal.sci
//******************************************************************

//**********************************************************************
//
//  Name: playSpectrum
//
//  Purpose: The purpose of this function is to to read a file of
//  IQ data samples and display the signal and the spectrum in an
//  animated fashion.
//  The data stream arrives as I,Q,I,Q.... The data format is
//  8-bit 2's complement.
//
//  Calling Sequence: state = playSpectrum(fileName,
//                                         segmentSize,
//                                         totalSamples,
//                                         dwellTime)
//
//  Inputs:
//
//    fileName - The name of the rile that contains the IQ samples.
//
//    segmentSize - The number of samples to read each time.
//
//    totalSamples - The total number of samples in the file.
//
//    dwellTime - The time, in milliseconds, to pause after each
//    plot of the magnitude data.
//
//  Outputs:
//
//    None.
//
//**********************************************************************
function playSpectrum(fileName,segmentSize,totalSamples,dwellTime)

  // Construct Hanning window.
  win = window('hn',segmentSize/2);

  // Convert to microseconds.
  delay = dwellTime * 1000;

  // Open the file.
  fd = mopen(fileName);

  // Set up the display.
  scf(1);

  // Initialize or loop entry.
  done = 0;

  while done == 0

    // This helps to figure out where, in the file, interesting parts are.
    filePosition = mtell(fd);
    printf("%d\n",filePosition);

    // Grab the next segment of the signal.
    x = mget(segmentSize,'c',fd);

    if length(x) == 0
      // All data has been read, so bail out.
      done = 1;
    else
      // Separate the in-phase and quadrature components.
      i = x(1:2:$);
      q = x(2:2:$);

      m = sqrt(i.*i + q.*q);

      // Window the data.
      i = i .* win;
      q = q .* win;

      // Form complex vector.
      z = i + %i*q;

      // Form frequency domain vector.
      Z = fft(z,-1);

      // Place zero frequency at the center.
      Z = fftshift(Z);

      // Display the signal magnitude.
      subplot(211);
      title('Signal Magnitude sqrt(i^2 + q^2)');
      plot(m);

      // Display spectrum magnitude.
      subplot(212);
      title('Power Spectrum, dB');
      plot(10*log10(abs(Z)));

      // Pause for a little bit.
      xpause(delay);

      // Clear the plot.
      clf(1);
    end

    if filePosition > totalSamples
      // We're done.
      done = 1;
    end // if

  end  // while

  // We're done with the file.
  mclose(fd);

endfunction

//*******************************************************************
// Mainline code.
//*******************************************************************

playSpectrum('yoyo.iq',4096,3000000,500);
//playSpectrum('f135_4.iq',4096,3000000,500);
//playSpectrum('f120_35.iq',4096,40000000,500);
//playSpectrum('f162_425.iq',4096,40000000,500);
//playSpectrum('f154_845.iq',4096,40000000,500);
//playSpectrum('f90_1.iq',4096,40000000,500);
