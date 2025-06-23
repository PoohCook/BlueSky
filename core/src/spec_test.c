/* Test program for bit reversal functions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reverse.h"

/* Prototypes  */
int HexToBinary(char* hex, unsigned char *binary, int lenBinary);


/*-----------------------------------------------------------------------------------------------
 Program to test bit reversal code. Standard arguments are not used.
  The main program loop reads lines from stdin converts the input line from ASCII
  hex to an array of bytes (unsigned chars), reverses the bits, and prints the results. This
  keeps looping until CTRL-C is pressed or end-of-file is reached,
----------------------------------------------------------------------------------------------
*/
int main(void)
{
  char buf[80];                    /* Input buffer for ASCII hex string*/
  unsigned char bits[40];          /* Stores bits to be reversed and result of reversal */
  int len, i;

  /* Loop reading and reversing hex string until EOF or CTRL-C */
  printf("Enter hexadecimal number to be bit reversed. Example: 3F2C45\n");
  while (fgets(buf, sizeof(buf), stdin) != NULL) {
    len = HexToBinary(buf, bits, sizeof(bits));

    /* Print starting value of bits array (hex)*/
    for (i=0; i<len; i++)
      printf("%02X", bits[i]);
    printf(" --> ");

    ReverseBits(bits, len);

    /* Print reversed bits array (hex) */
    for (i=0; i<len; i++)
      printf("%02X", bits[i]);
    printf("\n");
  }

  return 0;
}


/*---------------------------------------------------------------------------------------------
 Remove trailing newlines and carriage returns
---------------------------------------------------------------------------------------------
*/
void Chomp(char* pLine)
{
  char *p = pLine + strlen(pLine) - 1;  /* Point to last char in pLine*/
  /* Loop removing trailing newlines and carriage returns. Don't go past beginning of string.*/
  for ( ; p >= pLine && (*p == '\n' || *p == '\r') ; --p)
    *p = 0;  /* Erase the current character.*/
}


/*---------------------------------------------------------------------------------------------
 Convert a hex string to binary. Return the number of bytes in the result.
---------------------------------------------------------------------------------------------
*/
int HexToBinary(char* hex, unsigned char *binary, int lenBinary)
{
  int i, len, hexlen;
  char convert[3] = { 0, 0, 0 };   /* Buffer to convert two hex chars at a time to binary*/

  Chomp(hex);              /* Remove trailing newlines*/
  hexlen = strlen(hex);
  len = (hexlen+1) / 2;
  if (len > lenBinary) len = lenBinary;

  /* For each byte, convert from ASCII hex to binary */
  for (i=0; i<len; i++) {
    if (hexlen - i*2 - 2 < 0)   /* Handle odd number of digits*/
      convert[0] = '0';
    else
      convert[0] = hex[hexlen - i*2 - 2];      /* Copy two chars from input string into 'convert'*/
    convert[1] = hex[hexlen - i*2 - 1];       /*  so that we can convert one byte at a time.     */
    binary[len - i - 1] = (unsigned char) strtol(convert, NULL, 16);  /* Convert from hex to binary*/
  }

  return len;
}
