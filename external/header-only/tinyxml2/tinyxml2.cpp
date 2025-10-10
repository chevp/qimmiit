/*
TinyXML2 - Minimal Implementation for Build Compatibility
This is a stub for demo purposes.
In a real project, download the actual tinyxml2.cpp from:
https://github.com/leethomason/tinyxml2
*/

#include "tinyxml2.h"

namespace tinyxml2 {

    const XMLElement* XMLNode::NextSiblingElement(const char* name) const {
        // Stub implementation
        return nullptr;
    }

    XMLElement* XMLNode::NextSiblingElement(const char* name) {
        // Stub implementation
        return nullptr;
    }

    const char* XMLElement::Attribute(const char* name, const char* defaultValue) const {
        // Stub implementation
        return defaultValue;
    }

    const char* XMLElement::GetText() const {
        // Stub implementation
        return "";
    }

    XMLDocument::XMLDocument(bool processEntities, Whitespace whitespace) {
        // Stub implementation
    }

    XMLDocument::~XMLDocument() {
        // Stub implementation
    }

    XMLError XMLDocument::Parse(const char* xml, size_t nBytes) {
        // Stub implementation
        return XML_SUCCESS;
    }

    XMLElement* XMLDocument::RootElement() {
        // Stub implementation
        return nullptr;
    }
}