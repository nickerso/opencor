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
// GUI utilities
//==============================================================================

#ifndef GUIUTILS_H
#define GUIUTILS_H

//==============================================================================

#include "coreglobal.h"

//==============================================================================

#include <QColor>
#include <QKeySequence>
#include <QString>

//==============================================================================

class QAction;
class QFrame;
class QLabel;
class QMenu;
class QWidget;

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

static const auto SettingsBaseColor      = QStringLiteral("BaseColor");
static const auto SettingsBorderColor    = QStringLiteral("BorderColor");
static const auto SettingsHighlightColor = QStringLiteral("HighlightColor");
static const auto SettingsShadowColor    = QStringLiteral("Shadow");
static const auto SettingsWindowColor    = QStringLiteral("WindowColor");

//==============================================================================

QString CORE_EXPORT getOpenFileName(const QString &pCaption,
                                    const QString &pFilter);
QStringList CORE_EXPORT getOpenFileNames(const QString &pCaption,
                                         const QString &pFilter);
QString CORE_EXPORT getSaveFileName(const QString &pCaption,
                                    const QString &pFileName,
                                    const QString &pFilter);

void CORE_EXPORT setFocusTo(QWidget *pWidget);

QMenu CORE_EXPORT * newMenu(const QString &pName, QWidget *pParent = 0);
QMenu CORE_EXPORT * newMenu(const QIcon &pIcon, QWidget *pParent = 0);

QAction CORE_EXPORT * newAction(const bool &pCheckable, const QIcon &pIcon,
                                const QKeySequence &pKeySequence,
                                QWidget *pParent);
QAction CORE_EXPORT * newAction(const bool &pCheckable,
                                const QKeySequence &pKeySequence,
                                QWidget *pParent = 0);
QAction CORE_EXPORT * newAction(const bool &pCheckable, QWidget *pParent = 0);
QAction CORE_EXPORT * newAction(const QIcon &pIcon,
                                const QList<QKeySequence> &pKeySequences,
                                QWidget *pParent = 0);
QAction CORE_EXPORT * newAction(const QIcon &pIcon,
                                const QKeySequence &pKeySequence,
                                QWidget *pParent = 0);
QAction CORE_EXPORT * newAction(const QIcon &pIcon, QWidget *pParent = 0);
QAction CORE_EXPORT * newAction(const QKeySequence &pKeySequence,
                                QWidget *pParent = 0);
QAction CORE_EXPORT * newAction(const QKeySequence::StandardKey &pStandardKey,
                                QWidget *pParent = 0);

QFrame CORE_EXPORT * newLineWidget(const bool &pHorizontal,
                                   const QColor &pColor, QWidget *pParent);
QFrame CORE_EXPORT * newLineWidget(const bool &pHorizontal, QWidget *pParent);
QFrame CORE_EXPORT * newLineWidget(const QColor &pColor, QWidget *pParent);
QFrame CORE_EXPORT * newLineWidget(QWidget *pParent);

QLabel CORE_EXPORT * newLabel(const QString &pText,
                              const double &pFontPercentage,
                              const bool &pBold, const bool &pItalic,
                              const Qt::Alignment &pAlignment,
                              QWidget *pParent);
QLabel CORE_EXPORT * newLabel(const QString &pText,
                              const double &pFontPercentage,
                              const bool &pBold, const bool &pItalic,
                              QWidget *pParent);
QLabel CORE_EXPORT * newLabel(const QString &pText,
                              const double &pFontPercentage,
                              const bool &pBold, QWidget *pParent);
QLabel CORE_EXPORT * newLabel(const QString &pText,
                              const double &pFontPercentage, QWidget *pParent);
QLabel CORE_EXPORT * newLabel(const QString &pText, QWidget *pParent);

void CORE_EXPORT showEnableAction(QAction *pAction, const bool &pShowEnable);
void CORE_EXPORT showEnableWidget(QWidget *pWidget, const bool &pShowEnable);

void CORE_EXPORT updateColors();

QColor CORE_EXPORT baseColor();
QColor CORE_EXPORT borderColor();
QColor CORE_EXPORT highlightColor();
QColor CORE_EXPORT shadowColor();
QColor CORE_EXPORT windowColor();

QColor CORE_EXPORT lockedColor(const QColor &pColor);

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================