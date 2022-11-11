#ifndef ARDUINO
#include "Arduino.h"
#endif ARDUINO

#ifndef CSVFILELOGGER
#define CSVFILELOGGER

#include "SD.h"
#include "SPI.h"

/* class which stores measurement values in an CSV file and ensures file
 * handling such as file open or close.
 
 * BEHAVIOR FILE I/O:
 * ------------------
 * - on each initialisation, the class creates a new directory, where a all
 *   log files will be stored in.
 * - starting a new logging cycle creates a new file in the currently active
 *   directory
 * - ending a logging cycle closes the file
 
 * BEHAVIOR NAMES:
 * ---------------
 * -- target logging directories
 *  - the target directory is named "SET*Number*"
 *  - the number increases on each initialisation
 * -- logging file 
 *  - when logging ist startet, the name will be *number*.csv
 *  - the number although increases for new log startet
 
 */

class CSVFileLogger
{
    private:
        
        const bool _useSerialDebug  = true; // enable serial output for debug information
        const bool _isTargetWindows = false;// set the linebreak character
        
        char _targetDirName[9];             // name of the logging directory
        
        // flag indicating, that the cursor has moved to a new line
        bool _isNewLine = true;
        
        // iterator from which the next file name will be created
        uint16_t _fileNameIterator = 1;     // 
        File _fileHandle;                   // file handle
    //
    // end private properties
    
    public:
        CSVFileLogger(bool  enableSerialDebug_ = true);
    
        bool begin(const uint8_t pinChipSelect_ = 5);
    
        // file operation and status methods
        bool startLogging(void);
        bool endLogging(void);
        bool isLoggingEnabled(void);

        // append value section
        bool addLineBreak(void);
        bool addValue(float value_);
        bool addValue(float value_, const uint8_t decimalPlaces_ = 2);

        // append text
        bool addText(const char *text_);
        bool addText(char *text_, uint16_t len_);
        bool addText(String text_);
    //
    // end public methods
    
    private:
        uint16_t _getTargetLoggingDirectoryNmbr(void);
    //
    // end private methods
        
}; // end class -- CSVFileLogger
#endif CSVFILELOGGER
