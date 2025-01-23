/*
 * Copyright (C) 2024, Robert Patterson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <sstream>
#include <functional>
#include <numeric>

 // This header includes method implementations that need to see all the classes in the dom

#include "musx/musx.h"

#if ! defined(MUSX_RUNNING_ON_WINDOWS)
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace musx {
namespace dom {

// *****************
// ***** Entry *****
// *****************

std::shared_ptr<Entry> Entry::getNext() const
{
    if (!m_next) return nullptr;
    auto retval = getDocument()->getEntries()->get<Entry>(m_next);
    if (!retval) {
        MUSX_INTEGRITY_ERROR("Entry " + std::to_string(m_entnum) + " has next entry " + std::to_string(m_next) + " that does not exist.");
    }
    return retval;
}

std::shared_ptr<Entry> Entry::getPrevious() const
{
    if (!m_prev) return nullptr;
    auto retval = getDocument()->getEntries()->get<Entry>(m_prev);
    if (!retval) {
        MUSX_INTEGRITY_ERROR("Entry " + std::to_string(m_entnum) + " has previous entry " + std::to_string(m_prev) + " that does not exist.");
    }
    return retval;
}

Entry::NoteType Entry::calcNoteType() const
{
    if (duration <= 1 || duration >= 0x10000) {
        throw std::invalid_argument("Duration is out of valid range for NoteType.");
    }

    // Find the most significant bit position
    Edu value = duration;
    Edu msb = 1;
    while (value > 1) {
        value >>= 1;
        msb <<= 1;
    }

    return static_cast<Entry::NoteType>(msb);
}

int Entry::calcAugmentationDots() const
{
    int count = 0;
    for (Edu msb = Edu(calcNoteType()) >> 1; duration & msb; msb >>= 1) {
        count++;
    }
    return count;
}

// ***********************
// ***** FontOptions *****
// ***********************

std::shared_ptr<FontInfo> options::FontOptions::getFontInfo(options::FontOptions::FontType type) const
{
    auto it = fontOptions.find(type);
    if (it == fontOptions.end()) {
        throw std::invalid_argument("Font type " + std::to_string(int(type)) + " not found in document");
    }
    return it->second;
}

std::shared_ptr<FontInfo> options::FontOptions::getFontInfo(const DocumentPtr& document, options::FontOptions::FontType type)
{
    auto options = document->getOptions();
    if (!options) {
        throw std::invalid_argument("No options found in document");
    }
    auto fontOptions = options->get<options::FontOptions>();
    if (!fontOptions) {
        throw std::invalid_argument("Default fonts not found in document");
    }
    return fontOptions->getFontInfo(type);
}

// ********************
// ***** FontInfo *****
// ********************

std::string FontInfo::getName() const
{
    auto fontDef = getDocument()->getOthers()->get<others::FontDefinition>(getPartId(), fontId);
    if (fontDef) {
        return fontDef->name;
    }
    throw std::invalid_argument("font definition not found for font id " + std::to_string(fontId));
}

void FontInfo::setFontIdByName(const std::string& name)
{
    auto fontDefs = getDocument()->getOthers()->getArray<others::FontDefinition>(getPartId());
    for (auto fontDef : fontDefs) {
        if (fontDef->name == name) {
            fontId = fontDef->getCmper();
            return;
        }
    }
    throw std::invalid_argument("font definition not found for font \"" + name + "\"");
}

bool FontInfo::calcIsSMuFL() const
{
    auto name = getName();
    auto standardFontPaths = calcSMuFLPaths();
    for (const auto& path : standardFontPaths) {
        if (!path.empty()) {
            std::filesystem::path metaFilePath(path / name / name);
            metaFilePath.replace_extension(".json");
            if (std::filesystem::is_regular_file(metaFilePath)) {
                return true;
            }
        }
    }
    return false;
}

std::vector<std::filesystem::path> FontInfo::calcSMuFLPaths()
{
#if defined(MUSX_RUNNING_ON_WINDOWS)
    auto systemEnv = "COMMONPROGRAMFILES";
    auto userEnv = "LOCALAPPDATA";
#elif defined(MUSX_RUNNING_ON_MACOS)
    auto systemEnv = "";
    auto userEnv = "HOME";
#elif defined(MUSX_RUNNING_ON_LINUX_UNIX)
    auto systemEnv = "XDG_DATA_DIRS";
    auto userEnv = "XDG_DATA_HOME";
#else
    static_assert(false, "Unsupported OS for FontInfo::calcSMuFLPaths");
#endif

#if ! defined(MUSX_RUNNING_ON_WINDOWS)    
    auto getHomePath = []() -> std::string {
        auto homeEnv = getenv("HOME");
        if (homeEnv) {
            return homeEnv;
        }
        uid_t uid = getuid(); // Get the current user's UID
        struct passwd *pw = getpwuid(uid); // Fetch the password entry for the UID
        if (pw) {
            return pw->pw_dir;
        }
        return "";
    };
#else
    auto getHomePath = []() -> void {};
#endif
    
    auto getBasePaths = [getHomePath](const std::string& envVariable) -> std::vector<std::string> {
        std::vector<std::string> paths;
#if defined(MUSX_RUNNING_ON_WINDOWS)
        char* buffer = nullptr;
        size_t bufferSize = 0;
        if (_dupenv_s(&buffer, &bufferSize, envVariable.c_str()) == 0 && buffer != nullptr) {
            paths.emplace_back(buffer);
            free(buffer);
        } else {
            return {};
        }
#else
        if (envVariable == "HOME") {
            paths.emplace_back(getHomePath());
        } else if (!envVariable.empty()) {
            if (auto envValue = getenv(envVariable.c_str())) {
                std::stringstream ss(envValue);
                std::string path;
                while (std::getline(ss, path, ':')) {
                    paths.push_back(path);
                }
#if defined(MUSX_RUNNING_ON_LINUX_UNIX)
            } else if (envVariable == "XDG_DATA_HOME") {
                paths.emplace_back(getHomePath() + "/.local/share");
            } else if (envVariable == "XDG_DATA_DIRS") {
                paths.emplace_back("/usr/local/share");
                paths.emplace_back("/usr/share");
#endif         
            } else {
                return {};
            }
        }
        else {
            paths.emplace_back("/");
        }
#endif
        return paths;
    };
    auto paths = getBasePaths(userEnv);
    auto temp = getBasePaths(systemEnv);
    paths.insert(paths.end(),
                 std::make_move_iterator(temp.begin()),
                 std::make_move_iterator(temp.end()));
    std::vector<std::filesystem::path> retval;
    for (const auto& next : paths) {
        std::filesystem::path path = next;
#if defined(MUSX_RUNNING_ON_MACOS)
        path = path / "Library" / "Application Support";
#endif
        path = path / "SMuFL" / "Fonts";
        retval.emplace_back(std::move(path));
    }
    return retval;
}

// **********************
// ***** GFrameHold *****
// **********************

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
struct TupletState
{
    util::Fraction remainingSymbolicDuration;         // The remaining symbolic duration
    util::Fraction ratio;             // The remaining actual duration
    std::shared_ptr<const details::TupletDef> tuplet; // The tuplet.

    void accountFor(util::Fraction actual)
    {
        remainingSymbolicDuration -= (actual / ratio);
    }

    TupletState(const std::shared_ptr<details::TupletDef>& t)
        : remainingSymbolicDuration(t->displayNumber * t->displayDuration, int(Entry::NoteType::Whole)),
          ratio(t->inTheTimeOfNumber * t->inTheTimeOfDuration, t->displayNumber * t->displayDuration),
          tuplet(t)
    {
    }
};
#endif // DOXYGEN_SHOULD_IGNORE_THIS

bool details::GFrameHold::iterateEntries(LayerIndex layerIndex, std::function<bool(const std::shared_ptr<const EntryInfo>&)> iterator)
{
    if (layerIndex >= frames.size()) { // note: layerIndex is unsigned
        throw std::invalid_argument("invalid layer index [" + std::to_string(layerIndex) + "]");
    }
    if (!frames[layerIndex]) return true; // nothing here
    auto document = getDocument();
    auto frameIncis = document->getOthers()->getArray<others::Frame>(getPartId(), frames[layerIndex]);
    auto frame = [frameIncis]() -> std::shared_ptr<others::Frame> {
        for (const auto& frame : frameIncis) {
            if (frame->startEntry) {
                return frame;
            }
        }
        return nullptr;
    }();
    if (frame) {
        auto firstEntry = document->getEntries()->get<Entry>(frame->startEntry);
        if (!firstEntry) {
            MUSX_INTEGRITY_ERROR("GFrameHold for staff " + std::to_string(getStaff()) + " and measure " + std::to_string(getMeasure()) + " is not iterable.");
            return true; // we won't get here if we are throwing; otherwise it is just a warning and we can continue
        }
        std::vector<TupletState> activeTuplets; // List of active tuplets
        util::Fraction actualElapsedDuration = 0;
        for (const auto& f : frameIncis) {
            actualElapsedDuration += util::Fraction(f->startTime, int(Entry::NoteType::Whole)); // if there is an old-skool pickup, this accounts for it
        }
        for (auto nextEntry = firstEntry; nextEntry; nextEntry = nextEntry->getNext()) {
            auto entryInfo = std::make_shared<EntryInfo>(getStaff(), getMeasure(), layerIndex, nextEntry);
            auto tuplets = document->getDetails()->getArray<details::TupletDef>(SCORE_PARTID, nextEntry->getEntryNumber());
            for (const auto& tuplet : tuplets) {
                activeTuplets.emplace_back(tuplet);
            }

            // @todo: calculate and add running values (clef, key)
            util::Fraction cumulativeRatio = 1;
            for (const auto& t : activeTuplets) {
                cumulativeRatio *= t.ratio;
            }
            util::Fraction actualDuration = nextEntry->calcFraction() * cumulativeRatio;
            entryInfo->actualDuration = actualDuration;
            entryInfo->elapsedDuration = actualElapsedDuration;

            if (!iterator(entryInfo)) {
                return false;
            }

            if (nextEntry->getEntryNumber() == frame->endEntry) {
                break;
            }

            actualElapsedDuration += actualDuration;
            for (auto& t : activeTuplets) {
                t.remainingSymbolicDuration -= actualDuration / t.ratio;
            }
            activeTuplets.erase(
                std::remove_if(activeTuplets.begin(), activeTuplets.end(),
                    [](const TupletState& t) { return t.remainingSymbolicDuration <= 0; }),
                activeTuplets.end()
            );
        }
    } else {
        MUSX_INTEGRITY_ERROR("GFrameHold for staff " + std::to_string(getStaff()) + " and measure "
            + std::to_string(getMeasure()) + " points to non-existent frame [" + std::to_string(frames[layerIndex]) + "]");
    }
    return true;
}

bool details::GFrameHold::iterateEntries(std::function<bool(const std::shared_ptr<const EntryInfo>&)> iterator)
{
    for (LayerIndex layerIndex = 0; layerIndex < frames.size(); layerIndex++) {
        if (!iterateEntries(layerIndex, iterator)) {
            return false;
        }
    }
    return true;
}

// **************************
// ***** InstrumentUsed *****
// **************************

std::shared_ptr<others::Staff> others::InstrumentUsed::getStaffAtIndex(const std::vector<std::shared_ptr<others::InstrumentUsed>>& iuArray, Cmper index)
{
    if (index > iuArray.size()) return nullptr;
    auto iuItem = iuArray[index];
    return iuItem->getDocument()->getOthers()->get<others::Staff>(iuItem->getPartId(), iuItem->staffId);
}

// ****************************
// ***** MarkingCategiory *****
// ****************************

std::string others::MarkingCategory::getName() const
{
    auto catName = getDocument()->getOthers()->get<others::MarkingCategoryName>(getPartId(), getCmper());
    if (catName) {
        return catName->name;
    }
    return {};
}

// *****************************
// ***** PageFormatOptions *****
// *****************************

std::shared_ptr<options::PageFormatOptions::PageFormat> options::PageFormatOptions::calcPageFormatForPart(Cmper partId) const
{
    const auto& baseOptions = (partId == SCORE_PARTID) ? pageFormatScore : pageFormatParts;
    auto retval = std::make_shared<options::PageFormatOptions::PageFormat>(*baseOptions);
    auto pages = getDocument()->getOthers()->getArray<others::Page>(partId);
    auto page1 = pages.size() >= 1 ? pages[0] : nullptr;
    auto page2 = pages.size() >= 2 ? pages[1] : page1; // left page
    auto page3 = pages.size() >= 3 ? pages[2] : page1; // right page that isn't page 1
    if (page2) {
        retval->pageHeight = page2->height;
        retval->pageWidth = page2->width;
        retval->pagePercent = page2->percent;
        retval->leftPageMarginTop = page2->margTop;
        retval->leftPageMarginLeft = page2->margLeft;
        retval->leftPageMarginBottom = page2->margBottom;
        retval->leftPageMarginRight = page2->margRight;
    }
    if (page1) {
        if (retval->differentFirstPageMargin || page1->margTop != page2->margTop) {
            retval->firstPageMarginTop = page1->margTop;
            retval->differentFirstPageMargin = true;
        }
    }
    if (page3) {
        if (retval->facingPages || page3->margTop != page2->margTop || page3->margLeft != page2->margLeft
               || page3->margBottom != page2->margBottom || page3->margRight != page2->margRight) {
            retval->facingPages = true;
            retval->rightPageMarginTop = page3->margTop;
            retval->rightPageMarginLeft = page3->margLeft;
            retval->rightPageMarginBottom = page3->margBottom;
            retval->rightPageMarginRight = page3->margRight;
        }
    }
    auto systems = getDocument()->getOthers()->getArray<others::StaffSystem>(partId);
    auto system1 = systems.size() >= 1 ? systems[0] : nullptr;
    auto system2 = systems.size() >= 2 ? systems[1] : system1;
    if (system2) {
        retval->sysPercent = system2->ssysPercent;
        retval->rawStaffHeight = system2->staffHeight >> 2; // divide by 4 to convert Efix (1/64 Evpu) to Evpu16ths
        retval->sysMarginTop = system2->top;
        retval->sysMarginLeft = system2->left;
        retval->sysMarginBottom = system2->bottom;
        retval->sysMarginRight = system2->right;
        // do not copy system2->distanceToPrev because it varies from the default quite often
    }
    if (system1) {
        if (retval->differentFirstSysMargin || system1->top != system2->top || system1->left != system2->left) {
            retval->differentFirstSysMargin = true;
            retval->firstSysMarginTop = system1->top;
            retval->firstSysMarginLeft = system1->left;
            // do not change retval->firstSysMarginDistance because it varies so much depending on context
        }
    }
    return retval;
}

// **************************
// ***** PartDefinition *****
// **************************

std::string others::PartDefinition::getName() const
{
    return TextBlock::getText(getDocument(), nameId, true); // true: trim tags
}

// ********************
// ***** TextBase *****
// ********************

std::shared_ptr<FontInfo> TextsBase::parseFirstFontInfo() const
{
    std::string searchText = this->text;
    auto fontInfo = std::make_shared<FontInfo>(this->getDocument());
    bool foundTag = false;

    while (true) {
        if (!musx::util::EnigmaString::startsWithFontCommand(searchText)) {
            break;
        }

        size_t endOfTag = searchText.find_first_of(')');
        if (endOfTag == std::string::npos) {
            break;
        }

        std::string fontTag = searchText.substr(0, endOfTag + 1);
        if (!musx::util::EnigmaString::parseFontCommand(fontTag, *fontInfo.get())) {
            return nullptr;
        }

        searchText.erase(0, endOfTag + 1);
        foundTag = true;
    }

    if (foundTag) {
        return fontInfo;
    }

    return nullptr;
}

// *****************
// ***** Staff *****
// *****************

std::string others::Staff::getFullName() const
{
    return others::TextBlock::getText(getDocument(), fullNameTextId, true); // true: strip enigma tags
}

// *********************
// ***** TextBlock *****
// *********************

std::string others::TextBlock::getText(bool trimTags) const
{
    auto document = getDocument();
    auto getText = [&](const auto& block) -> std::string {
        if (!block) return {};
        if (!trimTags) return block->text;
        auto retval = musx::util::EnigmaString::replaceAccidentalTags(block->text);
        return musx::util::EnigmaString::trimTags(retval);
    };
    switch (textType) {
    default:
    case TextType::Block:
        return getText(document->getTexts()->get<texts::BlockText>(textId));
    case TextType::Expression:
        return getText(document->getTexts()->get<texts::ExpressionText>(textId));        
    }
}

std::string others::TextBlock::getText(const DocumentPtr& document, const Cmper textId, bool trimTags)
{
    auto textBlock = document->getOthers()->get<others::TextBlock>(SCORE_PARTID, textId);
    if (textBlock) {
        return textBlock->getText(trimTags);
    }
    return {};
}

// *****************************
// ***** TextExpressionDef *****
// *****************************

std::shared_ptr<others::Enclosure> others::TextExpressionDef::getEnclosure() const
{
    if (!hasEnclosure) return nullptr;
    return getDocument()->getOthers()->get<others::TextExpressionEnclosure>(getPartId(), getCmper());
}

} // namespace dom    
} // namespace musx
