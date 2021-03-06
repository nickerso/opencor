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
// CellML Text view widget
//==============================================================================

#ifndef CELLMLTEXTVIEWWIDGET_H
#define CELLMLTEXTVIEWWIDGET_H

//==============================================================================

#include "cellmlfile.h"
#include "cellmltextviewconverter.h"
#include "cellmltextviewparser.h"
#include "corecliutils.h"
#include "viewwidget.h"

//==============================================================================

#include <QMap>

//==============================================================================

namespace Ui {
    class CellmlTextViewWidget;
}

//==============================================================================

namespace OpenCOR {

//==============================================================================

namespace Core {
    class XslTransformer;
}   // namespace Core

//==============================================================================

namespace CoreCellMLEditing {
    class CoreCellmlEditingWidget;
}   // namespace CoreCellMLEditing

//==============================================================================

namespace Editor {
    class EditorWidget;
}   // namespace Editor

//==============================================================================

namespace EditorList {
    class EditorListWidget;
}   // namespace EditorList

//==============================================================================

namespace QScintillaSupport {
    class QScintillaWidget;
}   // namespace QScintillaSupport

//==============================================================================

namespace CellMLTextView {

//==============================================================================

class CellmlTextViewWidgetData
{
public:
    explicit CellmlTextViewWidgetData(CoreCellMLEditing::CoreCellmlEditingWidget *pEditingWidget = 0,
                                      const QString &pSha1 = QString(),
                                      const bool &pValid = false,
                                      const CellMLSupport::CellmlFile::Version &pCellmlVersion = CellMLSupport::CellmlFile::Cellml_1_0,
                                      QDomDocument pRdfNodes = QDomDocument());

    CoreCellMLEditing::CoreCellmlEditingWidget * editingWidget() const;
    QString sha1() const;
    bool isValid() const;
    CellMLSupport::CellmlFile::Version cellmlVersion() const;
    QDomDocument rdfNodes() const;

    void setSha1(const QString &pSha1);

private:
    CoreCellMLEditing::CoreCellmlEditingWidget *mEditingWidget;
    QString mSha1;
    bool mValid;
    CellMLSupport::CellmlFile::Version mCellmlVersion;
    QDomDocument mRdfNodes;
};

//==============================================================================

class CellmlTextViewWidget : public Core::ViewWidget
{
    Q_OBJECT

public:
    explicit CellmlTextViewWidget(QWidget *pParent);
    ~CellmlTextViewWidget();

    virtual void loadSettings(QSettings *pSettings);
    virtual void saveSettings(QSettings *pSettings) const;

    virtual void retranslateUi();

    bool contains(const QString &pFileName) const;

    void initialize(const QString &pFileName, const bool &pUpdate = true);
    void finalize(const QString &pFileName);

    void fileReloaded(const QString &pFileName);
    void fileRenamed(const QString &pOldFileName, const QString &pNewFileName);

    Editor::EditorWidget * editor(const QString &pFileName) const;

    bool isEditorUseable(const QString &pFileName) const;
    bool isEditorContentsModified(const QString &pFileName) const;

    bool saveFile(const QString &pOldFileName, const QString &pNewFileName);

    virtual QList<QWidget *> statusBarWidgets() const;

    void reformat(const QString &pFileName);

    bool validate(const QString &pFileName);

private:
    Ui::CellmlTextViewWidget *mGui;

    bool mNeedLoadingSettings;
    QString mSettingsGroup;

    CoreCellMLEditing::CoreCellmlEditingWidget *mEditingWidget;

    QMap<QString, CellmlTextViewWidgetData> mData;

    CellMLTextViewConverter mConverter;
    CellmlTextViewParser mParser;

    QList<EditorList::EditorListWidget *> mEditorLists;

    QMap<QString, QString> mPresentationMathmlEquations;

    Core::XslTransformer *mXslTransformer;

    QString mContentMathmlEquation;

    void commentOrUncommentLine(QScintillaSupport::QScintillaWidget *editor,
                                const int &pLineNumber,
                                const bool &pCommentLine);

    bool parse(const QString &pFileName, const bool &pOnlyErrors = false);

    QString partialStatement(const int &pPosition, int &pFromPosition,
                             int &pToPosition) const;
    QString beginningOfPiecewiseStatement(int &pPosition) const;
    QString endOfPiecewiseStatement(int &pPosition) const;
    QString statement(const int &pPosition) const;

private Q_SLOTS:
    void editorKeyPressed(QKeyEvent *pEvent, bool &pHandled);

    void updateViewer();

    void selectFirstItemInEditorList(EditorList::EditorListWidget *pEditorList = 0);

    void xslTransformationDone(const QString &pInput, const QString &pOutput);
};

//==============================================================================

}   // namespace CellMLTextView
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
