#include "CSVFileLogger.h"

CSVFileLogger::CSVFileLogger(bool enableSerialDebug_)
        :_useSerialDebug(enableSerialDebug_)
{
    /*
     * PARAMETER:
     * ----------
     *  enableSerialDebug_:bool
     *      enable output of status information to serial console
     *  enableTimeStamp_:bool
     *      add a time to each value, since this class has been initialised
     * 
     */
} // end public method -- CSVFileLogger()

/*-----------------------------------------------------------------------------
 BEGIN PUBLIC METHODS
 ----------------------------------------------------------------------------*/
bool
CSVFileLogger::begin(const uint8_t pinChipSelect_)
{
    /* begin initialising the SD card and print information to the serial
     * console about the SD card
     *

     * RETURN:
     * -------
     * true,
     *      if the initialisation of the SD card was successfull, otherwise
     * false

     * SYNTAX:
     * -------
     *  bool myBool = logger.begin();
     */ 
    
    if(!SD.begin(pinChipSelect_))
    {
        if (_useSerialDebug) { Serial.println("ERROR: Card Mount Failed"); }
        return EXIT_FAILURE;
    }

    // create target logging directory 
    uint16_t targetDirectoryNmbr = _getTargetLoggingDirectoryNmbr();
    sprintf(_targetDirName, "/SET%04d", targetDirectoryNmbr);
    SD.mkdir(_targetDirName);
    
    if (!SD.exists(_targetDirName))
    {
        if (_useSerialDebug)
        {
            Serial.printf("ERROR: Creating the target directory %s failed!\n",
                            _targetDirName);
        }
        return EXIT_FAILURE;
    }
    
    if (!_useSerialDebug) { return EXIT_SUCCESS; }
    
    // print card information
    uint8_t cardType = SD.cardType();
    Serial.println("-----------------------------------------");
    Serial.print("INFO: SD Card Type: ");
    if(cardType == CARD_NONE)       { Serial.println("No SD card attached!");
                                        while(1);}
    else if(cardType == CARD_MMC)   { Serial.println("MMC");  }
    else if(cardType == CARD_SD)    { Serial.println("SDSC"); }
    else if(cardType == CARD_SDHC)  { Serial.println("SDHC"); }
    else                            { Serial.println("UNKNOWN"); }
    
    Serial.printf("INFO: SD - Total space: %llu MB\n",
                    SD.totalBytes() / (1024 * 1024));
    Serial.printf("INFO: SD - Used space: %llu MB\n",
                    SD.usedBytes() / (1024 * 1024));
    
    // print logger info
    Serial.print("INFO: time stamped data sets: ");
    if (_useTimeStamp)              { Serial.println("true!"); }
    else                            { Serial.println("false!"); }
    
    // print target directory
    Serial.printf("INFO: target logging directory: %s\n", _targetDirName);
    Serial.println("-----------------------------------------");
    
    return EXIT_SUCCESS;
} // end public method -- begin()

bool
CSVFileLogger::startLogging(void)
{
    /* start logging to a csv file, by creating a new file in the target
     * directory.
     *

     * SYNTAX:
     * -------
     * myLogger.startLogging();
     */
    
    // check if a file is already open
    if (_fileHandle)
    {   
        if (_useSerialDebug)
        {
            Serial.println("WARNING: logging already enabled!");
        }
        return EXIT_SUCCESS;
    }

    // generate the file name
    char fileName[22];
    sprintf(fileName, "%s/%07d.CSV\0", _targetDirName, _fileNameIterator);
    
    // open file and check for success
    _fileHandle = SD.open(fileName, FILE_WRITE);
    
    if (!_fileHandle)
    {
        if (_useSerialDebug)
        {
            Serial.printf("ERROR: Opening file %s for logging failed!\n",
                            fileName);
        }
        return EXIT_FAILURE;
    }
    
    // reset time stamp counter
    if (_useTimeStamp)  { _millisStart = millis(); }
    
    if (_useSerialDebug)
    {
        Serial.printf("INFO: Logging to file %s has startet.\n", fileName);
    }
    
    _isNewLine = true;
    
    return EXIT_SUCCESS;
} // end public method -- startLogging()


bool
CSVFileLogger::endLogging(void)
{
    /* end logging to a csv file by closing the file handler and update
     * the filename iterator.

     * SYNTAX:
     * -------
     * myLogger.endLogging();
     */
    if (!_fileHandle)    { return EXIT_FAILURE; }

    _fileHandle.close();
    
    // print debug information to serial console
    if (_useSerialDebug)
    {
        Serial.printf("INFO: Logging data to file %s/%07d.CSV ended.\n",
                _targetDirName, _fileNameIterator);
    }
    
    _fileNameIterator++;
    
    return EXIT_SUCCESS;
} // end public method -- endLogging()

bool
CSVFileLogger::isLoggingEnabled(void)
{
    /* check if logging is currently enabled or not.
     */
    if (!_fileHandle)   { return false; }
    return true;
}

bool
CSVFileLogger::addLineBreak(void)
{
    /* add a linebreak to the log file
     * 

     * RETURN:
     * -------
     * true / EXIT_SUCCESS,
     *      if appending the linebreak was successfull, otherwise
     * false / EXIT_FAILURE,
     *      if anything went wrong 

     * SYNTAX:
     * -------
     *  bool myBool = logger.addLineBreak();
     */ 
    if (!_fileHandle)       { return EXIT_FAILURE; }

    if (_isTargetWindows)   { _fileHandle.print("\r"); }
    _fileHandle.print("\n");
    _isNewLine = true;

    return EXIT_SUCCESS;
}

bool
CSVFileLogger::addValue(float value_)
{
    /* simple function, which adds a float value to the current csv log file
     
     * RETURN:
     * -------
     * true / EXIT_SUCCESS,
     *      if appending the value was successfull, otherwise
     * false / EXIT_FAILURE,
     *      if anything went wrong

     * SYNTAX:
     * -------
     *  bool myBool = logger.addValue(myFloat);
     */
    if (!_fileHandle)   { return EXIT_FAILURE; }
    if (!_isNewLine)    { _fileHandle.print(", "); }

    _fileHandle.print(value_);
    return EXIT_SUCCESS;
} // end public method -- addValue(float)

bool
CSVFileLogger::addValue(float value_, const uint8_t decimalPlaces_)
{
    /* apped a single float value to the log file and add a linebreak
     
     * PARAMETER:
     * ----------
     * value_:float
     *      value to log
     * decimalPlaces_:uint8_t
     *      number of decimal places of the float value
     
     * RETURN:
     * -------
     * true,
     *      if appending the value was successfull, otherwise
     * false,
     *      if anything went wrong 
     */
    
    if (!_fileHandle)   { return EXIT_FAILURE; }
    if (!_isNewLine)    { _fileHandle.print(", "); }
    
    // convert the float value to a string and append it
    char valueChar[20];
    dtostrf(value_, (4+decimalPlaces_), decimalPlaces_, valueChar);
    _fileHandle.print(String(valueChar));
        
    // revert the new line flag, so that no comma will be added
    _isNewLine = false;
    
    return EXIT_SUCCESS;
} // end public method -- appendValue(float, decimalPlaces)

bool
CSVFileLogger::addText(const char *text_)
{
    /* adding text to the logging file

     * SYNTAX:
     * -------
     *  // saving compile date
     *  bool myBool = logger.addText(__DATE__);
     */

    if (!_fileHandle)   { return EXIT_FAILURE; }
    if (!_isNewLine)    { _fileHandle.print(", "); }

    _fileHandle.print(text_);

    return EXIT_SUCCESS;
} // end public method -- addText(const char*)

bool
CSVFileLogger::addText(char *text_, uint16_t len_)
{
    /* adding text to the logging file

     * SYNTAX:
     * -------
     *  bool myBool = logger.addText(myText, 3);
     */

    if (!_fileHandle)   { return EXIT_FAILURE; }
    if (!_isNewLine)    { _fileHandle.print(", "); }

    for (uint16_t run=0; run<len_; run++)
    {
        _fileHandle.print(text_[run]);
    }

    return EXIT_SUCCESS;
} // end public method -- addText(char* text, uint16_t len)

bool
CSVFileLogger::addText(String text_)
{
    /* add a String to the current csv file.

     * SYNTAX:
     * -------
     *  bool myBool = logger.addText(myString);
     */
    if (!_fileHandle)   { return EXIT_FAILURE; }
    if (!_isNewLine)    { _fileHandle.print(", "); }

    _fileHandle.print(text_);
    return EXIT_SUCCESS;
} // end public method -- addText(String)

/*-----------------------------------------------------------------------------
 BEGIN PRIVATE METHODS
 ----------------------------------------------------------------------------*/
uint16_t
CSVFileLogger::_getTargetLoggingDirectoryNmbr(void)
{
    /* function, which returns the number of the target logging directory.
     
     * PARAMETER:
     * ----------
     *  None
     
     * SYNTAX:
     * -------
     *  uint16_t nmbr = _getTargetLoggingDirectoryCntr();
     */
    int16_t dirCntr = -1;
    int16_t dirCntrOld = 0;
    
    File root = SD.open("/");
     
    if (!root)
    {
         if (_useSerialDebug)
        {
            Serial.println("ERROR: Can not read from SD-Card!");
        }
        return false;
    }
    
    // look for existing directories named SET*number*
    while (true)
    {
        File entry =  root.openNextFile();
        
        // break if entry does not exist or is not a directory
        if (!entry)                 { break; }
        if (!entry.isDirectory())   { continue; }
        
        // check if the current name matches the target logging directory pattern
        char junk;
        if (sscanf(entry.name(), "%*1[/]SET%d", &dirCntrOld) != 1)
        { 
            entry.close();
            continue;
        }

        // update the directory counter
        if (dirCntrOld > dirCntr)   { dirCntr = dirCntrOld; }

        entry.close();
        
    } // end while -- cycle thru whole card
    
    root.close();
    
    // return the found number
    return (dirCntr+1);
} // end private method -- _getTargetLoggingDirectoryNmbr()

