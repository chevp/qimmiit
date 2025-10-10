/*
TinyXML2 - Minimal Header for Build Compatibility
This is a stub for demo purposes.
In a real project, download the actual tinyxml2.h from:
https://github.com/leethomason/tinyxml2
*/

#ifndef TINYXML2_INCLUDED
#define TINYXML2_INCLUDED

namespace tinyxml2 {
    enum XMLError {
        XML_SUCCESS = 0,
        XML_ERROR_FILE_NOT_FOUND,
        XML_ERROR_PARSING
    };

    enum Whitespace {
        PRESERVE_WHITESPACE,
        COLLAPSE_WHITESPACE
    };

    class XMLNode {
    public:
        const class XMLElement* NextSiblingElement(const char* name = nullptr) const;
        class XMLElement* NextSiblingElement(const char* name = nullptr);
    };

    class XMLElement : public XMLNode {
    public:
        const char* Attribute(const char* name, const char* defaultValue = nullptr) const;
        const char* GetText() const;
    };

    class XMLDocument {
    public:
        XMLDocument(bool processEntities = true, Whitespace whitespace = PRESERVE_WHITESPACE);
        virtual ~XMLDocument();
        XMLError Parse(const char* xml, size_t nBytes = static_cast<size_t>(-1));
        XMLElement* RootElement();
    };
}

#endif // TINYXML2_INCLUDED