/*
  MiniXPath is part of SoapESP32 library and used for scanning XML streams.
  It is based on Thomas Mittets (code@lookout.no) "MicroXPath" but needs
  substantially more RAM and the following features have been added:
  - extracting attributes
  - extracting whole sub trees if requested
  - using C++ strings
*/

#include "MiniXPath.h"

MiniXPath::MiniXPath() {
	this->pathSize = 0;
	reset();
}

MiniXPath::~MiniXPath() { ; }

void MiniXPath::reset() {
	state = XML_PARSER_UNINITIATED;
	level = 0;
	position = 0;
	treeFlag = false;
	matchCount = 0;
	matchLevel = 0;  // Current level we have reached after scanning path tags
#ifdef MINIXPATH_DEBUG
	Serial.printf("reset: pathsiz=%d stat=%02d lev=%d matchcnt=%d matchlev=%d path=%s\n", pathSize, state, level,
				  matchCount, matchLevel, pathSize == 0 ? "" : path[pathSize - 1]);
	Serial.flush();
#endif
}

void MiniXPath::setPath(const char *path[], size_t pathSize) {
	uint8_t newMatchLevel = 0;
	for(uint8_t i = 0; i < pathSize && i < this->pathSize && i < matchLevel && i == newMatchLevel; i++) {
		if(strcmp(path[i], this->path[i]) == 0) newMatchLevel++;
	}
	this->matchCount = 0;
	this->matchLevel = newMatchLevel;
	this->path = path;
	this->pathSize = pathSize;
#ifdef MINIXPATH_DEBUG
	Serial.printf("setPath: pathsiz=%d stat=%02d lev=%d matchcnt=%d matchlev=%d path=%s\n", pathSize, state, level,
				  matchCount, matchLevel, pathSize == 0 ? "" : path[pathSize - 1]);
	Serial.flush();
#endif
}

bool MiniXPath::findValue(char charToParse, bool subTree) {
	return find(charToParse, subTree) && state == XML_PARSER_ELEMENT_CONTENT;
}

bool MiniXPath::getValue(char charToParse, String *result, String *attrib, bool subTree) {
	if(find(charToParse, subTree)) {
		if(subTree) {
			if(treeFlag) {
				*result += charToParse;
				if(state == XML_PARSER_END_TAG && level == matchLevel) {
					if(result->endsWith("</")) result->remove(result->length() - 2);
					result->trim();
					return true;
				}
			}
		}
		// Ignore sub elements
		else if(level == matchLevel) {
			if(state == XML_PARSER_ELEMENT_CONTENT) {
				// "position > 0" means that we skip the tag-end character and any trailing whitespace
				if(position > 0) { *result += charToParse; }
			}
			else if(state == XML_PARSER_END_TAG && position == 0) {
				result->trim();  // Remove trailing whitespace
				return true;
			}
		}
		// Getting attribute data
		if(attrib != NULL && position > 0 && matchLevel > 0 && level == matchLevel - 1 &&
		   (state == XML_PARSER_ATTRIBUTES || state == XML_PARSER_ATTRIBUTE_VALUE)) {
			if(charToParse == '\t' || charToParse == '\r' || charToParse == '\n') { *attrib += ' '; }
			else { *attrib += charToParse; }
		}
	}
	else {
		if(level == matchLevel && state == XML_PARSER_START_TAG && position == 0) {
			// Make sure we start clean
			if(attrib != NULL) *attrib = "";
			*result = "";
		}
	}
	return false;
}

bool MiniXPath::find(char charToParse, bool subTree) {
	// Start parsing when the first "<" character is stumbled upon
	if(state == XML_PARSER_UNINITIATED && charToParse == '<') state = XML_PARSER_ROOT;
	if(state >= XML_PARSER_COMPLETE && charToParse > ' ') {}
	else if(state > XML_PARSER_UNINITIATED && state < XML_PARSER_COMPLETE) {
		switch(charToParse) {
			// Tag start
			case '<':
				if(level < pathSize) treeFlag = false;
				else if(subTree)
					treeFlag = true;
				if(state == XML_PARSER_ROOT || state == XML_PARSER_ELEMENT_CONTENT) { state = XML_PARSER_START_TAG; }
				position = 0;
				matchCount = 0;
				break;
			// Tag end
			case '>':
				if(level < pathSize) treeFlag = false;
				if(state == XML_PARSER_START_TAG_NAME || state == XML_PARSER_ATTRIBUTES) {
					if(elementPathMatch()) matchLevel++;
					level++;
					state = XML_PARSER_ELEMENT_CONTENT;
				}
				else if(state == XML_PARSER_END_TAG) {
					if(level == matchLevel) matchLevel--;
					level--;
					if(level > 0) { state = XML_PARSER_ELEMENT_CONTENT; }
					else {
						// state = XML_PARSER_COMPLETE;
						state = XML_PARSER_ROOT;
					}
				}
				else if(level == 0 && state == XML_PARSER_PROLOG_END) { state = XML_PARSER_ROOT; }
				else if(state == XML_PARSER_COMMENT) {
					state = (level == 0) ? XML_PARSER_ROOT : XML_PARSER_ELEMENT_CONTENT;
				}
				position = 0;
				matchCount = 0;
				break;
			// Prolog start and end character
			case '?':
				if(level < pathSize) treeFlag = false;
				if(state == XML_PARSER_START_TAG && level == 0) { state = XML_PARSER_PROLOG_TAG; }
				else if(state == XML_PARSER_PROLOG_TAG_NAME || state == XML_PARSER_PROLOG_ATTRIBUTES) {
					state = XML_PARSER_PROLOG_END;
				}
				break;
			// Comment start character
			case '!':
				// if (level < pathSize) treeFlag = false;
				if(state == XML_PARSER_START_TAG) { state = XML_PARSER_COMMENT; }
				break;
			// Whitespace
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				if(level < pathSize) treeFlag = false;
				switch(state) {
					case XML_PARSER_START_TAG_NAME:
						if(elementPathMatch()) matchLevel++;
						state = XML_PARSER_ATTRIBUTES;
						position = 0;
						break;
					case XML_PARSER_PROLOG_TAG_NAME:
						state = XML_PARSER_PROLOG_ATTRIBUTES;
						break;
					case XML_PARSER_ATTRIBUTES:
					case XML_PARSER_ATTRIBUTE_VALUE:
						position++;
						break;
				}
				break;
			// Attribute start character and end character
			case '"':
			case '\'':
				if(level < pathSize) treeFlag = false;
				position++;
				switch(state) {
					case XML_PARSER_ATTRIBUTES:
						state = XML_PARSER_ATTRIBUTE_VALUE;
						position++;
						break;
					case XML_PARSER_ATTRIBUTE_VALUE:
						state = XML_PARSER_ATTRIBUTES;
						position++;
						break;
				}
				break;
			// End tag character
			case '/':
				if(state == XML_PARSER_START_TAG) { state = XML_PARSER_END_TAG; }
				else if(state == XML_PARSER_START_TAG_NAME || state == XML_PARSER_ATTRIBUTES) {
					level++;
					state = XML_PARSER_END_TAG;
				}
				break;
			// All other characters
			default:
				if(level < pathSize || (subTree && state == XML_PARSER_END_TAG && level == matchLevel)) {
					treeFlag = false;
				}
				if(state == XML_PARSER_START_TAG) state = XML_PARSER_START_TAG_NAME;
				if(state == XML_PARSER_PROLOG_TAG) state = XML_PARSER_PROLOG_TAG_NAME;
				if(state == XML_PARSER_START_TAG_NAME) {
					if(level == matchLevel && matchCount == position && level < pathSize &&
					   position < strlen(path[level]) && charToParse == path[level][position]) {
						matchCount++;
					}
				}
				position++;
				break;
		}
	}
#ifdef MINIXPATH_DEBUG
	// TEST
	// if (pathSize == 1 && (*(path[0]) == 'c' || *(path[0]) == 'i')) {
	char charTP = (charToParse < 0x20) ? '.' : charToParse;
	Serial.printf("'%c' pathsiz=%d stat=%02d lev=%d matchcnt=%d matchlev=%d pos=%d treeFlg=%d ret=%d\n", charTP,
				  pathSize, state, level, matchCount, matchLevel, position, treeFlag, matchLevel == pathSize);
	//}
#endif
	return matchLevel == pathSize;
}

bool MiniXPath::elementPathMatch() {
	return level == matchLevel &&
		   // position == matchCount &&
		   level < pathSize && matchCount == strlen(path[level]);
}
