                                                                                     //**********************************************************************
// File Name: utils.sci
//**********************************************************************

//**********************************************************************
//
//  Name: shiftSampleInPipeline
//
//  Purpose: The purpose of this function is to place a new sample
//  into the pipeline referenced by the state parameter.  The format
//  of the pipeline appears below.
//
//      {x(n) x(n-1) x(n-2)...}
//
//  Calling Sequence: state = shiftSampleInPipeline(x,state)
//
//  Inputs:
//
//    x - The input sample to be shifted into the pipeline.
//
//    state - The pipeline that is typically used to store the state
//    of a digital filter.  The order of storage appears belos.
//
//  Outputs:
//
//    state - The updated pipeline.
//
//**********************************************************************
function state = shiftSampleInPipeline(x,state)

  // Ensure we have a column vector.
  state = state(:);

  // Shift the contents of the pipeline.
  state(2:$) = state(1:$-1);

  // Store sample value.
  state(1) = x;

endfunction

//**********************************************************************
//
//  Name: iirFilter
//
//  Purpose: The purpose of this function is to filter a signal using
//  a recursive filter (in this case an infinite impulse response).
//  Advantage has been taken of specific features of Scilab for the
//  computation of the convolution sums.  Specifically, the for loop
//  has been replaced with the inner product of a pipeline and the
//  corresponding filter coefficients.
//
//  Calling Sequence: [y,bState,aState = iirFilter(x,b,a,bState,aState)
//
//  Inputs:
//
//    x - The input sample.
//
//    b - The numerator (feedforward) coefficients of the filter.
//
//    a - The denominator (feedback) coefficients of the filter.  Note that
//    the unity term (leading zero power coefficient) is not expected to
//    be passed to this function.  If a = [1 0.5 -0.1], only [0.5 -0.1]
//    would be passed to the function, and the calling sequence would be
//    [y,bState,aState = iirFilter(x,b,a(2:$),bState,aState).
//
//    bState - The pipeline for the nonrecursive portion of the filter.
//
//    aState - The pipeline for the recursive portion of the filter.
//
//  Outputs:
//
//    y - The output of the filter.
//
//    bState - The pipeline for the nonrecursive portion of
//    the filter updated with the latest sample value.
//
//    aState - The pipeline for the recursive portion of the
//    filter updated with the latest output value.
//
//**********************************************************************
function [y,bState,aState] = iirFilter(x,b,a,bState,aState)

  // Ensure that we have column vectors.
  a = a(:);
  b = b(:);
  aState = aState(:);
  bState = bState(:);

  // Shift input sample into the forward pipeline.
  bState = shiftSampleInPipeline(x,bState);

 // Perform convolution sum for the nonrecursive portion of the filter.
  y = b'*bState;

  // Perform convolution sum for the recursive portion of the filter.
  y = y - a'*aState;

  // Shift output sample into the recursive pipeline.
  aState = shiftSampleInPipeline(y,aState);

endfunction

//**********************************************************************
//
//  Name: filterBlock
//
//  Purpose: The purpose of this function is to filter a signal using
//  a recursive filter.  The function accepts a block of input data
//  and generates a block of output data.  The pipelines are created in
//  this function since it only processes one block of data.
//
//  Calling Sequence: y = filterBlock(x,b,a)
//
//  Inputs:
//
//    x - The input vector.
//
//    b - The numerator coefficients of the filter.
//
//    a - The denominator coefficients of the filter.  Note that
//    the unity term (leading zero power coefficient) is not expected to
//    be passed to this function.  If a = [1 0.5 -0.1], only [0.5 -0.1]
//    would be passed to the function, and the calling sequence would be
//    y = filterBlock(x,b,a(2:$).
//
//  Outputs:
//
//    y - The output vector.
//
//**********************************************************************
function y = filterBlock(x,b,a)

  // Instantiate the pipelines.
  aState = zeros(1,length(a));
  bState = zeros(1,length(b));

  // Compute the filter response on a sample by sample basis.
  for i = 1:length(x)
    [y(i),bState,aState] = iirFilter(x(i),b,a,bState,aState);
  end

endfunction

//**********************************************************************
//
//  Name: convm
//
//  Purpose: The purpose of this function is to construct a convolution
//  matrix.  Now, what exactly is a convolution matrix?  I'm glad you
//  asked that question!  Let it be supposed that we have an FIR
//  (nonrecursive) filter, h(n) = [h(0) h(1) ... h(q-1)]'.  That is,
//  h(n) is described as a column vector.  Given that we have an input,
//  x(n), the output, y(n) = x(n) * h(n) = h(n) * x(n), where '*'
//  indicates convolution.  Now, convolution can be expressed as an
//  inner product, y = xh, where x is taken to be a row vector, and,
//  h is a column vector.  In general, an FIR is implemented  as a
//  multitap delay line for which a convolution sum is carried out
//  by the inner product of the delay line and the filter coefficients,
//  h.
//  What this function does is that it constructs, from an input
//  sequence, x of length N, a matrix X with p columns and N+p-1 rows.
//  Each row of the matrix, X, represents the state of a delay line,
//  of length p, at time index n.  By post-multiplying row 1 of X by h,
//  we have the output y(0), by multiplying row2 of X by h, we have
//  y(1), and so forth.  In general, the vector, y = Xh.  The
//  structure of the matrix with x = [x(0) x(1) x(2) x(3)]' and p = 4, is,
//
//              [x(0)  0     0        0]
//              [x(1)  x(0)  0        0]
//              [x(2)  x(1)  x(0)     0]
//              [x(3)  x(2)  x(1)  x(0)]
//              [0     x(3)  x(2)  x(1)]
//              [0     0     x(3)  x(2)]
//              [0     0     0     x(3)]
//
//  Calling Sequence: X = convm(x,p)
//
//  Inputs:
//
//    x - The input vector to be processed.
//
//    p - The desired number of columns in the matrix.
//
//  Outputs:
//
//    X - The convolution matrix.
//
//**********************************************************************
function X = convm(x,p)

  N = length(x) + 2 * p - 2;

  // Ensure that we have a column vector.
  x = x(:);

  // Construct the appropriate zero padding.
  xpad = [zeros(p-1,1); x; zeros(p-1,1)];

  // Construct the matrix.
  for i = 1:p
    // Construct current column vector.
    X(:,i) = xpad(p-i+1:N-i+1);
  end

endfunction

//**********************************************************************
//
//  Name: flipud
//
//  Purpose: The purpose of this function is to reverse the order of
//  each column of a matrix.
//
//  Calling Sequence: b = flipud(a)
//
//  Inputs:
//
//    a - The input matrix.
//
//  Outputs:
//
//    b - The matrix for which all of the columns of a have been
//    reversed.
//
//**********************************************************************
function b = flipud(a)

  // Determine the number of rows and columns
  s = size(a);

  // Create working copy.
  b = a;

  for i = 1:s(2)
    // Reverse the tontents of the current column.
    b(1:$,i) = a($:-1:1,i);
  end

endfunction

//**********************************************************************
//
//  Name: fliplr
//
//  Purpose: The purpose of this function is to reverse the order of
//  each row of a matrix.
//
//  Calling Sequence: b = fliplr(a)
//
//  Inputs:
//
//    a - The input matrix.
//
//  Outputs:
//
//    b - The matrix for which all of the rows of a have been
//    reversed.
//
//**********************************************************************
function b = fliplr(a)

  // Determine the number of rows and columns
  s = size(a);

  // Create working copy.
  b = a;

  for i = 1:s(1)
    // Reverse the tontents of the current row.
    b(i,1:$) = a(i,$:-1:1);
  end

endfunction

//**********************************************************************
//
//  Name: quantize
//
//  Purpose: The purpose of this function is to quantize a fractional
//  number to a finite number of integer bits.
//
//  Calling Sequence: aQ = quantize(a,b)
//
//  Inputs:
//
//    a - The input vector.
//
//    b - The number integer bits of precision.
//
//  Outputs:
//
//    aQ - The quantized output vector, if the quantization was
//    successful, otherwise, the original vector is returned.
//
//    success - An indicator of the outcome of the operation.  A value
//    of 1 implies that quantization had occurred, and a value of 0
//    implies that the quantization has not occurred.
//
//**********************************************************************
function [aQ,success] = quantize(a,b)

  // Force a column vector.
  a = a(:)

  // Default to failure.
  success = 0;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Search for any numbers whose magnitude matches or
  // exceeds unity.  Due to the fact that this function
  // processes fractional numbers, if numbers greater
  // than or equal to 1 in magnitude, the vector, a', is
  // not a candidate for conversion.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  indexOfBigNumbers = find(abs(a) >= 1);
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  if indexOfBigNumbers == []
    positiveLimit = 2^(b - 1) - 1;
    negativeLimit = 2^(b - 1);

    // Locate negative values.
    n = find(a < 0);

    // Compute the quantized value.
    aQ = a * positiveLimit;
    aQ(n) = a(n) * negativeLimit;

    // Remove any fractional part.
    aQ = round(aQ);

    // Indicate a successful quantization.
    success = 1;
  else
    // Handle the big number case.
    aQ = a;
  end

endfunction

//**********************************************************************
//
//  Name: unQuantize
//
//  Purpose: The purpose of this function is to unquantize an integer
//  number into a signed fractional representation.  Due to the fact,
//  that the input is quantized, the output will be an approximation
//  to the original fractional number since processing is constrained
//  to negative powers of two in the fractional representation.
//
//  Calling Sequence: a = unquantize(aQ,b)
//
//  Inputs:
//
//    b - The number integer bits of precision.
//
//   aQ - The quantized output vector, if the quantization was
//    successful, otherwise, the original vector is returned.
//
//  Outputs:
//
//    a - The input vector.
//
//    success - An indicator of the outcome of the operation.  A value
//    of 1 implies that unquantization had occurred, and a value of 0
//    implies that the unquantization has not occurred.
//
//**********************************************************************
function [a,success] = unquantize(aQ,b)

  // Force a column vector.
  aQ = aQ(:)

  // Default to failure.
  success = 0;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Search for any numbers whose is less than unity and
  // not equal to zero. Due to the fact that this
  // function processes integer numbers, if nonzero
  // numbers less than 1 in magnitude occur,the vector,
  // aQ', is not a candidate for conversion.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Create a working copy of aQ.
  c = aQ;

  // Remove zero entries.
  indexOfZeroNumbers = find(c == 0);
  c(indexOfZeroNumbers) = [];

  // Now, search for numbers less than unity.
  indexOfFractionalNumbers = find(abs(c) < 1);
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  if indexOfFractionalNumbers == []
    positiveLimit = 2^(b - 1) - 1;
    negativeLimit = 2^(b - 1);

    // Locate negative values.
    n = find(aQ < 0);

    // Compute the unquantized value.
    a = aQ / positiveLimit;
    a(n) = aQ(n) / negativeLimit;

    // Indicate a successful quantization.
    success = 1;
  else
    // Handle the fractional number case.
    a = aQ;
  end

endfunction

//**********************************************************************
//
//  Name: generateBipolarNoise
//
//  Purpose: The purpose of this function is to generate a white noise
//  sequence for which the random variable is uniformly distributed
//  between [-amplitude,amplitude].
//
//  Calling Sequence: w = generateBipolarNoise(amplitude,numberOfSamples)
//
//  Inputs:
//
//    amplitude - The maximum amplitude of the bipolar signal.
//
//    numberOfSampels - The number of random numbers to generate.
//
//  Outputs:
//
//    w - A uniformly distributed white noise sequence.
//
//**********************************************************************
function w = generateBipolarNoise(amplitude,numberOfSamples)

  // Ensure that we have a uniform distribution.
  rand('uniform');

  for i = 1:numberOfSamples
    // Generate the next random number.
    a = rand();

    // Translate for symmetry about 0.
    a = a - .5;

    // Scale so that -amplitude <= a <= amplitude.
    a = a * 2 * amplitude;

    // Save in returned vector.
    w(i) = a;
  end

endfunction

//**********************************************************************
//
//  Name: generateGaussianProcess
//
//  Purpose: The purpose of this function is to generate a Gaussian
// white noise process.
//
//  Calling Sequence: X = generateGaussianProcess(numberOfRealizations,
//                                                numberOfSamples,
//                                                sigmaSquared)
//
//  Inputs:
//
//    numberOfRealizations - The number of sample realizations in the
//    process.
//
//    numberOfSampels - The number of samples in each realization.
//
//    sigmaSquared - The variance of the process.
//
//  Outputs:
//
//    X - A matrix of column vectors for which each column represents
//    a sample realization of the process.  For example, if
//    numberOfRealizations is equal to 8, and numberOfSamples is
//    equal to 3, X will have 8 column vectors of length 3.
//
//**********************************************************************
function X = generateGaussianProcess(numberOfRealizations, ...
                                     numberOfSamples, ...
                                     sigmaSquared)

  for j = 1:numberOfRealizations
    // Initialize sample realization.
    noisegen(1,numberOfSamples,sqrt(sigmaSquared));

    // Fill in sample realization.
    X(:,j) = feval([1:numberOfSamples]',Noise);
  end

endfunction

//**********************************************************************
//
//  Name: crosscorrelate
//
//  Purpose: The purpose of this function is to compute the
//  estimated cross-correlation function of two input sequences.
//
//  Calling Sequence: rxy = crosscorrelate(x,y,numberOfSamples,lag)
//
//  Inputs:
//
//    x - The input vector to be processed.  Note that the lag
//    should be sigificantly less than the length of this vector.
//
//    y - The input vector to be cross-correlated with x.  Note that
//    the lag should be sigificantly less than the length of this vector.
//
//    lag - The cross-correlation lag.
//
//  Outputs:
//
//    rxy - The estimated cross-correlation function.
//
//**********************************************************************
function rxy = crosscorrelate(x,y,lag);

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Ensure that numberOfSamples is consistant with the length
  // of the data records.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Compute sequence lengths.
  Lx = length(x);
  Ly = length(y);

  // Choose the smaller of the two lengths.
  numberOfSamples = min(Lx,Ly);
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Start with a clean slate.
  accumulator = 0;

  // Default value if cross-correlation isn't executed.
  rxy = 0;

  if lag < numberOfSamples
    for i = lag+1:numberOfSamples
      // Perform correlation processing.
      accumulator = accumulator + x(i) * conj(y(i - lag));
    end

    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // Set result, and scale.  Note that lag is subtracted to
    // mitigate the end effect.
    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    rxy = accumulator / (numberOfSamples - lag);
  end

endfunction

//**********************************************************************
//
//  Name: pascal
//
//  Purpose: The purpose of this function is to generate a Pascal
//  matrix.  This is performed by taking the matrix exponential of the
//  appropriate generator matrix.
//
//  Calling Sequence: p = pascal(n,Type);
//
//  Inputs:
//
//    n - The order of the matrix.
//
//    Type - The type of matrix.  Valid values are:
//
//      [L|l] - Lower triangular.
//      [U|u] - Upper triangular.
//      [S|s] - Symmetric triangular.
//
//  Outputs:
//
//    P - The constructed Pascal matrix.
//
//**********************************************************************
function p = pascal(n,Type)

  // Construct the initial diagonal of our generator matrices.
  d = diag(1:n-1);

  // Set up the intial generator matrices.
  lgen = d;
  ugen = d;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Construct the subdiagonal matrix
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Add the top row.
  lgen = [zeros(1,n-1); lgen];

  // Add the right column.
  lgen = [lgen zeros(n,1)];
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Construct the superdiagonal matrix
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Add the bottom row.
  ugen = [ugen; zeros(1,n-1)];

  // Add the left column.
  ugen = [zeros(n,1) ugen];
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  select Type

   case 'L'
    // Generate lower triangular matrix.
    p = expm(lgen);

   case 'l'
    // Generate lower triangular matrix.
    p = expm(lgen);

    case 'U'
      // Generate upper triangular matrix.
      p = expm(ugen);

    case 'u'
      // Generate upper triangular matrix.
      p = expm(ugen);

    case 'S'
      // Generate symmetric triangular matrix.
      p = expm(lgen) * expm(ugen);

    case 's'
      // Generate symmetric triangular matrix.
      p = expm(lgen) * expm(ugen);

  end // select

endfunction

//**********************************************************************
//
//  Name: hilbertMatrix
//
//  Purpose: The purpose of this function is to generate a Hilbert
//  matrix, such that each entry of the matrix is defined by,
//  Hij = 1/ (i + j - 1).
//  Scilab provides a command to perform the inverse of this matrix:
//  testmatrix('hilb',n).
//
//  Calling Sequence: h = hilbertMatrix(n)
//
//  Inputs:
//
//    n - The order of the matrix.
//
//  Outputs:
//
//    h - The n by n Hilbert matrix.
//
//**********************************************************************
function h = hilbertMatrix(n)

  // Preallocate the matrix.
  h = zeros(n,n);

  for i = 1:n
    for j = 1:n
      h(i,j) = 1 / (i + j - 1);
    end
  end

endfunction

//**********************************************************************
//
//  Name: buildHadamardMatrix

//
//  Purpose: The purpose of this function is to generate a Hadamard
//  matrix that produces the maximum magnitude of the determinant of
//  this  matrix.
//
//  Calling Sequence: [H, detH] = buildHadamardMatrix(n)
//
//  Inputs:
//
//    n - The order of the Hadamard matrix to be generated.
//
//  Outputs:
//
//    H - The Hadamard matrix that produces the maximum determinant
//    value.
//
//    detH - The determinant of H.
//
//**********************************************************************
function [H, detH] = buildHadamardMatrix(n)

  p = (n - 1)^2;

  // Initialize the template matrix.
  A0 = ones(n,n);

  // Clear initial value of maximum value of the determinant.
  maxdet = 0;

  for k = 0 : 2^p - 1
    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // Generate a vector of negative powers of 2 multipled by
    // the loop index.
    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    m = k .* 2 .^(-p+1:0);

    // Compute the integer part of the vector.
    m = floor(m);

    // We want a vector of 1's and 0's.
    Asub = modulo(m,2);

    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // Reset A to our template since we want the top row and
    // the first column to be all 1's.
    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    A = A0;

    // Update the submatrix of A.
    A(2:n,2:n) = 1 - 2 * matrix(Asub,n-1,n-1);

    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // This block of code ensures that the matrix with the
    // maximum absolute value of its determinant is saved.
    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    if abs(det(A)) > maxdet
      maxdet = abs(det(A));
      maxA = A;
    end
  end

  // Set returned values.
  H = maxA;
  detH = det(maxA);

endfunction

//**********************************************************************
//
//  Name: permutationMatrix
//
//  Purpose: The purpose of this function is to construct a permutation
//  matrix based upon a sequence of column numbers.  The result will
//  be a permuted identity matrix.
//
//  Calling Sequence: < = permutationMatrix(p)
//
//  Inputs:
//
//    p - A vector that contains permuted values of 1..n.
//
//  Outputs:
//
//    M - The permuted identity matrix.  If the vector p (the input
//    data) contains invalid data, a value of 0 is returned.
//
//**********************************************************************
function M = permutationMatrix(p)

  // This will set size of the matrix.
  n = length(p);

  // Initialize to false;
  badData = 0;

  // Check of the data is correct.
  for i = 1:n
    k = find(p == i)

    if k == []
      badData = 1;
    end
  end

  // We only process good data.
  if badData == 0
    // Construct initial matrix.
    M = zeros(n,n);

    for i = 1:n
      // Fill in the current permuted row entry.
      M(i,p(i))= 1;
    end
  else
    // Indicate failure.
    M = 0;
  end

endfunction
