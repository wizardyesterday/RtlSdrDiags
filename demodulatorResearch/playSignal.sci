//******************************************************************
// File name: playSignal.sci
//******************************************************************
// This program assumes that an IQ data file contains data that is
// sampled at 256000S/s.  Data segments should be powers of 2 so
// that no zero padding is needed for the case that an FFT is run.
//******************************************************************

//**********************************************************************
//
//  Name: playSignal
//
//  Purpose: The purpose of this function is to to read a file of
//  IQ data samples and display the signal and the spectrum in an
//  animated fashion.
//  The data stream arrives as I,Q,I,Q.... The data format is
//  8-bit 2's complement.
//
//  Calling Sequence: state = playSignal(fileName,
//                                         segmentSize,
//                                         totalSamples,
//                                         dwellTime,
//                                         sig)
//
//  Inputs:
//
//    fileName - The name of the rile that contains the IQ samples.
//
//    segmentSize - The number of samples to read each time.  The
//    number of complex samples that are read is segmentSize / 2.
//    The duration of each segment of complex samples is,
//    (segmentSize / 2) / sampleRate,
//    where sampleRate is the system sample rate in samples (complex)
//    per second.  For example, with a sample rate of 256000S/s, and
//    a segment size of 4096 bytes (8-bit samples), the segment duration
//    is 2048 / 256000 = 8ms.
//
//    totalSamples - The total number of samples in the file.
//
//    dwellTime - The time, in milliseconds, to pause after each
//    plot of the magnitude data.
//
//    sig - An indicator of the type of data to display.  Valid values
//    are:
//      1 - Display signal magnitude.
//      2 - Display magnitude spectrum.
//      3 -- Display signal magnitude and magnitude spectrum.
//
//  Outputs:
//
//    None.
//
//**********************************************************************
function playSignal(fileName,segmentSize,totalSamples,dwellTime,sig)

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

      select sig
        case 1
          // Display the signal magnitude.
          title('Signal Magnitude sqrt(i^2 + q^2)');
          plot(m);

        case 2
          // Display spectrum magnitude.
          title('Power Spectrum, dB');
          plot(20*log10(abs(Z)));

        case 3
          // Display the signal magnitude.
          subplot(211);
          title('Signal Magnitude sqrt(i^2 + q^2)');
          plot(m);
          // Display spectrum magnitude.
          subplot(212);
          title('Power Spectrum, dB');
          plot(20*log10(abs(Z)));

      end // select

      // Pause for a little bit.
      xpause(delay);
    end

    // Clear the display.
    clf(1);

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
//playSignal('yoyo.iq',4096,3000000,500,3);
//playSignal('f135_4.iq',4096,3000000,500,1);
playSignal('f120_35.iq',4096,40000000,500,2);
//playSignal('f162_425.iq',4096,40000000,500,1);
//playSignal('f154_845.iq',4096,40000000,500,1);
//playSignal('f90_1.iq',4096,40000000,500,1);
