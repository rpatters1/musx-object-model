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
#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <stdexcept>
#include <map>

#include "BaseClasses.h"
// do not add other dom class dependencies. Use Implementations.h for implementations that need total class access.

namespace musx {
namespace dom {
namespace others {

/**
 * @class Enclosure
 * @brief Represents the enclosure settings for text expressions.
 */
class Enclosure : public OthersBase
{
public:
    /**
     * @enum Shape
     * @brief Enumeration for enclosure shapes.
     */
    enum class Shape : uint8_t
    {
        NoEnclosure = 0,    ///< No enclosure
        Rectangle = 1,      ///< Rectangle
        Ellipse = 2,        ///< Ellipse
        Triangle = 3,       ///< Triangle
        Diamond = 4,        ///< Diamond
        Pentagon = 5,       ///< Pentagon
        Hexagon = 6,        ///< Hexagon
        Heptagon = 7,       ///< Heptagon
        Octogon = 8         ///< Octogon
    };

    /**
     * @brief Constructs an Enclosure object.
     * @param document Shared pointer to the document.
     * @param partId Usually 0. This parameter is needed for the generic factory routine.
     * @param cmper Comparison parameter.
     */
    explicit Enclosure(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    Evpu xAdd{};              ///< Center X offset - offsets text from center (in EVPU).
    Evpu yAdd{};              ///< Center Y offset - offsets text from center (in EVPU).
    Evpu xMargin{};           ///< Half width - extra space on left/right sides (in EVPU).
    Evpu yMargin{};           ///< Half height - extra space on top/bottom sides (in EVPU).
    Efix lineWidth{};         ///< Line thickness in 64ths of an EVPU (EFIX).
    Shape shape{};            ///< Enclosure shape (default: NoEnclosure).
    Efix cornerRadius{};      ///< Corner radius (in EFIX).
    bool fixedSize{};         ///< Whether the enclosure is fixed size (ignore text bounding box)
    bool equalAspect{};       ///< "Match Height and Width"
    bool notTall{};           ///< "Enforce Minimum Width": don't let shape get taller than it is wide
    bool opaque{};            ///< Whether the enclosure is opaque.
    bool roundCorners{};      ///< Whether the enclosure has rounded corners.
};

/**
 * @class FontDefinition
 * @brief The name and font characteristics of fonts contained.
 *
 * The cmper is the font ID used in classes throughout the document.
 *
 * This class is identified by the XML node name "fontName".
 */
class FontDefinition : public OthersBase
{
public:
    /** @brief Constructor function */
    explicit FontDefinition(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    // Public properties corresponding to the XML structure
    std::string charsetBank;    ///< probably only "Mac" or "Win"
    int charsetVal{};           ///< A value specifying the character set, usually 4095 or 0
    int pitch{};                ///< Represents the `<pitch>` element, e.g., 0. (use unknown)
    int family{};               ///< Represents the `<family>` element, e.g., 0. (use unknown)
    std::string name;           ///< The font name e.g., "Broadway Copyist Text".

    constexpr static std::string_view XmlNodeName = "fontName"; ///< The XML node name for this type.
};

/**
 * @class LayerAttributes
 * @brief Represents the attributes of a Finale "layer".
 *
 * The cmper is the layer ID (0-3). In the Finale UI these are represented as Layer 1 thru Layer 4.
 * The 0-based cmper value is used to identify layers where needed, rather than the 1-based UI value.
 *
 * This class is identified by the XML node name "layerAtts".
 */
class LayerAttributes : public OthersBase
{
public:
    /** @brief Constructor function */
    explicit LayerAttributes(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    // Public properties corresponding to the XML structure
    int restOffset{};                   ///< "Adjust Floating Rests by " number of staff steps.
    bool freezTiesToStems{};            ///< "Freeze Ties in the Same Direction as Stems" (xml node is `<flipTies>`)
    bool onlyIfOtherLayersHaveNotes{};  ///< "Apply Settings Only if Notes are in Other Layers" (xml node is `<floatLayer>`)
    bool useRestOffset{};               ///< "Adjust Floating Rests by ". Determines if `restOffset` is used. (xml node is `<useRestOff>`)
    bool freezeStemsUp{};               ///< If true, freeze stems up, otherwise freeze stems down. Only used if `freezeLayer` is true. (xml node is `<freezeLayUp>`)
    bool freezeLayer{};                 ///< "Freeze Stems and Ties"
    bool playback{};                    ///< If true, playback this layer. If false, the layer is muted.
    bool affectSpacing{};               ///< "Affect Music Spacing" (xml node is `<spacing>`)
    bool ignoreHiddenNotesOnly{};       ///< "Ignore Layers Containing Only Hidden Notes" (xml node is `<ignoreHidden>`)
    bool ignoreHiddenLayers{};          ///< "Ignore Hidden Layers"
    bool hideLayer{};                   ///< "Hide Layer when Inactive"

    constexpr static std::string_view XmlNodeName = "layerAtts"; ///< The XML node name for this type.
};

/**
 * @enum RehearsalMarkStyle
 * @brief Specifies the sequencing style for rehearsal marks
 */
enum class RehearsalMarkStyle
{
    None,                       ///< Default value, no rehearsal mark style
    Letters,                    ///< A, B, C...AA, BB, CC
    LetterNumbers,              ///< A, B, C...A1, B1, C1
    LettersLowerCase,           ///< a, b, c...aa, bb, cc
    LettersNumbersLowerCase,    ///< a, b, c...a1, b1, c1
    Numbers,                    ///< 1, 2, 3, 4
    MeasureNumber               ///< Displays the current measure number
};

/**
 * @enum PlaybackType
 * @brief Specifies the playback behavior for the an expression or marking category.
 */
enum class PlaybackType
{
    None,                   ///< Default value, no playback (the default).
    Tempo,                  ///< Playback affects tempo.  (xml value is "time")
    MidiController,         ///< Playback type for MIDI controller commands.
    KeyVelocity,            ///< Playback affects key velocity. (xml value is "amplitude")
    Transpose,              ///< Playback causes transposition.
    Channel,                ///< Playback set the MIDI channel.
    MidiPatchChange,        ///< Playback changes the MIDI patch.
    PercussionMidiMap,      ///< Playback uses percussion MIDI map. (xml value is "percMidiMap")
    MidiPitchWheel,         ///< Playback affects the MIDI pitch wheel. (xml value is "midiPitchwheel")
    ChannelPressure,        ///< Playback affects MIDI channel pressure. (xml vlaue is "midiPressure")
    RestrikeKeys,           ///< Playback retrikes keys. (xml value is "rekey")
    Dump,                   ///< Playback is an arbitrary data dump. (Data is in <playDumpText> with the same Cmper value.)
    PlayTempoToolChanges,   ///< Play changes from Tempo Tool. (xml value is "startTempo")
    IgnoreTempoToolChanges, ///< Ignore changes from Tempo Tool. (xml value is "stopTempo")
    Swing,                  ///< Playback in swing style
    SmartPlaybackOn,        ///< Turn on smart playback. (xml value is "hpOn")
    SmartPlaybackOff,       ///< Turn off smart playback. (xml value is "hpOff")
};

/**
 * @enum HorizontalMeasExprAlign
 * @brief Specifies the horizontal alignment relative to musical elements for an expression or marking category.
 */
enum class HorizontalMeasExprAlign
{
    LeftBarline,            ///< Align with left barline (the default). (xml value is "leftEdge", if encountered)
    Manual,                 ///< "Horizontal Click Position" alignment.
    LeftOfAllNoteheads,     ///< Align left of all noteheads
    LeftOfPrimaryNotehead,  ///< Align to the left of the primary notehead.
    Stem,                   ///< Align to the stem.
    CenterPrimaryNotehead,  ///< Align to the center of the primary notehead.
    CenterAllNoteheads,     ///< Align to the center of all noteheads.
    RightOfAllNoteheads,    ///< Align to the right of all noteheads.
    StartTimeSig,           ///< Align at the start of the time signature.
    AfterClefKeyTime,       ///< Align after clef/key/time/repeat.
    StartOfMusic,           ///< Align at start of music.
    CenterOverBarlines,     ///< Align over the center of the barlines.
    CenterOverMusic,        ///< Align over the center of music.
    RightBarline            ///< Align with left barline. (xml value is "rightEdge")
};

/**
 * @enum VerticalMeasExprAlign
 * @brief Specifies the vertical alignment relative to musical elements for an expression or marking category.
 */
enum class VerticalMeasExprAlign
{
    AboveStaff,          ///< Align above ths staff (the default).
    Manual,              ///< "Vertical Click Position" alignment.
    RefLine,             ///< Align to staff reference line.
    BelowStaff,          ///< Align below the staff.
    TopNote,             ///< Align with top note.
    BottomNote,          ///< Align with bottom note.
    AboveEntry,          ///< Align above entry.
    BelowEntry,          ///< Align below entry.
    AboveStaffOrEntry,   ///< Align above the staff or entry.
    BelowStaffOrEntry    ///< Align below the staff or entry.
};

/**
 * @enum HorizontalExprJustification
 * @brief Specifies the horizontal alignment for text expressions and marking categories.
 */
enum class HorizontalExprJustification
{
    Left,    ///< Justified left.
    Center,  ///< Justified center.
    Right    ///< Justified right.
};

class TextExpressionDef;

/**
 * @class MarkingCategory
 * @brief Represents a category of markings used.
 *
 * This class is identified by the XML node name "markingsCategory".
 */
class MarkingCategory : public OthersBase
{
public:
    /** @brief Enumeration for the type of marking category */
    enum class CategoryType : Cmper
    {
        Invalid,            ///< There should always be a category type supplied
        Dynamics,           ///< Dynamics markings, such as forte, piano, etc.
        TempoMarks,         ///< Tempo indications such as Allegro (with or without metronome marking).
        TempoAlterations,   ///< Tempo alteration markings, such as accel. and rit. (xml value is "tempoAlts")
        ExpressiveText,     ///< Expressive text such as "espressivo."
        TechniqueText,      ///< Technique text such as "pizzicato" or "arco."
        RehearsalMarks,     ///< Rehearsal marks, often used for reference points in the score.
        Misc                ///< Represents miscellaneous markings that do not fit into other categories.
    };

    /** @brief Constructor function */
    explicit MarkingCategory(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    CategoryType categoryType{}; ///< Category type of the marking

    // Font information for the marking category
    std::shared_ptr<FontInfo> textFont;      ///< Text font
    std::shared_ptr<FontInfo> musicFont;     ///< Music font
    std::shared_ptr<FontInfo> numberFont;    ///< Number font

    HorizontalMeasExprAlign horzAlign{};     ///< Horizontal alignment for the marking
    VerticalMeasExprAlign vertAlign{};       ///< Vertical alignment for the marking
    HorizontalExprJustification justification{}; ///< Justification for the text within the marking

    // Vertical and horizontal offsets for positioning adjustments
    Evpu horzOffset{};         ///< Additional horizontal offset
    Evpu vertOffsetBaseline{}; ///< Additional vertical offset
    Evpu vertOffsetEntry{};    ///< Additional vertical entry offset

    // Usage flags representing certain behaviors and visual elements
    bool usesTextFont{};      ///< whether this category uses the text font
    bool usesMusicFont{};     ///< whether this category uses the music font
    bool usesNumberFont{};    ///< whether this category uses the number font
    bool usesPositioning{};   ///< whether this category uses the positioning elements (Finale UI only allows true)
    bool usesStaffList{};     ///< whether this category uses a staff list
    bool usesBreakMmRests{};  ///< whether this category uses the `breakMmRest` setting
    bool breakMmRest{};       ///< whether expressions in this category break multimeasure rests
    bool userCreated{};       ///< whether the user created this category

    // Staff list represented as an integer
    Cmper staffList{};        ///< The staff list if `useStaffList` is true

    /** @brief A list of text expressions in this category.
     *
     * (This in not in the xml but is created by the factory.)
     */
    std::map<Cmper, std::weak_ptr<TextExpressionDef>> textExpressions;

    /** @brief gets the name of the marking category */
    std::string getName() const;

    constexpr static std::string_view XmlNodeName = "markingsCategory"; ///< The XML node name for this type.
};

/**
 * @class MarkingCategoryName
 * @brief Represents the name associated with a @ref MarkingCategory.
 *
 * This class has the same #Cmper as its @ref MarkingCategory.
 *
 * This class is identified by the XML node name "markingsCategoryName".
 */
class MarkingCategoryName : public OthersBase
{
public:
    /** @brief Constructor function */
    explicit MarkingCategoryName(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    std::string name; ///< The name of the marking category.

    constexpr static std::string_view XmlNodeName = "markingsCategoryName"; ///< The XML node name for this type.
};

/**
 * @class PartGlobals
 * @brief Represents global values that can vary by part
 *
 * The cmper is always 65534.
 *
 * This class is identified by the XML node name "partGlobals".
 */
class PartGlobals : public OthersBase
{
public:
    /** @brief Constructor function */
    explicit PartGlobals(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    // Public properties corresponding to the XML structure
    bool showTransposed{};                  ///< If true, "Display Concert Pitch" is unchecked for the part.
    Cmper scrollViewIUlist{};               ///< If this value is non-zero, it is the iuList @ref Cmper of the current Staff List in Scroll View.
    Cmper studioViewIUlist{};               ///< The iuList @ref Cmper for Studio View.
    /** @brief If non-zero, Special Part Extraction is in effect and this is the @ref Cmper for its @ref InstrumentUsed array.
     *
     * When Special Part Extraction is in effect, staff systems no longer have their own instrument lists. Instead, they use this value.
     *
     * (xml node is `<pageViewIUlist>`)
    */
    Cmper specialPartExtractionIUList{};    ///< If non-zero, Special Part Extraction is in effect and this is the iuList @ref Cmper. 

    constexpr static std::string_view XmlNodeName = "partGlobals"; ///< The XML node name for this type.
};

/**
 * @class TextExpressionDef
 * @brief Stores the properties and behaviors of text expressions.
 *
 * This class is identified by the XML node name "textExprDef".
 */
class TextExpressionDef : public OthersBase
{
public:
    Cmper textIdKey{};                              ///< Identifier for the @ref TextBlock associated with this 
    Cmper categoryId{};                             ///< Identifier for the category of the text expression.
    RehearsalMarkStyle rehearsalMarkStyle{};        ///< Auto-sequencing style for rehearsal marks.
    int value{};                                    ///< Value associated with the expression (e.g., velocity).
    int auxData1{};                                 ///< Auxiliary data for the expression. (xml node is "auxdata1")
    int playPass{};                                 ///< "Play Only on Pass" value.
    bool hideMeasureNum;                            ///< "Hide Measure Numbers" (used on Rehearsal Marks)
    bool useAuxData{};                              ///< Whether auxiliary data is used.
    bool hasEnclosure{};                            ///< Whether the text expression has an enclosure. (xml node is "newEnclosure")
    bool breakMmRest{};                             ///< Whether the text breaks multimeasure rests.
    PlaybackType playbackType{};                    ///< Playback behavior of the text expression.
    HorizontalMeasExprAlign horzMeasExprAlign{};    ///< Horizontal alignment of the expression.
    VerticalMeasExprAlign vertMeasExprAlign{};      ///< Vertical alignment of the expression.
    HorizontalExprJustification horzExprJustification{}; ///< Horizontal justification of the text expression.
    Evpu measXAdjust{};                             ///< Horizontal adjustment for measurement alignment.
    Evpu yAdjustEntry{};                            ///< Vertical adjustment for entry alignment.
    Evpu yAdjustBaseline{};                         ///< Vertical adjustment for baseline alignment.
    bool useCategoryFonts{};                        ///< Whether to use category fonts.
    bool useCategoryPos{};                          ///< Whether to use category position.
    std::string description;                        ///< Description of the text expression. (xml node is "descStr")

    /**
     * @brief Constructor.
     *
     * Initializes all fields to their default values.
     */
    explicit TextExpressionDef(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    /** @brief Gets the enclosure for this expression, or nullptr if none. */
    std::shared_ptr<Enclosure> getEnclosure() const;

    constexpr static std::string_view XmlNodeName = "textExprDef"; ///< The XML node name for this type.
};

/**
 * @class TextExpressionEnclosure
 * @brief The enclosure for a text expression (if it exists)
 *
 * The cmper is the same as for the associated @ref TextExpressionDef`.
 *
 * This class is identified by the XML node name "textExpressionEnclosure".
 */
class TextExpressionEnclosure : public Enclosure
{
public:
    using Enclosure::Enclosure;

    constexpr static std::string_view XmlNodeName = "textExpressionEnclosure"; ///< The XML node name for this type.
};

/**
 * @class TextRepeatEnclosure
 * @brief The enclosure for a text expression (if it exists)
 *
 * The cmper is the same as for @ref TextRepeateDef.
 *
 * This class is identified by the XML node name "textRepeatEnclosure".
 */
class TextRepeatEnclosure : public Enclosure
{
public:
    using Enclosure::Enclosure;

    constexpr static std::string_view XmlNodeName = "textRepeatEnclosure"; ///< The XML node name for this type.
};

} // namespace others
} // namespace dom
} // namespace musx