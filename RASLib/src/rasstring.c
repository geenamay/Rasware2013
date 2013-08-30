#include <stdarg.h>
#include <math.h>
#include "driverlib/debug.h"

static const char * const g_pcHex_U = "0123456789ABCDEF";
static const char * const g_pcHex_L = "0123456789abcdef";

// writes n chars from src to dst, then returns a pointer to the location after the last char written to dst
// also sets the location after the last char to 0
static char* StrWrite(char *dst, const char *src, int n) {
    int i;
    
    for (i = 0; i < n; i++) {
        dst[i] = src[i];
    }
    
    dst[n] = 0;
    
    return &dst[n];
}

// does what uart's convert does, except it calls StrWrite instead of UARTwrite at the end
static char* StrConvert(char *dst, unsigned long ulValue, unsigned long ulCount, const char *pcHex, char cNeg, char cFill, unsigned long ulBase)
{
  char pcBuf[16];
  unsigned long ulIdx, ulPos = 0;
  
  for(ulIdx = 1;
      (((ulIdx * ulBase) <= ulValue) &&
      (((ulIdx * ulBase) / ulBase) == ulIdx));
      ulIdx *= ulBase, ulCount--)
  {
  }
  
  // If the value is negative, reduce the count of padding
  // characters needed.
  if(cNeg)
  {
    ulCount--;
  }

  // If the value is negative and the value is padded with
  // zeros, then place the minus sign before the padding.
  if(cNeg && (cFill == '0'))
  {
    // Place the minus sign in the output buffer.
    pcBuf[ulPos++] = '-';

    // The minus sign has been placed, so turn off the
    // negative flag.
    cNeg = 0;
  }

  // Provide additional padding at the beginning of the
  // string conversion if needed.
  if((ulCount > 1) && (ulCount < 16))
  {
    for(ulCount--; ulCount; ulCount--)
    {
      pcBuf[ulPos++] = cFill;
    }
  }

  // If the value is negative, then place the minus sign
  // before the number.
  if(cNeg)
  {
    // Place the minus sign in the output buffer.
    pcBuf[ulPos++] = '-';
  }

  // Convert the value into a string.
  for(; ulIdx; ulIdx /= ulBase)
  {
    pcBuf[ulPos++] = pcHex[(ulValue / ulIdx) % ulBase];
  }

  // Write the string.
  return StrWrite(dst, pcBuf, ulPos);
}

// For dumping printf output into a buffer
void SPrintf(char *str, const char *pcString, ...)
{
  unsigned long ulValue, ulIdx, ulCount, ulDecCount;
  char *pcStr, cNeg, cDec, cFill;
  const char *pcHex;
  va_list vaArgP;

  // Check the arguments.
  ASSERT(pcString != 0);

  // Start the varargs processing.
  va_start(vaArgP, pcString);

  // Loop while there are more characters in the string.
  while(*pcString)
  {
    // Find the first non-% character, or the end of the string.
    for(ulIdx = 0; (pcString[ulIdx] != '%') && (pcString[ulIdx] != '\0');
        ulIdx++)
    {
    }

    // Write this portion of the string.
    str = StrWrite(str, pcString, ulIdx);

    // Skip the portion of the string that was written.
    pcString += ulIdx;

    // See if the next character is a %.
    if(*pcString == '%')
    {
      // Skip the %.
      pcString++;

      // Set the digit count to zero, and the fill character to space
      // (i.e. to the defaults).
      ulCount = 0;
      ulDecCount = 6;
      cDec = 0;
      cFill = ' ';
    
      // Presets the template string to lowercase
      pcHex = g_pcHex_L;
          
again:

      // Determine how to handle the next character.
      switch(*pcString++)
      {
        // Handle the digit characters.
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
          // If this is a zero, and it is the first digit, then the
          // fill character is a zero instead of a space.
          if((pcString[-1] == '0') && (ulCount == 0))
          {
            cFill = '0';
          }

          // See if we're after the decimal point
          if(cDec)
          {
            // Update the digit count
            // Can only print one decimal digit worth of precision
            ulDecCount = pcString[-1] - '0';
          }
          else
          {
            // Update the digit count.
            ulCount *= 10;
            ulCount += pcString[-1] - '0';
          }

          // Get the next character.
          goto again;
        }
                
        // Handle the . character
        case '.' :
        {
          // Now we're looking at precision value
          cDec = 1;
          
          // Get the next character.
          goto again;
        }

        // Handle the %c command.
        case 'c':
        {
          // Get the value from the varargs.
          ulValue = va_arg(vaArgP, unsigned long);

          // Print out the character.
          str = StrWrite(str, (char *)&ulValue, 1);

          // This command has been handled.
          break;
        }

        // Handle the %d and %i commands.
        case 'd':
        case 'i':
        {
          // Get the value from the varargs.
          ulValue = va_arg(vaArgP, unsigned long);
        
          // If the value is negative, make it positive and indicate
          // that a minus sign is needed.
          if((long)ulValue < 0)
          {
            // Make the value positive.
            ulValue = -(long)ulValue;

            // Indicate that the value is negative.
            cNeg = 1;
          }
          else
          {
            // Indicate that the value is positive so that a minus
            // sign isn't inserted.
            cNeg = 0;
          }

          // Convert the value to ASCII.
          str = StrConvert(str, ulValue, ulCount, pcHex, cNeg, cFill, 10);
          break;
        }
                
        // Handle the %o command.
        case 'o':
        {
          // Get the value from the varargs.
          ulValue = va_arg(vaArgP, unsigned long);

          // If the value is negative, make it positive and indicate
          // that a minus sign is needed.
          if((long)ulValue < 0)
          {
              // Make the value positive.
              ulValue = -(long)ulValue;

              // Indicate that the value is negative.
              cNeg = 1;
          }
          else
          {
              // Indicate that the value is positive so that a minus
              // sign isn't inserted.
              cNeg = 0;
          }

          // Convert the value to ASCII.
          str = StrConvert(str, ulValue, ulCount, pcHex, cNeg, cFill, 8);
          break;
      }
      
      // Handle the %s command.
      case 's':
      {
          // Get the string pointer from the varargs.
          pcStr = va_arg(vaArgP, char *);

          // Determine the length of the string.
          for(ulIdx = 0; pcStr[ulIdx] != '\0'; ulIdx++)
          {
          }

          // Write the string.
          str = StrWrite(str, pcStr, ulIdx);

          // Write any required padding spaces
          if(ulCount > ulIdx)
          {
            ulCount -= ulIdx;
            while(ulCount--)
            {
              str = StrWrite(str, " ", 1);
            }
          }
          // This command has been handled.
          break;
        }

        // Handle the %u command.
        case 'u':
        {
          // Get the value from the varargs.
          ulValue = va_arg(vaArgP, unsigned long);

          // Indicate that the value is positive so that a minus sign
          // isn't inserted.
          cNeg = 0;

          // Convert the value to ASCII.
          str = StrConvert(str, ulValue, ulCount, pcHex, cNeg, cFill, 10);
          break;
        }

        // Handle the %x and %X commands.  We alias %p to %x.
        case 'X':
          // Make the template string uppercase
          pcHex = g_pcHex_U;
        case 'x':
        case 'p':
        {
          // Get the value from the varargs.
          ulValue = va_arg(vaArgP, unsigned long);

          // Indicate that the value is positive so that a minus sign
          // isn't inserted.
          cNeg = 0;
        
          // Convert the value to ASCII.
          str = StrConvert(str, ulValue, ulCount, pcHex, cNeg, cFill, 16);
          break;
        }

        // Handle the %f and %F commands.
        case 'F': // Not different
        case 'f':
        {
          // Declare and read a double
          double dValue;
          dValue = va_arg(vaArgP, double);
        
          // Check if the value is negative
          if(dValue < 0)
          {
            cNeg = 1;
            dValue = 0 - dValue;
          }
          else
          {
              cNeg = 0;
          }
                              
          // Check for out of range constants
          if(isnan(dValue))
          {
            str = StrWrite(str, "NaN", 3);
          }
          else if(dValue == INFINITY)
          {
            if(cNeg)
            {
              str = StrWrite(str, "-INF", 4);
            }
            else
            {
              str = StrWrite(str, "INF", 3);
            }
          }
          else
          {
            // Convert the integer value to ASCII.
            str = StrConvert(str, (unsigned long)dValue, ulCount, pcHex, cNeg, cFill, 10);
            // Remove the original integer value and multiply to move decimal places forward
            dValue = (dValue - (float)((unsigned long)dValue));
            // This loop clobbers ulCount, but it gets reset before we need it again
            for(ulCount = 0; ulCount < ulDecCount; ulCount++)
            {
              dValue *= 10;
            }
            str = StrWrite(str, ".", 1);
            str = StrConvert(str, (unsigned long)dValue, ulDecCount, pcHex, 0, '0', 10);
          }
          break;
        }
                
        // %E and %e for scientific notation
        case 'E':
          // Make the template string uppercase
          pcHex = g_pcHex_U;
        case 'e':
        {
          // Declare and read a double
          double dValue, dExp, dTmp;
          dValue = va_arg(vaArgP, double);
        
          // Check if the value is negative
          if(dValue < 0)
          {
            str = StrWrite(str, "-", 1);
            dValue = 0 - dValue;
          }
                              
          // Check for out of range constants
          if(isnan(dValue))
          {
            str = StrWrite(str, "NaN", 3);
          }
          else if(dValue == INFINITY)
          {
            str = StrWrite(str, "INF", 3);
          }
          else 
          {
            // Print the most significant digit
            dExp = log10(dValue);
            if(dExp < 0)
            {
              // Handler for negative exponents
              dTmp = dValue / pow(10, (long) dExp - 1);
              cNeg = 1;
              dExp = 0 - dExp;
            }
            else
            {
              dTmp = dValue / pow(10, (long) dExp);
              cNeg = 0;
            }
            str = StrWrite(str, &pcHex[(int)dTmp], 1);
            str = StrWrite(str, ".", 1);
            
            // Print ulDecCount following digits
            while(ulDecCount --> 0)
            {
                dTmp -= (long) dTmp;
                dTmp *= 10;
                str = StrWrite(str, &pcHex[(int)dTmp], 1);
            }
            
            // Write the exponent
            str = StrWrite(str, &pcHex[14], 1);
            
            str = StrConvert(str, (unsigned long)dExp, 0, pcHex, cNeg, cFill, 10);
          }
          break;
        }
                
        // Handle the %% command.
        case '%':
        {
          // Simply write a single %.
          str = StrWrite(str, pcString - 1, 1);

          // This command has been handled.
          break;
        }

        // Handle all other commands.
        default:
        {
          // Indicate an error.
          str = StrWrite(str, "ERROR", 5);

          // This command has been handled.
          break;
        }
      }
    }
  }
}

// We don't have strnlen, strcpy, or strcmp so we need to implement them here
// Use uppercase first letter to avoid naming collision
unsigned int Strnlen(char *str, unsigned int max_len) {
    int i = 0;

    while (str[i] && i < max_len) {
        i++;
    }

    return i;
}

char* Strcpy (char *dst, const char *src) {
    int i = 0;
    
    do {
        dst[i] = src[i];
        i += 1;
    } while (src[i]);
    
    return dst;
}

// Returns an integral value indicating the relationship between the strings:
// A zero value indicates that both strings are equal.
// A value greater than zero indicates that the first character that does not match has a greater value in str1 than in str2; 
// And a value less than zero indicates the opposite.
// int strcmp ( const char * str1, const char * str2 );
int Strcmp(const char *str1, const char *str2) {
    int i = 0;

    while (str1[i] && str2[i]) {
        if (str1[i] < str2[i]) {
            return -1;
        } else if (str1[i] > str2[i]) {
            return 1;
        }
        
        i++;
    }
    
    return 0;
}
