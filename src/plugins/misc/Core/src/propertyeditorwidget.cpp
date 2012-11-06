//==============================================================================
// Property editor widget
//==============================================================================

#include "propertyeditorwidget.h"

//==============================================================================

#include <float.h>

//==============================================================================

#include <QKeyEvent>
#include <QStandardItem>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

DoubleEditWidget::DoubleEditWidget(const double &pValue, QWidget *pParent) :
    QLineEdit(QString::number(pValue), pParent)
{
#ifdef Q_WS_MAC
    setAttribute(Qt::WA_MacShowFocusRect, 0);
    // Note: the above removes the focus border since it messes up the look of
    //       our editor
#endif
}

//==============================================================================

void DoubleEditWidget::keyPressEvent(QKeyEvent *pEvent)
{
    // Let people know if the user wants to go to the previous/next property

    if (   !(pEvent->modifiers() & Qt::ShiftModifier)
        && !(pEvent->modifiers() & Qt::ControlModifier)
        && !(pEvent->modifiers() & Qt::AltModifier)
        && !(pEvent->modifiers() & Qt::MetaModifier)) {
        if (pEvent->key() == Qt::Key_Up) {
            // The user wants to go to the previous property, so...

            emit goToPreviousPropertyRequested();

            return;
        } else if (pEvent->key() == Qt::Key_Down) {
            // The user wants to go to the previous property, so...

            emit goToNextPropertyRequested();

            return;
        }
    }

    // Default handling of the event

    QLineEdit::keyPressEvent(pEvent);
}

//==============================================================================

PropertyItemDelegate::PropertyItemDelegate() :
    QStyledItemDelegate(),
    mModel(0)
{
}

//==============================================================================

QWidget * PropertyItemDelegate::createEditor(QWidget *pParent,
                                             const QStyleOptionViewItem &pOption,
                                             const QModelIndex &pIndex) const
{
    Q_UNUSED(pOption);

    // Create and return an editor for our double
    // Note: we don't allow the editing of a string, so...

    QStandardItem *item = mModel->itemFromIndex(pIndex);

    DoubleEditWidget *editor = new DoubleEditWidget(item->text().toDouble(),
                                                    pParent);

    // Keep track of when the editing finishes

    connect(editor, SIGNAL(editingFinished()),
            this, SLOT(commitAndCloseEditor()));

    // Propagate the fact that the user wants to go to the previous/next
    // property

    connect(editor, SIGNAL(goToPreviousPropertyRequested()),
            this, SIGNAL(goToPreviousPropertyRequested()));
    connect(editor, SIGNAL(goToNextPropertyRequested()),
            this, SIGNAL(goToNextPropertyRequested()));

    // Let people know that there is a new editor

    emit openEditor(editor);

    // Return the editor

    return editor;
}

//==============================================================================

void PropertyItemDelegate::commitAndCloseEditor()
{
    // Commit the new value and close the editor

    DoubleEditWidget *editor = qobject_cast<DoubleEditWidget *>(sender());

    emit commitData(editor);
    emit closeEditor(editor);
}

//==============================================================================

void PropertyItemDelegate::setModel(QStandardItemModel *pModel)
{
    // Set the model to be used by us

    if (pModel != mModel)
        mModel = pModel;
}

//==============================================================================

PropertyItem::PropertyItem(const Type &pType, const QString &pValue,
                           const bool &pEditable) :
    QStandardItem(pValue),
    mType(pType)
{
    // Check whether the item should be editable

    if (!pEditable)
        setFlags(flags() & ~Qt::ItemIsEditable);
}

//==============================================================================

int PropertyItem::type() const
{
    // Return the property item's type

    return mType;
}

//==============================================================================

PropertyEditorWidget::PropertyEditorWidget(QWidget *pParent) :
    TreeViewWidget(pParent)
{
    // Create our item delegate and set it, after making sure that we handle a
    // few of its signals

    mPropertyItemDelegate = new PropertyItemDelegate();

    connect(mPropertyItemDelegate, SIGNAL(openEditor(QWidget *)),
            this, SLOT(editorOpened(QWidget *)));
    connect(mPropertyItemDelegate, SIGNAL(closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint)),
            this, SLOT(editorClosed()));

    connect(mPropertyItemDelegate, SIGNAL(goToPreviousPropertyRequested()),
            this, SLOT(goToPreviousProperty()));
    connect(mPropertyItemDelegate, SIGNAL(goToNextPropertyRequested()),
            this, SLOT(goToNextProperty()));

    setItemDelegate(mPropertyItemDelegate);

    // Further customise ourself

    setSelectionMode(QAbstractItemView::SingleSelection);
}

//==============================================================================

void PropertyEditorWidget::initialize(QStandardItemModel *pModel)
{
    // Stop tracking the change of property, if needed

    if (selectionModel())
        disconnect(selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
                   this, SLOT(editProperty(const QModelIndex &, const QModelIndex &)));

    // Update our model and, as a result, our item delegate

    setModel(pModel);

    mPropertyItemDelegate->setModel(pModel);

    // Keep track of the change of property
    // Note: the idea is to automatically start the editing of a property...

    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(editProperty(const QModelIndex &, const QModelIndex &)));
}

//==============================================================================

PropertyItem * PropertyEditorWidget::newNonEditableString(const QString &pValue)
{
    // Create and return a non-editable item

    return new PropertyItem(PropertyItem::String, pValue, false);
}

//==============================================================================

PropertyItem * PropertyEditorWidget::newEditableDouble(const double &pValue)
{
    // Create and return an editable item

    return new PropertyItem(PropertyItem::Double, QString::number(pValue));
}

//==============================================================================

void PropertyEditorWidget::keyPressEvent(QKeyEvent *pEvent)
{
    // Check some key combinations

    if (   (pEvent->modifiers() & Qt::ControlModifier)
        && (pEvent->key() == Qt::Key_A))
        // The user wants to select everything which we don't want to allow,
        // so...

        pEvent->ignore();
    else
        // Not a key combination we handle, so...

        TreeViewWidget::keyPressEvent(pEvent);
}

//==============================================================================

void PropertyEditorWidget::editorOpened(QWidget *pEditor)
{
    // We are starting the editing of a property, so use its editor as our focus
    // proxy and make sure that it immediately gets the focus
    // Note: if we were not to immediately give the editor the focus, then the
    //       central widget would give the focus to the previously focused
    //       widget (see CentralWidget::updateGui()), so...

    setFocusProxy(pEditor);

    pEditor->setFocus();
}

//==============================================================================

void PropertyEditorWidget::editorClosed()
{
    // We have stopped editing a property, so reset our focus proxy and make
    // sure that we get the focus (see editorOpened() above for the reason)

    setFocusProxy(0);

    setFocus();
}

//==============================================================================

void PropertyEditorWidget::editProperty(const QModelIndex &pNewItem,
                                        const QModelIndex &pOldItem)
{
    Q_UNUSED(pOldItem);

    // Edit the current property (which value is always in column 1)

    edit(model()->index(pNewItem.row(), 1));
}

//==============================================================================

void PropertyEditorWidget::goToPreviousProperty()
{
    // Go to the previous property

    int newRow = currentIndex().row()-1;

    if (newRow >= 0)
        selectItem(newRow);
}

//==============================================================================

void PropertyEditorWidget::goToNextProperty()
{
    // Go to the next property

    int newRow = currentIndex().row()+1;

    if (newRow < model()->rowCount())
        selectItem(newRow);
}

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
