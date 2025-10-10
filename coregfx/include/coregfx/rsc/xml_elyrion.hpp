/**
 * Copyright (C) by Patrice Chevillat
 */
#pragma once
#include <string>
#include <tinyxml2.h>
#include <vector>
#include <unordered_set>
#include "nyx1.grpc.pb.h"
#include <coregfx/core/ocean_log.hpp>
#include <fstream>
#include <iostream>
#include "../../../adapters/curl_stubs.h"

namespace xmlelyrion::ElyrionLoader {

    namespace detail {
        inline std::string getAttribute(tinyxml2::XMLElement* elem, const std::string& attr) {
            const char* value = elem ? elem->Attribute(attr.c_str()) : nullptr;
            return value ? value : "";
        }

        inline std::vector<tinyxml2::XMLElement*> getChildren(tinyxml2::XMLElement* parent, const std::string& tag) {
            std::vector<tinyxml2::XMLElement*> result;
            if (!parent) return result;
            for (auto* elem = parent->FirstChildElement(tag.c_str()); elem; elem = elem->NextSiblingElement(tag.c_str()))
                result.push_back(elem);
            return result;
        }

        inline tinyxml2::XMLElement* findChild(tinyxml2::XMLElement* parent, const std::string& tag) {
            return parent ? parent->FirstChildElement(tag.c_str()) : nullptr;
        }
    }

    namespace {
            void static parseScenes(tinyxml2::XMLElement* eRoot, nyx::Elyrion& model) {
                if (auto* eScenes = detail::findChild(eRoot, "scenes")) {
                    for (auto* eScene : detail::getChildren(eScenes, "scene")) {
                        if (!eScene) continue;

                        nyx::ElyScene* scene = model.add_scenes();

                        {
                            std::string id = detail::getAttribute(eScene, "id");
                            if (!id.empty()) scene->set_id(id.c_str());

                            std::string name = detail::getAttribute(eScene, "name");
                            if (!name.empty()) scene->set_name(name.c_str());
                        }

                        // entities
                        if (auto* eEntities = detail::findChild(eScene, "entities")) {
                            for (auto* eEntity : detail::getChildren(eEntities, "entity")) {
                                if (!eEntity) continue;

                                nyx::ElyEntity* entity = scene->add_entities();

                                std::string id = detail::getAttribute(eEntity, "id");
                                if (!id.empty()) entity->set_id(id.c_str());

                                std::string type = detail::getAttribute(eEntity, "type");
                                if (!type.empty()) entity->set_type(type.c_str());

                                std::string asset = detail::getAttribute(eEntity, "asset");
                                if (!asset.empty()) entity->set_asset(asset.c_str());
                            }
                        }

                        // instructions
                        if (auto* eInstructions = detail::findChild(eScene, "instructions")) {
                            for (auto* eInstr : detail::getChildren(eInstructions, "instruction")) {
                                if (!eInstr) continue;

                                nyx::ElyInstruction* instr = scene->add_instructions();

                                std::string action = detail::getAttribute(eInstr, "action");
                                if (!action.empty()) instr->set_action(action.c_str());

                                std::string trigger = detail::getAttribute(eInstr, "trigger");
                                if (!trigger.empty()) instr->set_trigger(trigger.c_str());

                                for (auto* param : detail::getChildren(eInstr, "param")) {
                                    if (!param) continue;

                                    nyx::ElyParam* p = instr->add_param();

                                    std::string name = detail::getAttribute(param, "name");
                                    if (!name.empty()) p->set_name(name.c_str());

                                    std::string value = detail::getAttribute(param, "value");
                                    if (!value.empty()) p->set_value(value.c_str());
                                }
                            }
                        }
                    }
                }
            }
            void static parseAssets(tinyxml2::XMLElement* eRoot, nyx::Elyrion& model) {
                std::unordered_set<std::string> assetIds;

                if (auto* eAssets = detail::findChild(eRoot, "assets")) {
                    for (auto* eAsset : detail::getChildren(eAssets, "asset")) {
                        if (!eAsset) continue;

                        std::string id = detail::getAttribute(eAsset, "id");

                        // Check for duplicate asset IDs
                        if (!id.empty()) {
                            if (assetIds.find(id) != assetIds.end()) {
                                ocean::error("CRITICAL: Duplicate asset ID '" + id + "' found in index.elyrion.xml - this will cause undefined behavior!");
                                ocean::error("Each asset must have a unique ID. Please fix the XML file and restart.");
                                throw std::runtime_error("Duplicate asset ID: " + id);
                            }
                            assetIds.insert(id);
                        }

                        nyx::ElyAsset* asset = model.add_assets();

                        std::string type = detail::getAttribute(eAsset, "type");
                        std::string version = detail::getAttribute(eAsset, "version");
                        std::string src = detail::getAttribute(eAsset, "src");
                        std::string pipeline = detail::getAttribute(eAsset, "pipeline");

                        if (!id.empty())       asset->set_id(id.c_str());
                        if (!type.empty())     asset->set_type(type.c_str());
                        if (!version.empty())  asset->set_version(version.c_str());
                        if (!src.empty())      asset->set_src(src.c_str());
                        if (!pipeline.empty()) asset->set_pipeline(pipeline.c_str());

                        if (auto* eMeta = detail::findChild(eAsset, "meta")) {
                            nyx::ElyMeta* meta = asset->mutable_meta();

                            std::string metaType = detail::getAttribute(eMeta, "type");
                            if (!metaType.empty()) meta->set_type(metaType.c_str());

                            std::string sizeStr = detail::getAttribute(eMeta, "size");
                            if (!sizeStr.empty()) {
                                try {
                                    meta->set_size(std::stoll(sizeStr));
                                }
                                catch (...) {
                                    meta->set_size(0);
                                }
                            }

                            std::string sha = detail::getAttribute(eMeta, "sha256");
                            if (!sha.empty()) meta->set_sha256(sha.c_str());

                            for (auto* param : detail::getChildren(eMeta, "param")) {
                                if (!param) continue;

                                nyx::ElyParam* p = meta->add_param();

                                std::string name = detail::getAttribute(param, "name");
                                std::string value = detail::getAttribute(param, "value");

                                if (!name.empty())  p->set_name(name.c_str());
                                if (!value.empty()) p->set_value(value.c_str());
                            }
                        }
                    }
                }
            }
            void static parseStreams(tinyxml2::XMLElement* eRoot, nyx::Elyrion& model) {
                if (auto* eStreams = detail::findChild(eRoot, "streams")) {
                    for (auto* eStream : detail::getChildren(eStreams, "stream")) {
                        if (!eStream) continue;

                        nyx::ElyStream* stream = model.add_streams();

                        std::string id = detail::getAttribute(eStream, "id");
                        std::string type = detail::getAttribute(eStream, "type");
                        std::string rate = detail::getAttribute(eStream, "rate");
                        std::string prio = detail::getAttribute(eStream, "prio");
                        std::string priority = detail::getAttribute(eStream, "priority");
                        std::string src = detail::getAttribute(eStream, "src");
                        std::string trigger = detail::getAttribute(eStream, "trigger");
                        std::string onceVal = detail::getAttribute(eStream, "once");

                        if (!id.empty())       stream->set_id(id.c_str());
                        if (!type.empty())     stream->set_type(type.c_str());
                        if (!rate.empty())     stream->set_rate(rate.c_str());
                        if (!prio.empty())     stream->set_prio(prio.c_str());
                        if (!priority.empty()) stream->set_priority(priority.c_str());
                        if (!src.empty())      stream->set_src(src.c_str());
                        if (!trigger.empty())  stream->set_trigger(trigger.c_str());

                        if (!onceVal.empty())
                            stream->set_once(onceVal == "true" || onceVal == "1");

                        for (auto* eInstr : detail::getChildren(eStream, "instruction")) {
                            if (!eInstr) continue;

                            nyx::ElyInstruction* instr = stream->add_instruction();

                            std::string action = detail::getAttribute(eInstr, "action");
                            std::string trigger = detail::getAttribute(eInstr, "trigger");

                            if (!action.empty())  instr->set_action(action.c_str());
                            if (!trigger.empty()) instr->set_trigger(trigger.c_str());
                        }

                        for (auto* eAsset : detail::getChildren(eStream, "asset")) {
                            if (!eAsset) continue;

                            nyx::ElyAsset* asset = stream->add_asset();

                            std::string id = detail::getAttribute(eAsset, "id");
                            std::string type = detail::getAttribute(eAsset, "type");
                            std::string version = detail::getAttribute(eAsset, "version");
                            std::string src = detail::getAttribute(eAsset, "src");
                            std::string pipeline = detail::getAttribute(eAsset, "pipeline");

                            if (!id.empty())       asset->set_id(id.c_str());
                            if (!type.empty())     asset->set_type(type.c_str());
                            if (!version.empty())  asset->set_version(version.c_str());
                            if (!src.empty())      asset->set_src(src.c_str());
                            if (!pipeline.empty()) asset->set_pipeline(pipeline.c_str());
                        }
                    }
                }
            }
            void static parseUi(tinyxml2::XMLElement* eRoot, nyx::Elyrion& model) {

                if (auto* eUi = detail::findChild(eRoot, "ui")) {
                    if (auto* eLayout = detail::findChild(eUi, "layout")) {
                        nyx::ElyUI* ui = model.mutable_ui();
                        nyx::ElyLayout* layout = ui->mutable_layout();

                        std::string layoutId = detail::getAttribute(eLayout, "id");
                        if (!layoutId.empty()) layout->set_id(layoutId.c_str());

                        if (auto* eRootNode = detail::findChild(eLayout, "root")) {
                            nyx::ElyRoot* root = layout->mutable_root();

                            std::string type = detail::getAttribute(eRootNode, "type");
                            std::string layout = detail::getAttribute(eRootNode, "layout");

                            if (!type.empty())   root->set_type(type.c_str());
                            if (!layout.empty()) root->set_layout(layout.c_str());

                            if (auto* eChildren = detail::findChild(eRootNode, "children")) {
                                nyx::ElyChildren* children = root->mutable_children();
                                for (auto* eButton : detail::getChildren(eChildren, "button")) {
                                    if (!eButton) continue;

                                    nyx::ElyButton* btn = children->add_button();

                                    std::string id = detail::getAttribute(eButton, "id");
                                    std::string text = detail::getAttribute(eButton, "text");
                                    std::string action = detail::getAttribute(eButton, "action");

                                    if (!id.empty())     btn->set_id(id.c_str());
                                    if (!text.empty())   btn->set_text(text.c_str());
                                    if (!action.empty()) btn->set_action(action.c_str());
                                }
                            }
                        }
                    }
                }

            }
            void static parseScripts(tinyxml2::XMLElement* eRoot, nyx::Elyrion& model) {
                if (auto* eScripts = detail::findChild(eRoot, "scripts")) {
                    for (auto* eScript : detail::getChildren(eScripts, "script")) {
                        if (!eScript) continue;

                        nyx::ElyScript* script = model.mutable_scripts()->add_script();

                        std::string id = detail::getAttribute(eScript, "id");
                        std::string lang = detail::getAttribute(eScript, "lang");
                        std::string src = detail::getAttribute(eScript, "src");

                        if (!id.empty())   script->set_id(id.c_str());
                        if (!lang.empty()) script->set_lang(lang.c_str());
                        if (!src.empty())  script->set_src(src.c_str());

                        const char* innerText = eScript->GetText();
                        if (innerText && *innerText) {
                            script->set_content(innerText);
                        }
                        else if (!src.empty()) {
                            std::ifstream sfile(src);
                            if (sfile) {
                                std::string content((std::istreambuf_iterator<char>(sfile)),
                                    std::istreambuf_iterator<char>());
                                script->set_content(content);
                            }
                        }
                    }
                }
            }
        

        // Helper f�r libcurl WriteCallback
        size_t writeToString(void* contents, size_t size, size_t nmemb, void* userp) {
            size_t totalSize = size * nmemb;
            static_cast<std::string*>(userp)->append(static_cast<char*>(contents), totalSize);
            return totalSize;
        }

        inline bool parseXmlAndPopulateModel(const char* xml, nyx::Elyrion& model) {
            tinyxml2::XMLDocument doc;
            if (doc.Parse(xml) != tinyxml2::XML_SUCCESS) {
                std::cerr << "Failed to parse XML content.\n";
                return false;
            }

            auto* eRoot = doc.FirstChildElement("elyrion");
            if (!eRoot) {
                std::cerr << "No <elyrion> root element.\n";
                return false;
            }

            model.set_id(detail::getAttribute(eRoot, "id"));
            model.set_version(detail::getAttribute(eRoot, "version"));
            model.set_author(detail::getAttribute(eRoot, "author"));

            if (auto* eName = detail::findChild(eRoot, "name"))
                model.set_name(eName->GetText() ? eName->GetText() : "");

            if (auto* eDesc = detail::findChild(eRoot, "description"))
                model.set_description(eDesc->GetText() ? eDesc->GetText() : "");

            if (auto* eEntry = detail::findChild(eRoot, "entryScene"))
                model.set_entryscene(eEntry->GetText() ? eEntry->GetText() : "");

            if (auto* eDeps = detail::findChild(eRoot, "dependencies")) {
                for (auto* eDep : detail::getChildren(eDeps, "dependency")) {
                    nyx::ElyDependency* dep = model.add_dependencies();
                    dep->set_id(detail::getAttribute(eDep, "id"));
                    dep->set_version(detail::getAttribute(eDep, "version"));
                }
            }

            parseAssets(eRoot, model);
            parseScenes(eRoot, model);
            parseStreams(eRoot, model);
            parseUi(eRoot, model);
            parseScripts(eRoot, model);

            return true;
        }
    }

    inline bool loadXmlFromFile(std::string_view path, nyx::Elyrion& model) {
        std::ifstream file(std::string(path), std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open Elyrion XML file: " << path << "\n";
            return false;
        }

        std::string xmlData;
        file.seekg(0, std::ios::end);
        xmlData.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(&xmlData[0], xmlData.size());
        return parseXmlAndPopulateModel(xmlData.c_str(), model);
    }

    inline bool loadXmlFromUri(std::string_view uri, nyx::Elyrion& model) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cerr << "Failed to initialize CURL\n";
            return false;
        }

        std::string xmlData;
        curl_easy_setopt(curl, CURLOPT_URL, std::string(uri).c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &xmlData);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "xmlelyrion/1.0");

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
            return false;
        }

        return parseXmlAndPopulateModel(xmlData.c_str(), model);
    }
        // Behalte alle parse* Funktionen gleich, ersetze nur XmlAccessor durch detail::*
        // Beispiel f�r parseScenes folgt � du ersetzt es entsprechend auch in den anderen Funktionen
    
    
}
