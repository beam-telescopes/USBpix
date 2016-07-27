#include "DBInquire.h"
#include "DumbDB.h"
#include <fstream>
#include <iostream>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

using namespace pixlib;
using namespace std;
using namespace XERCES_CPP_NAMESPACE;


DumbDB::DumbDB(string namefile) : datafile(namefile.c_str(),std::ios_base::in), doc(0) {
	if(!datafile.good()) 
		throw exception("infile not good");

	try {
            XMLPlatformUtils::Initialize();
        }
    catch (const XMLException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            cout << "Error during initialization! :\n"
                 << message << "\n";
            XMLString::release(&message);
            throw exception("DumbDB: error during XMLPlatformUtils::Initialize()");
        }

	XMLCh tempStr[100];
    XMLString::transcode("LS", tempStr, 99);
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(tempStr);
    DOMBuilder* parser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);

        // optionally you can set some features on this builder
        if (parser->canSetFeature(XMLUni::fgDOMValidation, true))
            parser->setFeature(XMLUni::fgDOMValidation, true);
        if (parser->canSetFeature(XMLUni::fgDOMNamespaces, true))
            parser->setFeature(XMLUni::fgDOMNamespaces, true);
        if (parser->canSetFeature(XMLUni::fgDOMDatatypeNormalization, true))
            parser->setFeature(XMLUni::fgDOMDatatypeNormalization, true);

        try {
            doc = parser->parseURI(namefile.c_str());
        }
        catch (const XMLException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            cout << "Exception message is: \n"
                 << message << "\n";
            XMLString::release(&message);
			parser->release();
            throw exception("DumbDB: error during XML parsing");
        }
        catch (const DOMException& toCatch) {
            char* message = XMLString::transcode(toCatch.msg);
            cout << "Exception message is: \n"
                 << message << "\n";
            XMLString::release(&message);
			parser->release();
            throw exception("DumbDB: error during DOM building");
        }
        catch (...) {
            cout << "Unexpected Exception \n" ;
			parser->release();
            throw exception("DumbDB: unexpected exception");
        }
		cout << doc->getDocumentElement()->getTypeInfo()->getName();
//		parser->release();
		delete parser;
}

DBInquire* DumbDB::readRootRecord (int iteration){
	return 0;
}

