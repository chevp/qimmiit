/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <tinyxml2.h>
#include <windows.h>
#include <basetsd.h>
#include <coregfx/rsc/rsc_pocean.hpp>

namespace xmlpocean {

	class TinyPOCEAN {
        tinyxml2::XMLDocument doc;

	public:
		TinyPOCEAN() {}

        tinyxml2::XMLError loadFromXmlFile(rscpocean::Model* model, std::string* err, const std::string& filename)
        {
            auto error = doc.LoadFile(filename.c_str());

            if (error != tinyxml2::XMLError::XML_SUCCESS)
                return error;

            return loadFromXmlDocument(model, err);
        }

        tinyxml2::XMLError loadFromXmlString(rscpocean::Model* model, std::string* err, const char* xml)
        {
            auto error = doc.Parse(xml);

            if (error != tinyxml2::XMLError::XML_SUCCESS)
                return error;

            return loadFromXmlDocument(model, err);
        }

        tinyxml2::XMLError testScneario1() {
            static const char* xml =
                "<pocean version=\"1\" creator=\"chevp\">"
                    "<head>"
                        "<title>Scene1</title>"
                        "<mode>editor</mode>"
                        "<author>chevp</author>"
                        "<uuid>b4da375b-dbe0-4a1b-ba53-418a67868650</uuid>"
                    "</head>"
                "</pocean>";
            
            auto model = rscpocean::Model();
            std::string error;
            auto result = loadFromXmlString(&model, &error, xml);

            return result;
        }
        tinyxml2::XMLError testScneario2() {
            static const char* xml =
                "<pocean version='1' creator='chevp'>"
                    "<head>"
                        "<title>Scene1</title>"
                        "<mode>editor</mode>"
                        "<author>chevp</author>"
                        "<uuid>b4da375b-dbe0-4a1b-ba53-418a67868650</uuid>"
                    "</head>"
                "</pocean>";

            auto model = rscpocean::Model();
            std::string error;
            auto result = loadFromXmlString(&model, &error, xml);

            return result;
        }
    private:
        /**
        * doc wurde geladen und wird umgewandelt zu Model 
        */
        tinyxml2::XMLError loadFromXmlDocument(rscpocean::Model* model, std::string* err) {
        
            tinyxml2::XMLError validationResult = validateDoc();

            if (validationResult != tinyxml2::XMLError::XML_SUCCESS)
                return validationResult;

            tinyxml2::XMLError interpretResult = interpretXmlDocument(model, err);

            return interpretResult;
        }
        /**
        * Converts hex-string to uint32
        */
        uint32_t toUint32(const char* hex) {
            std::istringstream converter(hex);
            unsigned int value;
            converter >> std::hex >> value;
            return value;
        }

        tinyxml2::XMLError interpretXmlDocument(rscpocean::Model* model, std::string* err)
        {
            tinyxml2::XMLElement* eRoot = doc.FirstChildElement();

            if (eRoot == nullptr) return tinyxml2::XMLError::XML_ERROR_PARSING_ELEMENT;

            // head
            {
                model->head = rscpocean::Head();

                tinyxml2::XMLElement* elHead = eRoot->FirstChildElement("head");
                if (elHead == nullptr) return tinyxml2::XMLError::XML_ERROR_PARSING_ELEMENT;

                tinyxml2::XMLElement* eTitle = elHead->FirstChildElement("title");
                if (eTitle == nullptr) return tinyxml2::XMLError::XML_ERROR_PARSING_ELEMENT;
                model->head.title = eTitle->GetText();

                tinyxml2::XMLElement* eMode = elHead->FirstChildElement("mode");
                if (eMode == nullptr) return tinyxml2::XMLError::XML_ERROR_PARSING_ELEMENT;
                model->head.mode = eMode->GetText();
            }

            return tinyxml2::XMLError::XML_SUCCESS;
        }

        tinyxml2::XMLError validateDoc()
        {
            // 1. Pr�ft, dass das Root-Object wie folgt aussieht:
            // <pocean version="1" creator="chevp">
            tinyxml2::XMLElement* eRoot = doc.FirstChildElement();

            if (eRoot == nullptr) return tinyxml2::XMLError::XML_ERROR_PARSING_ELEMENT;

            int version;

            if (eRoot->QueryIntAttribute("version", &version) != tinyxml2::XMLError::XML_SUCCESS)
                return tinyxml2::XMLError::XML_ERROR_PARSING_ELEMENT;;

            if (version > 1)
                return tinyxml2::XMLError::XML_ERROR_PARSING_ELEMENT;;

            const char* creator;

            if (eRoot->QueryStringAttribute("creator", &creator) != tinyxml2::XMLError::XML_SUCCESS)
                return tinyxml2::XMLError::XML_ERROR_PARSING_ELEMENT;;

            if (_strcmpi(creator, "chevp") != 0)
                return tinyxml2::XMLError::XML_ERROR_PARSING_ELEMENT;;

            return tinyxml2::XMLError::XML_SUCCESS;
        }
	};
}