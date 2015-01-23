/*******************************************************************************

Licensed to the OpenCOR team under one or more contributor license agreements.
See the NOTICE.txt file distributed with this work for additional information
regarding copyright ownership. The OpenCOR team licenses this file to you under
the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

*******************************************************************************/

//==============================================================================
// CellML to pretty CellML converter
//==============================================================================

#include "corecliutils.h"
#include "prettycellmlviewcellmltoprettycellmlconverter.h"

//==============================================================================

#include <QDomDocument>

//==============================================================================

namespace OpenCOR {
namespace PrettyCellMLView {

//==============================================================================

PrettyCellMLViewCellmlToPrettyCellmlConverter::PrettyCellMLViewCellmlToPrettyCellmlConverter(const QString &pFileName) :
    mFileName(pFileName),
    mOutput(QString()),
    mIndent(QString()),
    mLastOutputType(None),
    mErrorLine(-1),
    mErrorColumn(-1),
    mErrorMessage(QString()),
    mWarnings(QStringList())
{
}

//==============================================================================

bool PrettyCellMLViewCellmlToPrettyCellmlConverter::execute()
{
    // Convert the CellML file to pretty CellML by first getting a DOM
    // representation of the file

    QString fileContents;
    QDomDocument domDocument;

    Core::readTextFromFile(mFileName, fileContents);

    if (domDocument.setContent(fileContents, &mErrorMessage, &mErrorLine, &mErrorColumn)) {
        mOutput = QString();
        mIndent = QString();

        if (!processModelNode(domDocument.documentElement())) {
            mOutput = fileContents;

            return false;
        } else {
            return true;
        }
    } else {
        mOutput = fileContents;

        return false;
    }
}

//==============================================================================

QString PrettyCellMLViewCellmlToPrettyCellmlConverter::output() const
{
    // Return our output

    return mOutput;
}

//==============================================================================

int PrettyCellMLViewCellmlToPrettyCellmlConverter::errorLine() const
{
    // Return our error line

    return mErrorLine;
}

//==============================================================================

int PrettyCellMLViewCellmlToPrettyCellmlConverter::errorColumn() const
{
    // Return our error column

    return mErrorColumn;
}

//==============================================================================

QString PrettyCellMLViewCellmlToPrettyCellmlConverter::errorMessage() const
{
    // Return our error message

    return mErrorMessage;
}

//==============================================================================

bool PrettyCellMLViewCellmlToPrettyCellmlConverter::hasWarnings() const
{
    // Return whether we have warnings

    return mWarnings.count();
}

//==============================================================================

QStringList PrettyCellMLViewCellmlToPrettyCellmlConverter::warnings() const
{
    // Return our warnings

    return mWarnings;
}

//==============================================================================

static const auto Indent = QStringLiteral("    ");

//==============================================================================

void PrettyCellMLViewCellmlToPrettyCellmlConverter::indent()
{
    // Indent our output

    mIndent += Indent;
}

//==============================================================================

void PrettyCellMLViewCellmlToPrettyCellmlConverter::unindent()
{
    // Unindent our output

    mIndent.chop(Indent.size());
}

//==============================================================================

void PrettyCellMLViewCellmlToPrettyCellmlConverter::outputString(const OutputType &pOutputType,
                                                                 const QString &pString)
{
    // Output the given string

    if (pString.isEmpty()) {
        if (mLastOutputType != EmptyLine)
            mOutput += "\n";
    } else {
        mOutput += mIndent+pString+"\n";
    }

    mLastOutputType = pOutputType;
}

//==============================================================================

static const auto CmetaId = QStringLiteral("cmeta:id");

//==============================================================================

QString PrettyCellMLViewCellmlToPrettyCellmlConverter::cmetaId(const QDomNode &pDomNode) const
{
    // Return the cmeta:id, if any, of the given node

    if (pDomNode.attributes().contains(CmetaId))
        return QString("{%1}").arg(pDomNode.attributes().namedItem(CmetaId).nodeValue());
    else
        return QString();
}

//==============================================================================

bool PrettyCellMLViewCellmlToPrettyCellmlConverter::processModelNode(const QDomNode &pDomNode)
{
    // Start processing the given model node

    outputString(DefModel,
                 QString("def model%1 %2 as").arg(cmetaId(pDomNode))
                                             .arg(pDomNode.attributes().namedItem("name").nodeValue()));

    indent();

    // Process the given model node's children

    QString nodeName;

    for (QDomNode domNode = pDomNode.firstChild();
         !domNode.isNull(); domNode = domNode.nextSibling()) {
        nodeName = domNode.nodeName();

        if (!nodeName.compare("#comment")) {
            if (!processCommentNode(domNode))
                return false;
        } else if (!nodeName.compare("rdf:RDF")) {
            if (!processRdfNode(domNode))
                return false;
        } else if (!nodeName.compare("import")) {
            if (!processImportNode(domNode))
                return false;
        } else if (!nodeName.compare("units")) {
            if (!processUnitsNode(domNode))
                return false;
        } else if (!nodeName.compare("component")) {
            if (!processComponentNode(domNode))
                return false;
        } else if (!nodeName.compare("group")) {
            if (!processGroupNode(domNode))
                return false;
        } else if (!nodeName.compare("connection")) {
            if (!processConnectionNode(domNode))
                return false;
        } else if (!processUnknownNode(domNode)) {
            return false;
        }
    }

    // Finish processing the given model node

    unindent();

    outputString(EndDef, "enddef;");

    return true;
}

//==============================================================================

bool PrettyCellMLViewCellmlToPrettyCellmlConverter::processCommentNode(const QDomNode &pDomNode)
{
    // Process the given comment node

    QStringList commentLines = pDomNode.nodeValue().remove("\r").split("\n");
    // Note: we don't know which end of line string is used by the file uses, so
    //       the above code ensures that we can handle both "\r\n" on Windows
    //       and "\n" on Linux / OS X...

    if (   (mLastOutputType == Comment)
        || (mLastOutputType == ImportUnit) || (mLastOutputType == ImportComp)
        || (mLastOutputType == DefBaseUnit) || (mLastOutputType == EndDef)
        || (mLastOutputType == Unit) || (mLastOutputType == Var)) {
        outputString();
    }

    foreach (const QString commentLine, commentLines)
        outputString(Comment, QString("// %1").arg(commentLine));

    return true;
}

//==============================================================================

bool PrettyCellMLViewCellmlToPrettyCellmlConverter::processRdfNode(const QDomNode &pDomNode)
{
    // Process the given RDF node

//---GRY--- TO BE DONE...
Q_UNUSED(pDomNode);
qWarning("RDF node: not yet implemented...");

    return true;
}

//==============================================================================

bool PrettyCellMLViewCellmlToPrettyCellmlConverter::processImportNode(const QDomNode &pDomNode)
{
    // Start processing the given import node

    if ((mLastOutputType == Comment) || (mLastOutputType == EndDef))
        outputString();

    outputString(DefImport,
                 QString("def import%1 using \"%2\" for").arg(cmetaId(pDomNode))
                                                         .arg(pDomNode.attributes().namedItem("xlink:href").nodeValue()));

    indent();

    // Process the given import node's children

    QString nodeName;

    for (QDomNode domNode = pDomNode.firstChild();
         !domNode.isNull(); domNode = domNode.nextSibling()) {
        nodeName = domNode.nodeName();

        if (!nodeName.compare("#comment")) {
            if (!processCommentNode(domNode))
                return false;
        } else if (!nodeName.compare("rdf:RDF")) {
            if (!processRdfNode(domNode))
                return false;
        } else if (!nodeName.compare("units")) {
            if (!processUnitsNode(domNode, true))
                return false;
        } else if (!nodeName.compare("component")) {
            if (!processComponentNode(domNode, true))
                return false;
        } else if (!processUnknownNode(domNode)) {
            return false;
        }
    }

    // Finish processing the given group node

    unindent();

    outputString(EndDef, "enddef;");

    return true;
}

//==============================================================================

bool PrettyCellMLViewCellmlToPrettyCellmlConverter::processUnitsNode(const QDomNode &pDomNode,
                                                                     const bool &pInImportNode)
{
    // Start processing the given units node

    QString baseUnits = pDomNode.attributes().namedItem("base_units").nodeValue();

    if (baseUnits.compare(QString()) && baseUnits.compare("yes") && baseUnits.compare("no")) {
        mErrorLine = pDomNode.lineNumber();
        mErrorMessage = QObject::tr("The 'base_units' attribute must have a value equal to 'yes' or 'no'.");

        return false;
    }

    bool isBaseUnits = !baseUnits.compare("yes");

    if (!pInImportNode && !isBaseUnits) {
        if (   (mLastOutputType == Comment) || (mLastOutputType == EndDef)
            || (mLastOutputType == DefBaseUnit)) {
            outputString();
        }

        outputString(DefUnit,
                     QString("def unit%1 %2 as").arg(cmetaId(pDomNode))
                                                .arg(pDomNode.attributes().namedItem("name").nodeValue()));

        indent();
    }

    // Process the given units node's children

    QString nodeName;

    for (QDomNode domNode = pDomNode.firstChild();
         !domNode.isNull(); domNode = domNode.nextSibling()) {
        nodeName = domNode.nodeName();

        if (!nodeName.compare("#comment")) {
            if (!processCommentNode(domNode))
                return false;
        } else if (!nodeName.compare("rdf:RDF")) {
            if (!processRdfNode(domNode))
                return false;
        } else if (   !isBaseUnits && !pInImportNode
                   && !nodeName.compare("unit")) {
            if (!processUnitNode(domNode))
                return false;
        } else if (!processUnknownNode(domNode)) {
            return false;
        }
    }

    // Finish processing the given units node

    if (pInImportNode) {
        if ((mLastOutputType == Comment) || (mLastOutputType == ImportComp))
            outputString();

        outputString(ImportUnit,
                     QString("unit%1 %2 using unit %3;").arg(cmetaId(pDomNode))
                                                        .arg(pDomNode.attributes().namedItem("name").nodeValue())
                                                        .arg(pDomNode.attributes().namedItem("units_ref").nodeValue()));
    } else if (isBaseUnits) {
        if ((mLastOutputType == Comment) || (mLastOutputType == EndDef))
            outputString();

        outputString(DefBaseUnit,
                     QString("def unit%1 %2 as base unit;").arg(cmetaId(pDomNode))
                                                           .arg(pDomNode.attributes().namedItem("name").nodeValue()));
    } else {
        unindent();

        outputString(EndDef, "enddef;");
    }

    return true;
}

//==============================================================================

bool PrettyCellMLViewCellmlToPrettyCellmlConverter::processUnitNode(const QDomNode &pDomNode)
{
    // Process the given unit node's children
    // Note #1: unlike for most nodes, we process the node's children before
    //          processing the node itself since it only outputs one line, so
    //          it's not like we can put a comment within a def...enddef;
    //          statement...
    // Note #2: when parsed back, comments will not be in the unit node anymore,
    //          but its parent one since we don't have a def...enddef;
    //          statement...

    QString nodeName;

    for (QDomNode domNode = pDomNode.firstChild();
         !domNode.isNull(); domNode = domNode.nextSibling()) {
        nodeName = domNode.nodeName();

        if (!nodeName.compare("#comment")) {
            if (!processCommentNode(domNode))
                return false;
        } else if (!nodeName.compare("rdf:RDF")) {
            if (!processRdfNode(domNode))
                return false;
        } else if (!processUnknownNode(domNode)) {
            return false;
        }
    }

    // Process the given unit node

    QString parameters = QString();

    if (!pDomNode.attributes().namedItem("prefix").nodeValue().isEmpty())
        parameters += "pref: "+pDomNode.attributes().namedItem("prefix").nodeValue();

    if (!pDomNode.attributes().namedItem("exponent").nodeValue().isEmpty()) {
        if (!parameters.isEmpty())
            parameters += ", ";

        parameters += "expo: "+pDomNode.attributes().namedItem("exponent").nodeValue();
    }

    if (!pDomNode.attributes().namedItem("multiplier").nodeValue().isEmpty()) {
        if (!parameters.isEmpty())
            parameters += ", ";

        parameters += "mult: "+pDomNode.attributes().namedItem("multiplier").nodeValue();
    }

    if (!pDomNode.attributes().namedItem("offset").nodeValue().isEmpty()) {
        if (!parameters.isEmpty())
            parameters += ", ";

        parameters += "off: "+pDomNode.attributes().namedItem("offset").nodeValue();
    }

    if (mLastOutputType == Comment)
        outputString();

    outputString(Unit,
                 QString("unit%1 %2%3;").arg(cmetaId(pDomNode))
                                        .arg(pDomNode.attributes().namedItem("units").nodeValue())
                                        .arg(parameters.isEmpty()?QString():" {"+parameters+"}"));

    return true;
}

//==============================================================================

bool PrettyCellMLViewCellmlToPrettyCellmlConverter::processComponentNode(const QDomNode &pDomNode,
                                                                         const bool &pInImportNode)
{
    // Start processing the given component node

    if (!pInImportNode) {
        if ((mLastOutputType == Comment) || (mLastOutputType == EndDef))
            outputString();

        outputString(DefComp,
                     QString("def comp%1 %2 as").arg(cmetaId(pDomNode))
                                                .arg(pDomNode.attributes().namedItem("name").nodeValue()));

        indent();
    }

    // Process the given component node's children

    QString nodeName;

    for (QDomNode domNode = pDomNode.firstChild();
         !domNode.isNull(); domNode = domNode.nextSibling()) {
        nodeName = domNode.nodeName();

        if (!nodeName.compare("#comment")) {
            if (!processCommentNode(domNode))
                return false;
        } else if (!nodeName.compare("rdf:RDF")) {
            if (!processRdfNode(domNode))
                return false;
        } else if (!pInImportNode && !nodeName.compare("units")) {
            if (!processUnitsNode(domNode))
                return false;
        } else if (!pInImportNode && !nodeName.compare("variable")) {
            if (!processVariableNode(domNode))
                return false;
        } else if (!pInImportNode && !nodeName.compare("math")) {
            if (!processMathNode(domNode))
                return false;
        } else if (!processUnknownNode(domNode)) {
            return false;
        }
    }

    // Finish processing the given component node

    if (pInImportNode) {
        if ((mLastOutputType == Comment) || (mLastOutputType == ImportUnit))
            outputString();

        outputString(ImportComp,
                     QString("comp%1 %2 using comp %3;").arg(cmetaId(pDomNode))
                                                        .arg(pDomNode.attributes().namedItem("name").nodeValue())
                                                        .arg(pDomNode.attributes().namedItem("component_ref").nodeValue()));
    } else {
        unindent();

        outputString(EndDef, "enddef;");
    }

    return true;
}

//==============================================================================

bool PrettyCellMLViewCellmlToPrettyCellmlConverter::processVariableNode(const QDomNode &pDomNode)
{
    // Process the given variable node's children
    // Note #1: unlike for most nodes, we process the node's children before
    //          processing the node itself since it only outputs one line, so
    //          it's not like we can put a comment within a def...enddef;
    //          statement...
    // Note #2: when parsed back, comments will not be in the variable node
    //          anymore, but its parent one since we don't have a def...enddef;
    //          statement...

    QString nodeName;

    for (QDomNode domNode = pDomNode.firstChild();
         !domNode.isNull(); domNode = domNode.nextSibling()) {
        nodeName = domNode.nodeName();

        if (!nodeName.compare("#comment")) {
            if (!processCommentNode(domNode))
                return false;
        } else if (!nodeName.compare("rdf:RDF")) {
            if (!processRdfNode(domNode))
                return false;
        } else if (!processUnknownNode(domNode)) {
            return false;
        }
    }

    // Process the given variable node

    QString parameters = QString();

    if (!pDomNode.attributes().namedItem("initial_value").nodeValue().isEmpty())
        parameters += "init: "+pDomNode.attributes().namedItem("initial_value").nodeValue();

    if (!pDomNode.attributes().namedItem("public_interface").nodeValue().isEmpty()) {
        if (!parameters.isEmpty())
            parameters += ", ";

        parameters += "pub: "+pDomNode.attributes().namedItem("public_interface").nodeValue();
    }

    if (!pDomNode.attributes().namedItem("private_interface").nodeValue().isEmpty()) {
        if (!parameters.isEmpty())
            parameters += ", ";

        parameters += "priv: "+pDomNode.attributes().namedItem("private_interface").nodeValue();
    }

    if (   (mLastOutputType == Comment)
        || (mLastOutputType == DefBaseUnit) || (mLastOutputType == EndDef)) {
        outputString();
    }

    outputString(Var,
                 QString("var%1 %2: %3%4;").arg(cmetaId(pDomNode))
                                           .arg(pDomNode.attributes().namedItem("name").nodeValue())
                                           .arg(pDomNode.attributes().namedItem("units").nodeValue())
                                           .arg(parameters.isEmpty()?QString():" {"+parameters+"}"));

    return true;
}

//==============================================================================

bool PrettyCellMLViewCellmlToPrettyCellmlConverter::processMathNode(const QDomNode &pDomNode)
{
    // Process the given math node

//---GRY--- TO BE DONE...
Q_UNUSED(pDomNode);
qWarning("Math node: not yet implemented...");

    return true;
}

//==============================================================================

bool PrettyCellMLViewCellmlToPrettyCellmlConverter::processGroupNode(const QDomNode &pDomNode)
{
//---GRY--- TO BE DONE...
qWarning("Group node: not yet fully implemented...");
    // Start processing the given group node

    if ((mLastOutputType == Comment) || (mLastOutputType == EndDef))
        outputString();

    outputString(DefGroup, QString("def group%1 as").arg(cmetaId(pDomNode)));

    indent();

    // Process the given group node's children

    QString nodeName;

    for (QDomNode domNode = pDomNode.firstChild();
         !domNode.isNull(); domNode = domNode.nextSibling()) {
        nodeName = domNode.nodeName();

        if (!nodeName.compare("#comment")) {
            if (!processCommentNode(domNode))
                return false;
        } else if (!nodeName.compare("rdf:RDF")) {
            if (!processRdfNode(domNode))
                return false;
        } else if (!processUnknownNode(domNode)) {
            return false;
        }
    }

    // Finish processing the given group node

    unindent();

    outputString(EndDef, "enddef;");

    return true;
}

//==============================================================================

bool PrettyCellMLViewCellmlToPrettyCellmlConverter::processConnectionNode(const QDomNode &pDomNode)
{
//---GRY--- TO BE DONE...
qWarning("Connection node: not yet fully implemented...");
    // Start processing the given connection node

    if ((mLastOutputType == Comment) || (mLastOutputType == EndDef))
        outputString();

    outputString(DefMap,
                 QString("def map%1 as").arg(cmetaId(pDomNode)));

    indent();

    // Process the given connection node's children

    QString nodeName;

    for (QDomNode domNode = pDomNode.firstChild();
         !domNode.isNull(); domNode = domNode.nextSibling()) {
        nodeName = domNode.nodeName();

        if (!nodeName.compare("#comment")) {
            if (!processCommentNode(domNode))
                return false;
        } else if (!nodeName.compare("rdf:RDF")) {
            if (!processRdfNode(domNode))
                return false;
        } else if (!processUnknownNode(domNode)) {
            return false;
        }
    }

    // Finish processing the given group node

    unindent();

    outputString(EndDef, "enddef;");

    return true;
}

//==============================================================================

bool PrettyCellMLViewCellmlToPrettyCellmlConverter::processUnknownNode(const QDomNode &pDomNode)
{
    // The given node is unknown, so warn the user about it

    mWarnings << QObject::tr("[%1] The '%2' node was found in the original CellML file, but it is not known and cannot therefore be processed.").arg(pDomNode.lineNumber())
                                                                                                                                                .arg(pDomNode.nodeName());

    return true;
}

//==============================================================================

}   // namespace PrettyCellMLView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
