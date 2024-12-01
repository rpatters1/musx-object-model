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

/** @mainpage
 * Welcome to the <b>Musx Document Model</b>.
 *
 * Use the <b>navigation buttons</b> at the top of this page to browse through
 * the framework documentation.
 *
 * The >Musx Document Model is a header-only <b>C++ class framework</b>
 * around the EnigmaXml format embedded in `.musx` files generated by Finale music software.
 * It requires the C++17 standard.
 * 
 * Features include:
 * - Header-only implementation.
 * - Dependency-free class definitions.
 * - Xml Deserializer interfaces that allow the caller to use any Xml utility for xml handling.
 *
 * (This documentation reference is generated directly from
 * the source code, by the Doxygen application.)
 *
 * @author Robert Patterson
 */

#pragma once

#include "util/ScoreFileEncoder.h"
#include "xml/XmlInterface.h"
#include "dom/Document.h"
#include "dom/Implementations.h"
#include "factory/DocumentFactory.h"

#ifdef MUSX_USE_TINYXML2 // usually defined on the compile line or in CMakeLists.txt
#include "xml/TinyXmlImpl.h"
#endif

#ifdef MUSX_USE_RAPIDXML // usually defined on the compile line or in CMakeLists.txt
// ToDo: provide this implementation
#include "xml/RapidXmlImpl.h"
#endif
