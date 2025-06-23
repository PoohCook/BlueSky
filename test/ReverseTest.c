#include "ReverseTest.h"
#include "reverse.h"
#include "Eeyore.h"

/*---------------------------------------------------------------------------------------------
 Remove trailing newlines and carriage returns
---------------------------------------------------------------------------------------------
*/
void Chomp(char *pLine)
{
    char *p = pLine + strlen(pLine) - 1; /* Point to last char in pLine*/
    /* Loop removing trailing newlines and carriage returns. Don't go past beginning of string.*/
    for (; p >= pLine && (*p == '\n' || *p == '\r'); --p)
        *p = 0; /* Erase the current character.*/
}

/*---------------------------------------------------------------------------------------------
 Convert a hex string to binary. Return the number of bytes in the result.
---------------------------------------------------------------------------------------------
*/
int HexToBinary(char *hex, unsigned char *binary, int lenBinary)
{
    int i, len, hexlen;
    char convert[3] = {0, 0, 0}; /* Buffer to convert two hex chars at a time to binary*/

    Chomp(hex); /* Remove trailing newlines*/
    hexlen = strlen(hex);
    len = (hexlen + 1) / 2;
    if (len > lenBinary)
        len = lenBinary;

    /* For each byte, convert from ASCII hex to binary */
    for (i = 0; i < len; i++)
    {
        if (hexlen - i * 2 - 2 < 0) /* Handle odd number of digits*/
            convert[0] = '0';
        else
            convert[0] = hex[hexlen - i * 2 - 2];                       /* Copy two chars from input string into 'convert'*/
        convert[1] = hex[hexlen - i * 2 - 1];                           /*  so that we can convert one byte at a time.     */
        binary[len - i - 1] = (unsigned char)strtol(convert, NULL, 16); /* Convert from hex to binary*/
    }

    return len;
}

void BinaryToHex(char *hex, unsigned char *binary, int len)
{
    for (int i = 0; i < len; i++)
        sprintf(hex + i * 2, "%02X", binary[i]);
}


void test_reverse(void)
{

    test_setup();

    unsigned char bits[40]; /* Stores bits to be reversed and result of reversal */
    char result_buffer[40]; /* Pointer to the bits array for convenience */
    int len;

    char *testStr1 = "550130";
    len = HexToBinary(testStr1, bits, sizeof(bits));

    ReverseBits(bits, len);

    BinaryToHex(result_buffer, bits, len);
    assert_str_equal(result_buffer, "0C80AA", "Reversed bits of 550130 should be 0C80AA");

    char *testStr2 = "200F";
    len = HexToBinary(testStr2, bits, sizeof(bits));

    ReverseBits(bits, len);

    BinaryToHex(result_buffer, bits, len);
    assert_str_equal(result_buffer, "F004", "Reversed bits of 550130 should be F004");


    char *testStr3 = "400";
    len = HexToBinary(testStr3, bits, sizeof(bits));

    ReverseBits(bits, len);

    BinaryToHex(result_buffer, bits, len);
    assert_str_equal(result_buffer, "0020", "Reversed bits of 550130 should be 0020");

    char *testStr4 = "15";
    len = HexToBinary(testStr4, bits, sizeof(bits));

    ReverseBits(bits, len);

    BinaryToHex(result_buffer, bits, len);
    assert_str_equal(result_buffer, "A8", "Reversed bits of 550130 should be A8");

}
