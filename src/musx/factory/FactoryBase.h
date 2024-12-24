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

#include <stdexcept>
#include <string>
#include <string_view>
#include <optional>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <tuple>
#include <sstream>

#include "musx/util/Logger.h"
#include "musx/xml/XmlInterface.h"
#include "musx/dom/BaseClasses.h"

namespace musx {

/**
 * @namespace musx::factory
 * @brief Class factories are in this namespace
 */
namespace factory {

using namespace musx::xml;
using namespace musx::dom;

/**
 * @class ElementLinker
 * @brief A utility class for managing deferred relationships between elements during document construction.
 *
 * This class allows capturing relationships that cannot be resolved immediately during factory creation.
 * The relationships are stored as resolver functions, which are executed later when all elements have been created.
 * It also provides a mechanism to ensure that specific resolvers are added only once by using unique keys.
 */
class ElementLinker {
public:
    /**
     * @typedef Resolver
     * @brief A callable type representing a deferred relationship resolver.
     *
     * Each resolver encapsulates the logic to resolve relationships between elements
     * once the document is fully constructed.
     */
    using Resolver = std::function<void(const dom::DocumentPtr&)>;

    /**
     * @brief Adds a resolver function to the linker.
     *
     * This function captures the logic for resolving relationships that cannot be resolved immediately.
     * Optionally, a unique key can be provided to ensure that the resolver is only added once.
     *
     * @param resolver A callable object that resolves a relationship when invoked.
     * @param key An optional unique key for the resolver. If provided, the resolver is added only once per key.
     */
    void addResolver(Resolver resolver, const std::string_view& key = {})
    {
        if (!key.empty()) {
            if (registeredResolvers.count(key) > 0) {
                return; // Skip adding if the resolver with the same key is already registered
            }
            registeredResolvers.insert(key);
        }
        resolvers.emplace_back(std::move(resolver));
    }

    /**
     * @brief Resolves all deferred relationships.
     *
     * Executes all stored resolver functions, establishing relationships between elements.
     * Clears the internal storage of resolvers and registered keys after execution.
     *
     * @param document The document in which relationships are resolved.
     * @throws std::runtime_error If any resolver function encounters an error.
     */
    void resolveAll(const dom::DocumentPtr& document)
    {
        for (auto& resolver : resolvers) {
            resolver(document);
        }
        resolvers.clear(); ///< Clear resolvers after execution
        registeredResolvers.clear(); ///< Clear registered keys after execution
    }

private:
    /// @brief A collection of resolver functions.
    std::vector<Resolver> resolvers;

    /**
     * @brief Tracks registered resolver keys to ensure uniqueness.
     *
     * This set stores keys for resolvers that have already been added, preventing duplicate registrations.
     */
    std::unordered_set<std::string_view> registeredResolvers;
};


/**
 * @brief Factory base class.
 */
class FactoryBase
{
protected:
    /** @brief Helper function to check if a child exists and populate it if so.
     *
     * EnigmaXml generally omits fields when they have the default value (usually 0). This helper
     * function handles that without causing an exception. However, fields retrieved with this function
     * are expected to exist unless the `expected` parameter is false. If they are missing but expected,
     * this fact may be logged.
     *
     * @tparam DataType the type of data to be populated (usually inferred from the call)
     * @tparam ParserFunc the parser function (usually inferred from the call)
     */
    template<typename DataType, typename ParserFunc>
    static void getFieldFromXml(const XmlElementPtr& element, const std::string& nodeName, DataType& dataField, ParserFunc parserFunc, bool expected = false)
    {
        if (auto childElement = element->getFirstChildElement(nodeName)) {
            dataField = parserFunc(childElement);
        } else if (expected) {
            std::stringstream msg;
            msg << "Expected field <" << element->getTagName() << "><" << nodeName << "> not found.";
            util::Logger::log(util::Logger::LogLevel::Warning, msg.str());
        }
    }

    /** @brief Helper function to throw when child element does not exist.
     *
     * @throws std::invalid_argument when child element does not exist.
     */
    static XmlElementPtr getFirstChildElement(const XmlElementPtr& element, const std::string& childElementName)
    {
        auto childElement = element->getFirstChildElement(childElementName);
        if (!childElement) {
            throw std::invalid_argument("Missing <" + childElementName + "> element.");
        }
        return childElement;
    }

    /** @brief Helper function to return std::nullopt when child element does not exist. */
    static std::optional<std::string> getOptionalChildText(const XmlElementPtr& element, const std::string& childElementName)
    {
        auto childElement = element->getFirstChildElement(childElementName);
        if (!childElement) {
            return std::nullopt;
        }
        return childElement->getText();
    }

    /** @brief Helper function to return std::nullopt when child element does not exist. */
    template<typename T>
    static std::optional<T> getOptionalChildTextAs(const XmlElementPtr& element, const std::string& childElementName, T defaultValue = {})
    {
        auto childElement = element->getFirstChildElement(childElementName);
        if (!childElement) {
            return std::nullopt;
        }
        return childElement->getTextAs<T>(defaultValue);
    }

public:
    virtual ~FactoryBase() {}
};

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

template <typename EnumClass, typename FromClass = std::string_view>
using XmlEnumMappingElement = std::unordered_map<FromClass, EnumClass>;
template <typename EnumClass, typename FromClass = std::string_view>
struct XmlEnumMapping
{
    static const XmlEnumMappingElement<EnumClass, FromClass> mapping;
};

#define MUSX_XML_ENUM_MAPPING(Type, ...) \
template <> \
const XmlEnumMappingElement<Type> XmlEnumMapping<Type>::mapping = __VA_ARGS__

template <typename EnumClass, typename FromClass = std::string_view>
class EnumMapper
{
    // If we ever need to, we can create a static lazy-initialize reverse mapping function here

public:
    static EnumClass xmlToEnum(const FromClass& value)
    {
        auto it = XmlEnumMapping<EnumClass>::mapping.find(value);
        if (it != XmlEnumMapping<EnumClass>::mapping.end()) {
            return it->second;
        }
        std::string msg = [value]() {
            if constexpr (std::is_arithmetic_v<FromClass>) {
                return "Invalid enum value from xml: " + std::to_string(value);
            }
            else {
                return "Invalid enum value from xml: " + std::string(value);
            }
        }();
#ifdef MUSX_THROW_ON_UNKNOWN_XML
        throw std::invalid_argument(msg);
#else
        util::Logger::log(util::Logger::LogLevel::Error, msg);
        return {};
#endif
    }
};

template<typename EnumClass, typename FromClass>
EnumClass toEnum(const FromClass& value)
{
    if constexpr (std::is_convertible_v<FromClass, std::string_view>) {
        return EnumMapper<EnumClass, std::string_view>::xmlToEnum(value);
    } else {
        return EnumMapper<EnumClass, FromClass>::xmlToEnum(value);
    }
}

#define MUSX_XML_ELEMENT_ARRAY(Type, ...) \
const ::musx::xml::XmlElementArray<Type> Type::XmlMappingArray = __VA_ARGS__

using ResolverList = std::vector<ElementLinker::Resolver>;
template <typename T>
struct ResolverArray
{
    inline static const ResolverList value = {};
};

#define MUSX_RESOLVER_ARRAY(Type, ...) \
template <> \
struct ResolverArray<Type> { \
    inline static const ResolverList value = __VA_ARGS__; \
};

template <typename T>
struct FieldPopulator : public FactoryBase
{
    static void populate(const std::shared_ptr<T>& instance, const XmlElementPtr& element)
    {
        for (auto child = element->getFirstChildElement(); child; child = child->getNextSibling()) {
            auto it = elementXref().find(child->getTagName());
            if (it != elementXref().end()) {
                std::get<1>(*it)(child, instance);
            }
            else {
                const bool requireField = [instance]() {
                    if constexpr (std::is_base_of_v<Base, T>) {
                        return instance->requireAllFields();
                    }
                    else {
                        return true;
                    }
                }();
                if (requireField) {
                    std::string msg = "xml element <" + element->getTagName() + "> has child <" + child->getTagName() + "> which is not in the element list.";
#ifdef MUSX_THROW_ON_UNKNOWN_XML
                    throw std::invalid_argument(msg);
#else
                    util::Logger::log(util::Logger::LogLevel::Error, msg);
#endif
                }
            }
        }
    }

    static void populate(const std::shared_ptr<T>& instance, const XmlElementPtr& element, ElementLinker& elementLinker)
    {
        populate(instance, element);
        for (const auto& resolver : ResolverArray<T>::value) {
            elementLinker.addResolver(resolver);
        }
    }

    template <typename... Args>
    static std::shared_ptr<T> createAndPopulate(const XmlElementPtr& element, Args&&... args)
    {
       return FieldPopulator<T>::createAndPopulateImpl(element, std::forward<Args>(args)...);
    }

private:
    static const std::unordered_map<std::string_view, XmlElementPopulator<T>>& elementXref()
    {
        static const std::unordered_map<std::string_view, XmlElementPopulator<T>> xref = []()
            {
                std::unordered_map<std::string_view, XmlElementPopulator<T>> retval;
                for (std::size_t i = 0; i < T::XmlMappingArray.size(); i++) {
                    const XmlElementDescriptor<T> descriptor = T::XmlMappingArray[i];
                    retval[std::get<0>(descriptor)] = std::get<1>(descriptor);
                }
                return retval;
            }();
        return xref;
    }

    template <typename... Args>
    static std::shared_ptr<T> createAndPopulateImpl(const XmlElementPtr& element, Args&&... args)
    {
        auto instance = std::make_shared<T>(std::forward<Args>(args)...);
        FieldPopulator<T>::populate(instance, element);
        return instance;
    }
};

template <typename EnumClass, typename EmbeddedClass>
static void populateEmbeddedClass(const XmlElementPtr& e, std::unordered_map<EnumClass, std::shared_ptr<EmbeddedClass>>& listArray)
{
    auto typeAttr = e->findAttribute("type");
    if (!typeAttr) {
        throw std::invalid_argument("<" + e->getTagName() + "> element has no type attribute");
    }
    listArray.emplace(toEnum<EnumClass>(typeAttr->getValueTrimmed()), FieldPopulator<EmbeddedClass>::createAndPopulate(e));
}

template <>
template <typename... Args>
inline std::shared_ptr<FontInfo> FieldPopulator<FontInfo>::createAndPopulate(const XmlElementPtr& element, Args&&... args)
{
    if (!element->getFirstChildElement()) return nullptr;
    return FieldPopulator<FontInfo>::createAndPopulateImpl(element, std::forward<Args>(args)...);
}

#endif // DOXYGEN_SHOULD_IGNORE_THIS

} // namespace factory
} // namespace musx
