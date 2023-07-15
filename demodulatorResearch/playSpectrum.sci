//******************************************************************
// File name: playSignal.sci
//******************************************************************

//**********************************************************************
//
//  Name: playSpectrum
//
//  Purpose: The purpose of this function is to to read a file of
//  IQ data samples and display the spectrum in an animated fashion.
//  The data stream arrives as I,Q,I,Q.... The data format is
//  8-bit 2's complement.
//
//  NOTE: This is a work in progress. I will probably update this
//  function to use something like the overlap add method (or the
//  overlap save method).  Additionally, I will window the data.
//
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

      // Form complex vector.
      z = i + %i*q;

      // Form frequency domain vector.
      Z = fft(z,-1);

     // Display the signal magnitude.
      plot(abs(Z(1:length(z)/2)));

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
//playSpectrum('f135_4.iq',20000,3000000,500);
