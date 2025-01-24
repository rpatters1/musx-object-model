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
#include <functional>
#include <vector>

#include "BaseClasses.h"

 // do not add other dom class dependencies. Use Implementations.h for implementations that need total class access.

namespace musx {
namespace dom {

class EntryFrame;

namespace options {

class TupletOptions;

} // namespace options

/**
 * @namespace musx::dom::details
 * @brief Classes in the @ref DetailsPool.
 */
namespace details {

/**
 * @class GFrameHold
 * @brief Represents the attributes of a Finale frame holder.
 *
 * Cmper1 is the staff (inst) @ref Cmper and Cmper2 is the measur @ref Cmper
 * This class is identified by the XML node name "gfhold".
 */
class GFrameHold : public DetailsBase
{
public:
    /**
     * @brief Constructor function
     * @param document A weak pointer to the associated document.
     * @param partId The part that this is for (probably always 0).
     * @param shareMode The sharing mode for this #GFrameHold (probably always #ShareMode::All)
     * @param inst The staff ID for this #GFrameHold.
     * @param meas The measure ID for this #GFrameHold.
     */
    explicit GFrameHold(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper inst, Cmper meas)
        : DetailsBase(document, partId, shareMode, inst, meas), frames(MAX_LAYERS) {}

    /**
     * @brief Enum representing the clef mode for the frame.
     */
    enum class ShowClefMode
    {
        WhenNeeded, ///< Clef is displayed only when needed (the default).
        Never,      ///< Clef is never displayed. (xml value is "hidden")
        Always      ///< Clef is always displayed. (xml value is "forced")
    };

    // Public properties corresponding to the XML structure
    std::optional<ClefIndex> clefId;        ///< clef index when there are no mid-measure clef changes. (xml tag is `<clefID>`).
    Cmper clefListId{};                     ///< The clef list ID when there are mid-measure clef changes, if non-zero. (xml tag is `<clefListID>`).
    ShowClefMode showClefMode{};            ///< "Show Clef" mode. (xml tag is `<clefMode>`)
    bool mirrorFrame{};                     ///< Indicates this is a mirror frame. (Not used after Finale 14.5.)
    int clefPercent{};                      ///< Clef percent where 100 means 100%.
    std::vector<Cmper> frames;              ///< @ref others::Frame values for layers 1..4 (layer indices 0..3) if non-zero

    /// @brief returns the inst (staff) number for this #GFrameHold
    InstCmper getStaff() const { return InstCmper(getCmper1()); }

    /// @brief returns the measure number for this #GFrameHold
    MeasCmper getMeasure() const { return MeasCmper(getCmper2()); }

    /** @brief Returns the @ref EntryFrame for all entries in the given layer.
     *
     * @param layerIndex The layer index (0..3) to iterate.
     * @return EntryFrame for layer or nullptr if none.
     */
    std::shared_ptr<const EntryFrame> createEntryFrame(LayerIndex layerIndex) const;
    
    /**
     * @brief iterates the entries for the specified layer in this #GFrameHold from left to right
     * @param layerIndex The layer index (0..3) to iterate.
     * @param iterator The callback function for each iteration.
     * @return true if higher-level iteration should continue. false if it should halt.
     * @throws std::invalid_argument if the layer index is out of range
     */
    bool iterateEntries(LayerIndex layerIndex, std::function<bool(const std::shared_ptr<const EntryInfo>&)> iterator);

    /**
     * @brief iterates the entries for this #GFrameHold from left to right for each layer in order
     * @param iterator The callback function for each iteration.
     * @return true if higher-level iteration should continue. false if it should halt.
     */
    bool iterateEntries(std::function<bool(const std::shared_ptr<const EntryInfo>&)> iterator);

    void integrityCheck() const override
    {
        this->DetailsBase::integrityCheck();
        if (clefListId && clefId.has_value()) {
            MUSX_INTEGRITY_ERROR("GFrameHold for staff " + std::to_string(getCmper1()) + " and measure " + std::to_string(getCmper2()) + " has both clef and clef list.");
        }
        if (!clefListId && !clefId.has_value()) {
            MUSX_INTEGRITY_ERROR("GFrameHold for staff " + std::to_string(getCmper1()) + " and measure " + std::to_string(getCmper2()) + " has neither clef nor clef list.");
        }
    }

    constexpr static std::string_view XmlNodeName = "gfhold"; ///< The XML node name for this type.
    static const xml::XmlElementArray<GFrameHold> XmlMappingArray; ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TupletDef
 * @brief Options controlling the appearance of tuplets.
 *
 * This class is identified by the XML node name "tupletDef".
 */
class TupletDef : public EntryDetailsBase
{
public:
    /** @brief Constructor function */
    explicit TupletDef(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum, Inci inci)
        : EntryDetailsBase(document, partId, shareMode, entnum, inci)
    {
    }

    /// @brief see @ref options::TupletOptions::AutoBracketStyle
    using AutoBracketStyle = options::TupletOptions::AutoBracketStyle;
    /// @brief see @ref options::TupletOptions::NumberStyle
    using NumberStyle = options::TupletOptions::NumberStyle;
    /// @brief see @ref options::TupletOptions::PositioningStyle
    using PositioningStyle = options::TupletOptions::PositioningStyle;
    /// @brief see @ref options::TupletOptions::BracketStyle
    using BracketStyle = options::TupletOptions::BracketStyle;

    int displayNumber{};                    ///< The number of notes to display (xml node is `<symbolicNum>`)
    Edu displayDuration{};                  ///< The duration of each note to display (xml node is `<symbolicDur>`)
    int referenceNumber{};                  ///< The number of notes "in the time of" (xml node is `<refNum>`)
    Edu referenceDuration{};                ///< The duration of eacn note "in the time of" (xml node is `<refDur>`)
    bool alwaysFlat{};                      ///< "Always Flat" (xml node is `<flat>`)
    bool fullDura{};                        ///< "Bracket Full Duration"
    bool metricCenter{};                    ///< "Center Number Using Duration"
    bool avoidStaff{};                      ///< "Avoid Staff"
    AutoBracketStyle autoBracketStyle{};    ///< Autobracket style
    Evpu tupOffX{};                         ///< Horizontal offset.
    Evpu tupOffY{};                         ///< Vertical.
    Evpu brackOffX{};                       ///< Horizontal offset for brackets.
    Evpu brackOffY{};                       ///< Vertical offset for brackets.
    NumberStyle numStyle{};                 ///< Number style
    PositioningStyle posStyle{};            ///< Positioning style
    bool allowHorz{};                       ///< "Allow Horizontal Drag"
    bool ignoreHorzNumOffset{};             ///< "Ignore Horizontal Number Offset" (xml node is `<ignoreGlOffs>`)
    bool breakBracket{};                    ///< "Break Slur or Bracket"
    bool matchHooks{};                      ///< "Match Length of Hooks"
    bool useBottomNote{};                   ///< "Use Bottom Note" (xml node is `<noteBelow>`)
    BracketStyle brackStyle{};              ///< Bracket style.
    bool smartTuplet{};                     ///< "Engraver Tuplets"
    Evpu leftHookLen{};                     ///< Length of the left hook in the tuplet bracket. (This value is sign-reversed in the Finale UI.)
    Evpu leftHookExt{};                     ///< Extension of the left hook beyond the tuplet bracket.
    Evpu rightHookLen{};                    ///< Length of the right hook in the tuplet bracket. (This value is sign-reversed in the Finale UI.)
    Evpu rightHookExt{};                    ///< Extension of the right hook beyond the tuplet bracket.
    Evpu manualSlopeAdj{};                  ///< "Manual Slope Adjustment" in @ref Evpu. (xml node is `<slope>`)

    /** @brief return the reference duration as a @ref util::Fraction of a whole note */
    util::Fraction calcReferenceDuration() const { return util::Fraction(referenceNumber * referenceDuration, Edu(Entry::NoteType::Whole)); }

    /** @brief return the display duration as a @ref util::Fraction of a whole note */
    util::Fraction calcDisplayDuration() const { return util::Fraction(displayNumber * displayDuration, Edu(Entry::NoteType::Whole)); }

    /** @brief return the tuplet ratio (reference / display) */
    util::Fraction calcRatio() const { return util::Fraction(referenceNumber * referenceDuration, displayNumber * displayDuration); }
        
    constexpr static std::string_view XmlNodeName = "tupletDef";    ///< The XML node name for this type.
    static const xml::XmlElementArray<TupletDef> XmlMappingArray;   ///< Required for musx::factory::FieldPopulator.
};

} // namespace details
} // namespace dom
} // namespace entries