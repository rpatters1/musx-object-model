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

#include <array>
#include <memory>
#include <string>
#include <tuple>

#include "musx/dom/BaseClasses.h"
#include "musx/dom/Options.h"
#include "musx/dom/Others.h"
#include "musx/xml/XmlInterface.h"
#include "FieldPopulatorsOptions.h"
#include "FieldPopulatorsOthers.h"

namespace musx {
namespace factory {

/**
 * @brief A registry of types for mapping XML node names to types.
 *
 * This template struct provides a mechanism to map XML node names to their corresponding types
 * and supports instantiation of types based on these names.
 *
 * @tparam Types The list of types to be registered.
 */
template <typename... Types>
class TypeRegistry
{
private:
    using VariantType = std::variant<Types*...>;
    using Base = dom::Base;

    /**
     * @brief A compile-time registry of types, each associated with an XML node name.
     *
     * The registry is an array of pairs, where each pair contains a node name (as a string)
     * and a corresponding type pointer (as nullptr).
     */
    static inline const auto registry = []() {
        return std::unordered_map<std::string_view, VariantType>{
            {Types::XmlNodeName, VariantType(static_cast<Types*>(nullptr))}...
        };
    }();

    /**
     * @brief Finds the registered type corresponding to the provided node name.
     *
     * Searches the registry for an entry that matches the given node name.
     *
     * @param nodeName The XML node name to search for.
     * @return A pair consisting of a boolean indicating success and a type pointer if found.
     */
    static std::optional<VariantType> findRegisteredType(std::string_view nodeName)
    {
        const auto it = registry.find(nodeName);
        if (it == registry.end()) {
            return std::nullopt;
        }
        return it->second;
    }

public:
    /**
     * @brief Creates an instance of the registered type corresponding to the provided node name.
     *
     * Uses the node name to look up the registered type and create an instance of it.
     *
     * @tparam Args The argument types required by the constructor of the target type.
     * @param node The XML node from which an instance is to be created.
     * @param args Arguments to be forwarded to the constructor of the target type.
     * @return A shared pointer to the created instance of the base type, or nullptr if not found.
     */
    template <typename... Args>
    static std::shared_ptr<Base> createInstance(const std::shared_ptr<xml::IXmlElement>& node, ElementLinker& elementLinker, Args&&... args)
    {
        auto typePtr = TypeRegistry::findRegisteredType(node->getTagName());
        if (!typePtr.has_value()) {
            return nullptr; // Type not yet implemented
        }

        return std::visit(
            [&](auto const& ptr) -> std::shared_ptr<Base> {
                using T = std::remove_pointer_t<std::remove_reference_t<decltype(ptr)>>;
                // Only enable this part if T is constructible with Args...
                if constexpr (std::is_constructible_v<T, Args...>) {
                    auto instance = std::make_shared<T>(std::forward<Args>(args)...);
                    factory::FieldPopulator<T>::populate(instance, node, elementLinker);
                    return instance;
                } else {
                    throw std::runtime_error("Selected type is not constructible with given arguments");
                }
            },
            typePtr.value()
        );
    }
};

/**
 * @brief The type registery. 
 */
using RegisteredTypes = TypeRegistry <
    // options
    dom::options::DefaultFonts,
    // others
    dom::others::FontDefinition,
    dom::others::MarkingCategory,
    dom::others::MarkingCategoryName,
    dom::others::TextExpressionDef,
    dom::others::TextExpressionEnclosure,
    dom::others::TextRepeatEnclosure
    // Add pointers to additional supported types here.
    // Also add a field populator in FieldPopulatorsOthers.h
>;

} // namespace factory
} // namespace musx
