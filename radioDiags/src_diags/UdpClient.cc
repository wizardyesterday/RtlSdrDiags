//**********************************************************************
// file name: UdpClient.cc
//**********************************************************************

#include "UdpClient.h"
#include <stdio.h>

/**************************************************************************

  Name: UdpClient

  Purpose: The purpose of this function is to serve as the constructor
  of a UdpClient object.  This function opens a socket, and if
  successful, populates the peer address structure with the appropriate
  information.

  Calling Sequence: UdpClient(ipAddressPtr,port)

  Inputs:

    ipAddressPtr - A pointer to a character string that represents the
    IP address of the link partner.  The format is in dotted decimal
    notation.

    port - The port number for the link partner listener.

  Outputs:

    None.

**************************************************************************/
UdpClient::UdpClient(char *ipAddressPtr,int port)
{
  int bufferLength;
  int status;

  // Let's make a large enough buffer.
  bufferLength = 32768;

  // Create UDP socket.
  socketDescriptor = socket(PF_INET,SOCK_DGRAM,0);

  if (socketDescriptor != -1)
  {
    // Always zero out the address structure to avoid side effects.
    bzero(&peerAddress,sizeof(peerAddress));

    // Populate the fields.
    peerAddress.sin_family = AF_INET;
    peerAddress.sin_addr.s_addr = inet_addr(ipAddressPtr);
    peerAddress.sin_port = htons(port);

    // Set the socket transmit buffer size.
    status = setsockopt(socketDescriptor,
                        SOL_SOCKET,
                        SO_SNDBUF,
                        &bufferLength,
                        sizeof(int));
  } // if
  else
  {
    // Indicate that the socket has not been opened.
    socketDescriptor = 0;
  } // else

  return;
 
} // UdpClient

/**************************************************************************

  Name: ~UdpClient

  Purpose: The purpose of this function is to serve as the destructor
  of a UdpClient object.

  Calling Sequence: ~UdpClient()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
UdpClient::~UdpClient(void)
{

  if (socketDescriptor != 0)
  {
    close(socketDescriptor);
  } // if

  return;
 
} // ~UdpClient

/**************************************************************************

  Name: connectionIsEstablished

  Purpose: The purpose of this function is to serve as the destructor
  of a UdpClient object.

  Calling Sequence: status = connectionIsEstablished()

  Inputs:

    None.

  Outputs:

    status - A boolean that indicates whether or not a connection is
    established.  A value of true indicates that a connect has been
    established, and a value of false indicates that a connection has
    not been established.

**************************************************************************/
bool UdpClient::connectionIsEstablished(void)
{
  bool status;

  if (socketDescriptor != 0)
  {
    status = true;
  } // if
  else
  {
    status = false;
  } // else

  return (status);
 
} // connectionIsEstablished

/**************************************************************************

  Name: sendData

  Purpose: The purpose of this function is to send a buffer of data over
  a UDP connection.  Note that for a nonblocking sendto(), replace the 0
  in the fourth argument (flags) with MSG_DONTWAIT.

  Calling Sequence: success = sendData(bufferPtr,bufferLength)

  Inputs:

    bufferPtr - A pointer to the octets to send.

    bufferLength - The number of octets to send.

    blockSize - The size of the payload of a UDP datagram.  Sometimes,
    the link partner might be an app, such as netcat, that reads its
    input from stdin, and sends 2048-octet payloads.  The receiver
    also expects this 2048-octet payload, and it will read 2048 bytes
    from stdin.  Any other length payload, that is sent, just won't
    work. 

  Outputs:

    success - An indicator of the outcome of the operation.  A value of
    true indicates that the operation was successful, and a value of false
    indicates that the operation was not successful.

**************************************************************************/
bool UdpClient::sendData(void *bufferPtr,int bufferLength,int blockSize)
{
  bool success;
  bool failureOccurred;
  ssize_t count;
  int i;
  int numberOfBlocks;
  int remainder;
  unsigned char *octetPtr;

  if (blockSize > 2048)
  {
    // Restrict the length to be nice to netcat.
    blockSize = 2048;
  } // if

  // Reference the buffer in the octet context.
  octetPtr = (unsigned char *)bufferPtr;

  numberOfBlocks = bufferLength / blockSize;
  remainder = bufferLength % blockSize;

  // Default to failure.
  success = false;

  // Default to no failures.
  failureOccurred = false;

  // We only send to open sockets.
  if (socketDescriptor != 0)
  {
    // Let's break up the payload to be nice to the link partner.
    for (i = 0; i < numberOfBlocks; i++)
    {
      // Send the data in blocking mode.  For non-blocking mode, delete the
      // 0, and uncomment MSG_DONTWAIT.
      count = sendto(socketDescriptor,
                     octetPtr,
                     blockSize,
                     0,//MSG_DONTWAIT,
                     (struct sockaddr *)&peerAddress,
                     sizeof(sockaddr));

      // Reference next block in buffer.
      octetPtr += blockSize;
  
      if (count != blockSize)
      {
        failureOccurred = true;
      } // if
    } // for

    if (remainder != 0)
    {
      count = sendto(socketDescriptor,
                     octetPtr,
                     remainder,
                     0,//MSG_DONTWAIT,
                     (struct sockaddr *)&peerAddress,
                     sizeof(sockaddr));

      if (count != remainder)
      {
        // Indicate that a failure occurred.
        failureOccurred = true;
      } // if
    } // if

    // Capture accumulated failures.
    success = failureOccurred;
  } // if

  return (success);

} // sendData
