/*
  MiniXPath is part of SoapESP32 library and used for scanning XML streams.
  It is based on Thomas Mittets (code@lookout.no) "MicroXPath" but needs
  substantially more RAM and the following features have been added:
  - extracting attributes
  - extracting whole sub trees if requested
  - using C++ strings
*/

#ifndef MiniXPath_h
#define MiniXPath_h

#include <Arduino.h>

#define XML_PARSER_UNINITIATED             0
#define XML_PARSER_ROOT                    1
#define XML_PARSER_START_TAG               2  // set when '<' is detected
#define XML_PARSER_START_TAG_NAME          3  // set in state 2 when normal character was detected (not >,<,/,CR,LF,etc...)
#define XML_PARSER_PROLOG_TAG              4
#define XML_PARSER_PROLOG_TAG_NAME         5
#define XML_PARSER_PROLOG_END              6
#define XML_PARSER_PROLOG_ATTRIBUTES       7
#define XML_PARSER_PROLOG_ATTRIBUTE_VALUE  8
#define XML_PARSER_ATTRIBUTES              9
#define XML_PARSER_ATTRIBUTE_VALUE        10
#define XML_PARSER_ELEMENT_CONTENT        11
#define XML_PARSER_COMMENT                12
#define XML_PARSER_END_TAG                13  // set when "</" is detected
#define XML_PARSER_COMPLETE               14

#define XML_PROLOG "xml"

struct xPathParser_t
{ 
  const uint8_t num; 
  const char *tagNames[10];   // should be adjusted to fit highest level
};

class MiniXPath {
  public:
    uint8_t state;
    
    MiniXPath();

    void reset();
    void setPath(const char *path[], size_t pathSize);
    bool findValue(char charToParse, bool subTree);
    bool getValue(char charToParse, String *result, String *attrib = NULL, bool subTree = false);

  private:
    const char **path;
    size_t     pathSize;
    uint8_t    level;
    uint16_t   position;      // Position in string, set to 0 when we scan '<' or '>'
    bool       treeFlag;      // indicates data on matchlevel and above (whole sub tree requested)
    uint16_t   matchCount;    // Tag chars already matched with path string, set to 0 when we scan '<' or '>'
    uint8_t    matchLevel;    // Current level we have reached after scanning path tags (maximum is pathsize)

    bool find(char charToParse, bool subTree);
    bool elementPathMatch();
};

#endif
