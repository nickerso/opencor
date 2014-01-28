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
// CellML annotation view CellML list widget
//==============================================================================

#include "cellmlannotationviewcellmllistwidget.h"
#include "cellmlannotationviewwidget.h"
#include "filemanager.h"
#include "treeviewwidget.h"

//==============================================================================

#include "ui_cellmlannotationviewcellmllistwidget.h"

//==============================================================================

#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QStandardItemModel>

//==============================================================================

#include <QtSingleApplication>

//==============================================================================

namespace OpenCOR {
namespace CellMLAnnotationView {

//==============================================================================

void CellmlAnnotationViewCellmlElementItemDelegate::paint(QPainter *pPainter,
                                                          const QStyleOptionViewItem &pOption,
                                                          const QModelIndex &pIndex) const
{
    // Paint the item as normal, except for error/warning/category items

    CellmlAnnotationViewCellmlElementItem *cellmlElementItem = static_cast<CellmlAnnotationViewCellmlElementItem *>(qobject_cast<const QStandardItemModel *>(pIndex.model())->itemFromIndex(pIndex));

    QStyleOptionViewItemV4 option(pOption);

    initStyleOption(&option, pIndex);

    if (   (cellmlElementItem->type() == CellmlAnnotationViewCellmlElementItem::Error)
        || (cellmlElementItem->type() == CellmlAnnotationViewCellmlElementItem::Warning)) {
        // This is an error/warning item, so prevent it from hoverable and make
        // it look enabled since it's actually disabled (so we can't select it),
        // yet we want to see as if it was enabled, so...

        option.state &= ~QStyle::State_MouseOver;
        option.state |=  QStyle::State_Enabled;
    } else if (cellmlElementItem->isCategory()) {
        // We are dealing with a category so show it bold

        option.font.setBold(cellmlElementItem->isCategory());
    }

    QStyledItemDelegate::paint(pPainter, option, pIndex);
}

//==============================================================================

void CellmlAnnotationViewCellmlElementItem::constructor(const bool &pCategory,
                                                        const Type &pType,
                                                        iface::cellml_api::CellMLElement *pElement,
                                                        const int &pNumber)
{
    // Some initialisations

    mCategory = pCategory;
    mType = pType;

    mElement = pElement;

    mNumber = pNumber;
}

//==============================================================================

CellmlAnnotationViewCellmlElementItem::CellmlAnnotationViewCellmlElementItem(const bool &pError, const QString &pText) :
    QStandardItem(pText)
{
    // Constructor for either an error or a warning

    constructor(false, pError?Error:Warning, 0, -1);

    // Disable the item and use its text as a tooltip (in case it's too long and
    // doesn't fit within the allocated space we have)
    // Note: the item will get 're-enabled' by our item delegate...

    setEnabled(false);
    setToolTip(pText);

    // Set the icon for the item

    setIcon(mType);
}

//==============================================================================

CellmlAnnotationViewCellmlElementItem::CellmlAnnotationViewCellmlElementItem(const Type &pType,
                                                                             const QString &pText) :
    QStandardItem(pText)
{
    // Constructor for a category

    constructor(true, pType, 0, -1);

    // Use its text as a tooltip (in case it's too long and doesn't fit within
    // the allocated space we have)

    setToolTip(pText);

    // Set the icon for the item

    setIcon(pType);
}

//==============================================================================

CellmlAnnotationViewCellmlElementItem::CellmlAnnotationViewCellmlElementItem(const Type &pType,
                                                                             iface::cellml_api::CellMLElement *pElement,
                                                                             const int pNumber) :
    QStandardItem()
{
    // Constructor for a CellML element

    constructor(false, pType, pElement, pNumber);

    // Set the text for some types

    enum {
        RightArrow = 0x2192
    };

    switch (pType) {
    case Import: {
        ObjRef<iface::cellml_api::URI> xlinkHref = dynamic_cast<iface::cellml_api::CellMLImport *>(pElement)->xlinkHref();

        setText(QString::fromStdWString(xlinkHref->asText()));

        break;
    }
    case UnitElement:
        setText(QString::fromStdWString(dynamic_cast<iface::cellml_api::Unit *>(pElement)->units()));

        break;
    case Group:
        setText(QObject::tr("Group #%1").arg(QString::number(pNumber)));

        break;
    case RelationshipReference:
        setText(QString::fromStdWString(dynamic_cast<iface::cellml_api::RelationshipRef *>(pElement)->relationship()));

        break;
    case ComponentReference:
        setText(QString::fromStdWString(dynamic_cast<iface::cellml_api::ComponentRef *>(pElement)->componentName()));

        break;
    case Connection:
        setText(QObject::tr("Connection #%1").arg(QString::number(pNumber)));

        break;
    case ComponentMapping: {
        ObjRef<iface::cellml_api::MapComponents> mapComponents = dynamic_cast<iface::cellml_api::MapComponents *>(pElement);

        setText(QString("%1 %2 %3").arg(QString::fromStdWString(mapComponents->firstComponentName()),
                                        QChar(RightArrow),
                                        QString::fromStdWString(mapComponents->secondComponentName())));

        break;
    }
    case VariableMapping: {
        ObjRef<iface::cellml_api::MapVariables> mapVariables = dynamic_cast<iface::cellml_api::MapVariables *>(pElement);

        setText(QString("%1 %2 %3").arg(QString::fromStdWString(mapVariables->firstVariableName()),
                                        QChar(RightArrow),
                                        QString::fromStdWString(mapVariables->secondVariableName())));

        break;
    }
    default:
        // Another type of element which has a name

        setText(QString::fromStdWString(dynamic_cast<iface::cellml_api::NamedCellMLElement *>(pElement)->name()));
    }

    // Use its text as a tooltip (in case it's too long and doesn't fit within
    // the allocated space we have)

    setToolTip(text());

    // Set the icon for the item

    setIcon(pType);
}

//==============================================================================

void CellmlAnnotationViewCellmlElementItem::setIcon(const Type &pType)
{
    // Determine the icon to be used for the item

    switch (pType) {
    case Error:
        QStandardItem::setIcon(QIcon(":CellMLSupport_errorNode"));

        break;
    case Warning:
        QStandardItem::setIcon(QIcon(":CellMLSupport_warningNode"));

        break;
    case Model:
        QStandardItem::setIcon(QIcon(":CellMLSupport_modelNode"));

        break;
    case Import:
        QStandardItem::setIcon(QIcon(":CellMLSupport_importNode"));

        break;
    case ImportUnit:
    case Unit:
        QStandardItem::setIcon(QIcon(":CellMLSupport_unitNode"));

        break;
    case UnitElement:
        QStandardItem::setIcon(QIcon(":CellMLSupport_unitElementNode"));

        break;
    case ImportComponent:
    case Component:
    case ComponentMapping:
        QStandardItem::setIcon(QIcon(":CellMLSupport_componentNode"));

        break;
    case Variable:
    case VariableMapping:
        QStandardItem::setIcon(QIcon(":CellMLSupport_variableNode"));

        break;
    case Group:
        QStandardItem::setIcon(QIcon(":CellMLSupport_groupNode"));

        break;
    case RelationshipReference:
        QStandardItem::setIcon(QIcon(":CellMLSupport_relationshipNode"));

        break;
    case ComponentReference:
        QStandardItem::setIcon(QIcon(":CellMLSupport_componentNode"));

        break;
    case Connection:
        QStandardItem::setIcon(QIcon(":CellMLSupport_connectionNode"));

        break;
    default:
        // Type which doesn't require an icon, so...

        ;
    }
}

//==============================================================================

bool CellmlAnnotationViewCellmlElementItem::isCategory() const
{
    // Return whether the CellML element item is a category

    return mCategory;
}

//==============================================================================

int CellmlAnnotationViewCellmlElementItem::type() const
{
    // Return the CellML element item's type

    return mType;
}

//==============================================================================

int CellmlAnnotationViewCellmlElementItem::number() const
{
    // Return the CellML element item's number

    return mNumber;
}

//==============================================================================

iface::cellml_api::CellMLElement * CellmlAnnotationViewCellmlElementItem::element() const
{
    // Return the CellML element item's element

    return mElement;
}

//==============================================================================

CellmlAnnotationViewCellmlListWidget::CellmlAnnotationViewCellmlListWidget(CellmlAnnotationViewWidget *pParent) :
    Widget(pParent),
    mCellmlFile(pParent->cellmlFile()),
    mGui(new Ui::CellmlAnnotationViewCellmlListWidget)
{
    // Set up the GUI

    mGui->setupUi(this);

    // Create and customise our tree view widget which will contain all of the
    // imports, units, components, groups and connections from a CellML file

    mTreeViewWidget = new Core::TreeViewWidget(pParent);
    mModel          = new QStandardItemModel(mTreeViewWidget);
    mItemDelegate   = new CellmlAnnotationViewCellmlElementItemDelegate();

    mTreeViewWidget->setModel(mModel);
    mTreeViewWidget->setItemDelegate(mItemDelegate);

    mTreeViewWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mTreeViewWidget->setHeaderHidden(true);
    mTreeViewWidget->setRootIsDecorated(false);
    mTreeViewWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    // Note: the selection mode we are opting for means that there is always
    //       going to be a CellML element which is selected, so it's something
    //       that we must keep in mind when showing the context menu...

    // Populate ourselves

    mGui->layout->addWidget(mTreeViewWidget);

    // We want a context menu for our tree view widget

    mTreeViewWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mTreeViewWidget, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showCustomContextMenu(const QPoint &)));

    // Some connections to handle the expansion/collapse of a CellML element

    connect(mTreeViewWidget, SIGNAL(expanded(const QModelIndex &)),
            this, SLOT(resizeTreeViewToContents()));
    connect(mTreeViewWidget, SIGNAL(collapsed(const QModelIndex &)),
            this, SLOT(resizeTreeViewToContents()));

    // Some connections to handle the change of CellML element

    connect(mTreeViewWidget->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(updateMetadataDetails(const QModelIndex &, const QModelIndex &)));

    // Initialise our tree view widget
    // Note: we don't want to select first item of our tree view widget just yet
    //       since we need a connection between ourselves and the metadata
    //       details widget to be set first (see
    //       CellmlAnnotationViewWidget::CellmlAnnotationViewWidget)...

    initializeTreeViewWidget(false);

    // Make our tree view widget our focus proxy

    setFocusProxy(mTreeViewWidget);
}

//==============================================================================

CellmlAnnotationViewCellmlListWidget::~CellmlAnnotationViewCellmlListWidget()
{
    // Delete the GUI

    delete mGui;
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::retranslateUi()
{
    // Retranslate our GUI

    mGui->retranslateUi(this);

    // Retranslate some of the CellML elements in our tree view widget

    retranslateDataItem(static_cast<CellmlAnnotationViewCellmlElementItem *>(mModel->invisibleRootItem()));
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::retranslateDataItem(CellmlAnnotationViewCellmlElementItem *pCellmlElementItem)
{
    // Retranslate some of the CellML element's children

    for (int i = 0, iMax = pCellmlElementItem->rowCount(); i < iMax; ++i)
        retranslateDataItem(static_cast<CellmlAnnotationViewCellmlElementItem *>(pCellmlElementItem->child(i)));

    // Check whether we are dealing with a category

    if (pCellmlElementItem->isCategory())
        // We are dealing with a category, so retranslate its type

        switch (pCellmlElementItem->type()) {
        case CellmlAnnotationViewCellmlElementItem::Import:
            pCellmlElementItem->setText(tr("Imports"));

            break;
        case CellmlAnnotationViewCellmlElementItem::Unit:
            pCellmlElementItem->setText(tr("Units"));

            break;
        case CellmlAnnotationViewCellmlElementItem::Component:
            pCellmlElementItem->setText(tr("Components"));

            break;
        case CellmlAnnotationViewCellmlElementItem::Variable:
            pCellmlElementItem->setText(tr("Variables"));

            break;
        case CellmlAnnotationViewCellmlElementItem::Group:
            pCellmlElementItem->setText(tr("Groups"));

            break;
        case CellmlAnnotationViewCellmlElementItem::RelationshipReference:
            pCellmlElementItem->setText(tr("Relationship References"));

            break;
        case CellmlAnnotationViewCellmlElementItem::ComponentReference:
            pCellmlElementItem->setText(tr("Component References"));

            break;
        case CellmlAnnotationViewCellmlElementItem::Connection:
            pCellmlElementItem->setText(tr("Connections"));

            break;
        default:
            // Not a type we can retranslate, so do nothing...

            ;
        }
    else
        // We are not dealing with a category, so check the type and see whether
        // a CellML element needs retranslating

        switch (pCellmlElementItem->type()) {
        case CellmlAnnotationViewCellmlElementItem::Group:
            pCellmlElementItem->setText(tr("Group #%1").arg(pCellmlElementItem->number()));

            break;
        case CellmlAnnotationViewCellmlElementItem::Connection:
            pCellmlElementItem->setText(tr("Connection #%1").arg(pCellmlElementItem->number()));

            break;
        default:
            // Not a sub-type we can retranslate, so do nothing...

            ;
        }
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::initializeTreeViewWidget(const bool &pSelectFirstItem)
{
    // Populate our tree view widget

    populateModel();

    // Expand our tree view widget enough so that we can see the meaningful
    // parts of the CellML file

    mTreeViewWidget->expandToDepth(1);

    // Resize our tree view widget, just to be on the safe side

    resizeTreeViewToContents();

    // Select the first item of our tree view widget

    if (pSelectFirstItem)
        mTreeViewWidget->selectFirstItem();
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::populateModel()
{
    // Make sure that the CellML file was properly loaded

    CellMLSupport::CellmlFileIssues issues = mCellmlFile->issues();
    int issuesCount = issues.count();

    if (issuesCount) {
        // Something went wrong while trying to load the CellML file, so report
        // the issue(s) and leave

        for (int i = 0; i < issuesCount; ++i)
            mModel->invisibleRootItem()->appendRow(new CellmlAnnotationViewCellmlElementItem(issues[i].type() == CellMLSupport::CellmlFileIssue::Error,
                                                                                             QString("[%1:%2] %3").arg(QString::number(issues[i].line()),
                                                                                                                       QString::number(issues[i].column()),
                                                                                                                       issues[i].formattedMessage())));

        return;
    }

    // Retrieve the model's root

    CellmlAnnotationViewCellmlElementItem *modelItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Model,
                                                                                                 mCellmlFile->model());

    mModel->invisibleRootItem()->appendRow(modelItem);

    // Retrieve the model's imports

    ObjRef<iface::cellml_api::CellMLImportSet> imports = mCellmlFile->model()->imports();

    if (imports->length()) {
        // Imports category

        CellmlAnnotationViewCellmlElementItem *importsItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Import,
                                                                                                       tr("Imports"));

        modelItem->appendRow(importsItem);

        // Retrieve the model's imports themselves

        ObjRef<iface::cellml_api::CellMLImportIterator> importsIterator = imports->iterateImports();

        for (ObjRef<iface::cellml_api::CellMLImport> import = importsIterator->nextImport();
             import; import = importsIterator->nextImport()) {
            CellmlAnnotationViewCellmlElementItem *importItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Import,
                                                                                                          import);

            importsItem->appendRow(importItem);

            // Retrieve the model's import's units

            ObjRef<iface::cellml_api::ImportUnitsSet> importUnitsSet = import->units();

            if (importUnitsSet->length()) {
                // Units category

                CellmlAnnotationViewCellmlElementItem *unitsItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::ImportUnit,
                                                                                                             tr("Units"));

                importItem->appendRow(unitsItem);

                // Retrieve the model's import's units themselves

                ObjRef<iface::cellml_api::ImportUnitsIterator> importUnitsIterator = importUnitsSet->iterateImportUnits();

                for (ObjRef<iface::cellml_api::ImportUnits> importUnit = importUnitsIterator->nextImportUnits();
                     importUnit; importUnit = importUnitsIterator->nextImportUnits())
                    unitsItem->appendRow(new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::ImportUnit,
                                                                                   importUnit));
            }

            // Retrieve the model's import's components

            ObjRef<iface::cellml_api::ImportComponentSet> importComponents = import->components();

            if (importComponents->length()) {
                // Components category

                CellmlAnnotationViewCellmlElementItem *componentsItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::ImportComponent,
                                                                                                                  tr("Components"));

                importItem->appendRow(componentsItem);

                // Retrieve the model's import's components themselves

                ObjRef<iface::cellml_api::ImportComponentIterator> importComponentsIterator = importComponents->iterateImportComponents();

                for (ObjRef<iface::cellml_api::ImportComponent> importComponent = importComponentsIterator->nextImportComponent();
                     importComponent; importComponent = importComponentsIterator->nextImportComponent())
                    componentsItem->appendRow(new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::ImportComponent,
                                                                                        importComponent));
            }
        }
    }

    // Retrieve the model's units

    ObjRef<iface::cellml_api::UnitsSet> unitsSet = mCellmlFile->model()->localUnits();

    populateUnitsModel(modelItem, unitsSet);

    // Retrieve the model's components

    ObjRef<iface::cellml_api::CellMLComponentSet> components = mCellmlFile->model()->localComponents();

    if (components->length()) {
        // Components category

        CellmlAnnotationViewCellmlElementItem *componentsItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Component,
                                                                                                          tr("Components"));

        modelItem->appendRow(componentsItem);

        // Retrieve the model's components themselves

        ObjRef<iface::cellml_api::CellMLComponentIterator> componentsIterator = components->iterateComponents();

        for (ObjRef<iface::cellml_api::CellMLComponent> component = componentsIterator->nextComponent();
             component; component = componentsIterator->nextComponent()) {
            CellmlAnnotationViewCellmlElementItem *componentItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Component,
                                                                                                             component);

            componentsItem->appendRow(componentItem);

            // Retrieve the model's component's units

            ObjRef<iface::cellml_api::UnitsSet> componentUnitsSet = component->units();

            populateUnitsModel(componentItem, componentUnitsSet);

            // Retrieve the model's component's variables

            ObjRef<iface::cellml_api::CellMLVariableSet> componentVariables = component->variables();

            if (componentVariables->length()) {
                // Variables category

                CellmlAnnotationViewCellmlElementItem *variablesItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Variable,
                                                                                                                 tr("Variables"));

                // Retrieve the model's component's variables themselves, but
                // only add a variable if neither its public nor its private
                // interface is equal to "in"

                bool atLeastOneVariable = false;
                ObjRef<iface::cellml_api::CellMLVariableIterator> componentVariablesIterator = componentVariables->iterateVariables();

                for (ObjRef<iface::cellml_api::CellMLVariable> componentVariable = componentVariablesIterator->nextVariable();
                     componentVariable; componentVariable = componentVariablesIterator->nextVariable())
                    if (   (componentVariable->publicInterface()  != iface::cellml_api::INTERFACE_IN)
                        && (componentVariable->privateInterface() != iface::cellml_api::INTERFACE_IN)) {
                        variablesItem->appendRow(new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Variable,
                                                                                           componentVariable));

                        atLeastOneVariable = true;
                    }

                // Check whether some variables were actually added

                if (atLeastOneVariable)
                    componentItem->appendRow(variablesItem);
                else
                    delete variablesItem;
            }
        }
    }

    // Retrieve the model's groups

    ObjRef<iface::cellml_api::GroupSet> groups = mCellmlFile->model()->groups();

    if (groups->length()) {
        // Groups category

        CellmlAnnotationViewCellmlElementItem *groupsItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Group,
                                                                                                      tr("Groups"));

        modelItem->appendRow(groupsItem);

        // Retrieve the model's groups themselves

        int counter = 0;
        ObjRef<iface::cellml_api::GroupIterator> groupsIterator = groups->iterateGroups();

        for (ObjRef<iface::cellml_api::Group> group = groupsIterator->nextGroup();
             group; group = groupsIterator->nextGroup()) {
            CellmlAnnotationViewCellmlElementItem *groupItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Group,
                                                                                                         group, ++counter);

            groupsItem->appendRow(groupItem);

            // Retrieve the model's group's relationship references

            ObjRef<iface::cellml_api::RelationshipRefSet> groupRelationshipReferences = group->relationshipRefs();

            if (groupRelationshipReferences->length()) {
                // Relationship references category

                CellmlAnnotationViewCellmlElementItem *groupRelationshipReferencesItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::RelationshipReference,
                                                                                                                                   tr("Relationship References"));

                groupItem->appendRow(groupRelationshipReferencesItem);

                // Retrieve the model's group's relationship references
                // themselves

                ObjRef<iface::cellml_api::RelationshipRefIterator> groupRelationshipReferencesIterator = groupRelationshipReferences->iterateRelationshipRefs();

                for (ObjRef<iface::cellml_api::RelationshipRef> groupRelationshipReference = groupRelationshipReferencesIterator->nextRelationshipRef();
                     groupRelationshipReference; groupRelationshipReference = groupRelationshipReferencesIterator->nextRelationshipRef())
                    groupRelationshipReferencesItem->appendRow(new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::RelationshipReference,
                                                                                                         groupRelationshipReference));
            }

            // Retrieve the model's group's component references

            ObjRef<iface::cellml_api::ComponentRefSet> groupComponentReferences = group->componentRefs();

            if (groupComponentReferences->length()) {
                // Component references category

                CellmlAnnotationViewCellmlElementItem *groupComponentReferencesItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::ComponentReference,
                                                                                                                                tr("Component References"));

                groupItem->appendRow(groupComponentReferencesItem);

                // Retrieve the model's group's relationship references
                // themselves

                ObjRef<iface::cellml_api::ComponentRefIterator> groupComponentReferencesIterator = groupComponentReferences->iterateComponentRefs();

                for (ObjRef<iface::cellml_api::ComponentRef> groupComponentReference = groupComponentReferencesIterator->nextComponentRef();
                     groupComponentReference; groupComponentReference = groupComponentReferencesIterator->nextComponentRef())
                    populateGroupComponentReferenceModel(groupComponentReferencesItem, groupComponentReference);
            }
        }
    }

    // Retrieve the model's connections

    ObjRef<iface::cellml_api::ConnectionSet> connections = mCellmlFile->model()->connections();

    if (connections->length()) {
        // Connections category

        CellmlAnnotationViewCellmlElementItem *connectionsItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Connection,
                                                                                                           tr("Connections"));

        modelItem->appendRow(connectionsItem);

        // Retrieve the model's connections themselves

        int counter = 0;
        ObjRef<iface::cellml_api::ConnectionIterator> connectionsIterator = connections->iterateConnections();

        for (ObjRef<iface::cellml_api::Connection> connection = connectionsIterator->nextConnection();
             connection; connection = connectionsIterator->nextConnection()) {
            CellmlAnnotationViewCellmlElementItem *connectionItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Connection,
                                                                                                              connection, ++counter);

            connectionsItem->appendRow(connectionItem);

            // Component mapping

            connectionItem->appendRow(new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::ComponentMapping,
                                                                                connection->componentMapping()));

            // Variable mappings

            ObjRef<iface::cellml_api::MapVariablesSet> connectionVariableMappings = connection->variableMappings();
            ObjRef<iface::cellml_api::MapVariablesIterator> connectionVariableMappingsIterator = connectionVariableMappings->iterateMapVariables();

            for (ObjRef<iface::cellml_api::MapVariables> connectionVariableMapping = connectionVariableMappingsIterator->nextMapVariables();
                 connectionVariableMapping; connectionVariableMapping = connectionVariableMappingsIterator->nextMapVariables())
                connectionItem->appendRow(new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::VariableMapping,
                                                                                    connectionVariableMapping));
        }
    }
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::populateUnitsModel(CellmlAnnotationViewCellmlElementItem *pCellmlElementItem,
                                                              iface::cellml_api::UnitsSet *pUnitsSet)
{
    // Retrieve the units

    if (pUnitsSet->length()) {
        // Units category

        CellmlAnnotationViewCellmlElementItem *unitsItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Unit,
                                                                                                     tr("Units"));

        pCellmlElementItem->appendRow(unitsItem);

        // Retrieve the units themselves

        ObjRef<iface::cellml_api::UnitsIterator> unitsIterator = pUnitsSet->iterateUnits();

        for (ObjRef<iface::cellml_api::Units> units = unitsIterator->nextUnits();
             units; units = unitsIterator->nextUnits()) {
            CellmlAnnotationViewCellmlElementItem *unitItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Unit,
                                                                                                        units);

            unitsItem->appendRow(unitItem);

            // Retrieve the units' unit references

            ObjRef<iface::cellml_api::UnitSet> unitSet = units->unitCollection();
            ObjRef<iface::cellml_api::UnitIterator> unitIterator = unitSet->iterateUnits();

            for (ObjRef<iface::cellml_api::Unit> unit = unitIterator->nextUnit();
                 unit; unit = unitIterator->nextUnit())
                unitItem->appendRow(new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::UnitElement,
                                                                              unit));
        }
    }
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::populateGroupComponentReferenceModel(CellmlAnnotationViewCellmlElementItem *pCellmlElementItem,
                                                                                iface::cellml_api::ComponentRef *pGroupComponentReference)
{
    CellmlAnnotationViewCellmlElementItem *groupComponentReferencesItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::ComponentReference,
                                                                                                                    pGroupComponentReference);

    pCellmlElementItem->appendRow(groupComponentReferencesItem);

    // Retrieve the component reference's children

    ObjRef<iface::cellml_api::ComponentRefSet> groupComponentReferences = pGroupComponentReference->componentRefs();
    ObjRef<iface::cellml_api::ComponentRefIterator> groupComponentReferencesIterator = groupComponentReferences->iterateComponentRefs();

    for (ObjRef<iface::cellml_api::ComponentRef> groupComponentReference = groupComponentReferencesIterator->nextComponentRef();
         groupComponentReference; groupComponentReference = groupComponentReferencesIterator->nextComponentRef())
        populateGroupComponentReferenceModel(groupComponentReferencesItem, groupComponentReference);
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::resizeTreeViewToContents()
{
    // Resize our tree view widget so that its contents is visible

    mTreeViewWidget->resizeColumnToContents(0);
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::updateMetadataDetails(const QModelIndex &pNewIndex,
                                                                 const QModelIndex &pOldIndex)
{
    Q_UNUSED(pOldIndex);

    // Make sure that we have a valid new index

    if (!pNewIndex.isValid())
        return;

    // Let people know that we want to see the metadata associated with the
    // CellML element

    emit metadataDetailsRequested(static_cast<CellmlAnnotationViewCellmlElementItem *>(mModel->itemFromIndex(pNewIndex))->element());
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::showCustomContextMenu(const QPoint &pPosition) const
{
    // Determine whether to show the context menu based on whether we are over
    // an item

    CellmlAnnotationViewCellmlElementItem *posItem = static_cast<CellmlAnnotationViewCellmlElementItem *>(mModel->itemFromIndex(mTreeViewWidget->indexAt(pPosition)));

    if (posItem) {
        // We are over an item, so create a custom context menu for our current
        // item

        // Update the enabled status of our actions

        bool fileReadableAndWritable = Core::FileManager::instance()->isReadableAndWritable(mCellmlFile->fileName());

        mGui->actionExpandAll->setEnabled(posItem->hasChildren() && !indexIsAllExpanded(mTreeViewWidget->currentIndex()));
        mGui->actionCollapseAll->setEnabled(posItem->hasChildren() && mTreeViewWidget->isExpanded(mTreeViewWidget->currentIndex()));

        mGui->actionRemoveCurrentMetadata->setEnabled(fileReadableAndWritable && !posItem->isCategory() && mCellmlFile->rdfTriples(posItem->element()).count());
        mGui->actionRemoveAllMetadata->setEnabled(fileReadableAndWritable && !posItem->isCategory() && mCellmlFile->rdfTriples().count());

        mGui->actionOpenImport->setEnabled(posItem->type() == CellmlAnnotationViewCellmlElementItem::Import);

        // Create and show the context menu, if it isn't empty

        QMenu menu;

        if (posItem->hasChildren()) {
            menu.addAction(mGui->actionExpandAll);
            menu.addAction(mGui->actionCollapseAll);
            menu.addSeparator();
        }

        if (!posItem->isCategory()) {
            menu.addAction(mGui->actionRemoveCurrentMetadata);
            menu.addAction(mGui->actionRemoveAllMetadata);
        }

        if (posItem->type() == CellmlAnnotationViewCellmlElementItem::Import) {
            menu.addSeparator();
            menu.addAction(mGui->actionOpenImport);
        }

        if (!menu.isEmpty())
            menu.exec(QCursor::pos());
    }
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::on_actionExpandAll_triggered()
{
    // Expand all the CellML elements below the current one
    // Note: we disable and then re-enable updates before expanding all the
    //       index since it may end up in quite a few updates...

    mTreeViewWidget->setUpdatesEnabled(false);
        qApp->setOverrideCursor(Qt::WaitCursor);

        indexExpandAll(mTreeViewWidget->currentIndex());

        qApp->restoreOverrideCursor();
    mTreeViewWidget->setUpdatesEnabled(true);
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::on_actionCollapseAll_triggered()
{
    // Collapse all the CellML elements below the current one
    // Note: see the note in on_actionExpandAll_triggered() above...

    mTreeViewWidget->setUpdatesEnabled(false);
        qApp->setOverrideCursor(Qt::WaitCursor);

        indexCollapseAll(mTreeViewWidget->currentIndex());

        qApp->restoreOverrideCursor();
    mTreeViewWidget->setUpdatesEnabled(true);
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::on_actionRemoveCurrentMetadata_triggered()
{
    // Remove all the metadata associated with the current node

    mCellmlFile->rdfTriples().remove(currentCellmlElementItem()->element());

    // Re-update the metadata details view now that the current node doesn't
    // have any metadata associated with it

    updateMetadataDetails(mTreeViewWidget->currentIndex(), QModelIndex());
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::on_actionRemoveAllMetadata_triggered()
{
    // Remove all the metadata associated with the CellML file

    mCellmlFile->rdfTriples().removeAll();

    // Re-update the metadata details view now that the CellML file doesn't have
    // any metadata associated with it

    updateMetadataDetails(mTreeViewWidget->currentIndex(), QModelIndex());
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::indexExpandAll(const QModelIndex &pIndex) const
{
    // Recursively expand all the CellML elements below the current one
    // Note: the test with against pIndex.child(0, 0) is to ensure that we are
    //       not trying to expand an index which item has no children. Indeed,
    //       a call to expand() is quite expensive, so the fewer of those we
    //       make the better...

    if (pIndex.child(0, 0).isValid()) {
        mTreeViewWidget->expand(pIndex);

        QStandardItem *item = mModel->itemFromIndex(pIndex);

        for (int i = 0, iMax = item->rowCount(); i < iMax; ++i)
            indexExpandAll(item->child(i)->index());
    }
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::indexCollapseAll(const QModelIndex &pIndex) const
{
    // Recursively collapse all the CellML elements below the current one
    // Note: see the note in indexExpandAll() above...

    if (pIndex.child(0, 0).isValid()) {
        QStandardItem *item = mModel->itemFromIndex(pIndex);

        for (int i = 0, iMax = item->rowCount(); i < iMax; ++i)
            indexCollapseAll(item->child(i)->index());

        mTreeViewWidget->collapse(pIndex);
    }
}

//==============================================================================

bool CellmlAnnotationViewCellmlListWidget::indexIsAllExpanded(const QModelIndex &pIndex) const
{
    // Recursively check that the current CellML element and all of its children
    // are expanded
    // Note: see the note in indexExpandAll() above...

    if (pIndex.child(0, 0).isValid()) {
        QStandardItem *item = mModel->itemFromIndex(pIndex);

        for (int i = 0, iMax = item->rowCount(); i < iMax; ++i)
            if (!indexIsAllExpanded(item->child(i)->index()))
                return false;

        return mTreeViewWidget->isExpanded(pIndex);
    } else {
        return true;
    }
}

//==============================================================================

Core::TreeViewWidget * CellmlAnnotationViewCellmlListWidget::treeViewWidget() const
{
    // Return our tree view widget

    return mTreeViewWidget;
}

//==============================================================================

CellmlAnnotationViewCellmlElementItem * CellmlAnnotationViewCellmlListWidget::currentCellmlElementItem() const
{
    // Return the current CellML element item

    return static_cast<CellmlAnnotationViewCellmlElementItem *>(mModel->itemFromIndex(mTreeViewWidget->currentIndex()));
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::on_actionOpenImport_triggered()
{
    // Ask OpenCOR to open the imported file

    static_cast<SharedTools::QtSingleApplication *>(qApp)->handleAction("gui://openFile/"+QFileInfo(QFileInfo(mCellmlFile->fileName()).canonicalPath()+QDir::separator()+currentCellmlElementItem()->text()).canonicalFilePath());
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::fileReloaded()
{
    // The file has been reloaded, so we need to clear our tree view widget
    // (i.e. the model associated with it) and (re)initialise it

    mModel->clear();

    initializeTreeViewWidget();
}

//==============================================================================

}   // namespace CellMLAnnotationView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
