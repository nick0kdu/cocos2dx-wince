/****************************************************************************
 Copyright (c) 2010 cocos2d-x.org  http://cocos2d-x.org
 Copyright (c) 2010 Максим Аксенов
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "CCSAXParser.h"
#include "CCMutableDictionary.h"
//#include <libxml/parser.h>
//#include <libxml/tree.h>
//#include <libxml/xmlmemory.h>
//#include "CCLibxml2.h"
#include "CCFileUtils.h"
#include "support/tinyxml2/tinyxml2.h"

NS_CC_BEGIN;

class XmlSaxHander : public tinyxml2::XMLVisitor
{
public:
	XmlSaxHander():m_ccsaxParserImp(0){};

	virtual bool VisitEnter( const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute );
	virtual bool VisitExit( const tinyxml2::XMLElement& element );
	virtual bool Visit( const tinyxml2::XMLText& text );
	virtual bool Visit( const tinyxml2::XMLUnknown&){ return true; }

	void setCCSAXParserImp(CCSAXParser* parser)
	{
		m_ccsaxParserImp = parser;
	}

private:
	CCSAXParser *m_ccsaxParserImp;
};


bool XmlSaxHander::VisitEnter( const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute )
{
	//CCLog(" VisitEnter %s",element.Value());

	std::vector<const char*> attsVector;
	for( const tinyxml2::XMLAttribute* attrib = firstAttribute; attrib; attrib = attrib->Next() )
	{
		//CCLog("%s", attrib->Name());
		attsVector.push_back(attrib->Name());
		//CCLog("%s",attrib->Value());
		attsVector.push_back(attrib->Value());
	}

	// nullptr is used in c++11
	//attsVector.push_back(nullptr);
	attsVector.push_back(NULL);

	CCSAXParser::startElement(m_ccsaxParserImp, (const CC_XML_CHAR *)element.Value(), (const CC_XML_CHAR **)(&attsVector[0]));
	return true;
}
bool XmlSaxHander::VisitExit( const tinyxml2::XMLElement& element )
{
	//CCLog("VisitExit %s",element.Value());

	CCSAXParser::endElement(m_ccsaxParserImp, (const CC_XML_CHAR *)element.Value());
	return true;
}

bool XmlSaxHander::Visit( const tinyxml2::XMLText& text )
{
	//CCLog("Visit %s",text.Value());
	CCSAXParser::textHandler(m_ccsaxParserImp, (const CC_XML_CHAR *)text.Value(), strlen(text.Value()));
	return true;
}

CCSAXParser::CCSAXParser()
{
	m_pDelegator = NULL;

}

CCSAXParser::~CCSAXParser(void)
{
}

bool CCSAXParser::init(const char *pszEncoding)
{
    CC_UNUSED_PARAM(pszEncoding);
	// nothing to do
	return true;
}

bool CCSAXParser::parse(const char *pszFile)
{
	CCFileData data(pszFile, "rt");
	unsigned long size = data.getSize();
	char *pBuffer = (char*) data.getBuffer();
	
	if (!pBuffer)
	{
		return false;
	}

	tinyxml2::XMLDocument tinyDoc;
	tinyDoc.Parse(pBuffer);
	XmlSaxHander printer;
	printer.setCCSAXParserImp(this);

	return tinyDoc.Accept( &printer );
		
	/*
	 * this initialize the library and check potential ABI mismatches
	 * between the version it was compiled for and the actual shared
	 * library used.
	 */
//	LIBXML_TEST_VERSION
//	xmlSAXHandler saxHandler;
//	memset( &saxHandler, 0, sizeof(saxHandler) );
//	// Using xmlSAXVersion( &saxHandler, 2 ) generate crash as it sets plenty of other pointers...
//	saxHandler.initialized = XML_SAX2_MAGIC;  // so we do this to force parsing as SAX2.
//	saxHandler.startElement = &CCSAXParser::startElement;
//	saxHandler.endElement = &CCSAXParser::endElement;
//	saxHandler.characters = &CCSAXParser::textHandler;
//	
//	int result = xmlSAXUserParseMemory( &saxHandler, this, pBuffer, size );
//	if ( result != 0 )
//	{
//		return false;
//	}
//	/*
//	 * Cleanup function for the XML library.
//	 */
//	xmlCleanupParser();
//	/*
//	 * this is to debug memory for regression tests
//	 */
//#if (CC_TARGET_PLATFORM != CC_PLATFORM_BADA)
//	xmlMemoryDump();
//#endif
//	
//	return true;
}

void CCSAXParser::startElement(void *ctx, const CC_XML_CHAR *name, const CC_XML_CHAR **atts)
{
	((CCSAXParser*)(ctx))->m_pDelegator->startElement(ctx, (char*)name, (const char**)atts);
}

void CCSAXParser::endElement(void *ctx, const CC_XML_CHAR *name)
{
	((CCSAXParser*)(ctx))->m_pDelegator->endElement(ctx, (char*)name);
}
void CCSAXParser::textHandler(void *ctx, const CC_XML_CHAR *name, int len)
{
	((CCSAXParser*)(ctx))->m_pDelegator->textHandler(ctx, (char*)name, len);
}
void CCSAXParser::setDelegator(CCSAXDelegator* pDelegator)
{
	m_pDelegator = pDelegator;
}

NS_CC_END;


