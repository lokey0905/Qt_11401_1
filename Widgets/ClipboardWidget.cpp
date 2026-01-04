#include "ClipboardWidget.h"
#include <QVBoxLayout>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDebug>
#include <QPainter>
#include <QCryptographicHash>
#include <QBuffer>

ClipboardWidget::ClipboardWidget(QWidget *parent) : BaseComponent(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(5);

    // Header
    QHBoxLayout *headerLayout = new QHBoxLayout();
    m_titleLabel = new QLabel("CLIPBOARD", this);
    m_titleLabel->setObjectName("titleLabel");
    
    m_clearButton = new QPushButton("Clear", this);
    m_clearButton->setCursor(Qt::PointingHandCursor);
    m_clearButton->setFixedWidth(50);
    connect(m_clearButton, &QPushButton::clicked, this, &ClipboardWidget::clearHistory);

    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_clearButton);
    mainLayout->addLayout(headerLayout);

    // List
    m_listWidget = new QListWidget(this);
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    connect(m_listWidget, &QListWidget::itemClicked, this, &ClipboardWidget::onItemClicked);
    mainLayout->addWidget(m_listWidget);

    // Clipboard Connection
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &ClipboardWidget::onClipboardChanged);

    initStyle();
    
    // Initial check? Maybe not, let's wait for changes or user action.
    // But if we want to populate with current clipboard:
    // onClipboardChanged(); 
}

ClipboardWidget::~ClipboardWidget() {
}

void ClipboardWidget::initStyle() {
    BaseComponent::initStyle();
    this->setStyleSheet(this->styleSheet() +
        "ClipboardWidget {"
        "  background-color: rgba(0, 0, 0, 150);"
        "  border: 1px solid rgba(255, 255, 255, 30);"
        "  border-radius: 8px;"
        "}"
        "QLabel { color: white; background: transparent; font-family: 'Segoe UI', 'Microsoft JhengHei'; }"
        "#titleLabel { font-size: 14px; font-weight: bold; color: rgba(255, 255, 255, 220); }"
        "QPushButton { "
        "  background-color: rgba(255, 255, 255, 20);"
        "  border: 1px solid rgba(255, 255, 255, 50);"
        "  border-radius: 4px;"
        "  color: white;"
        "  font-size: 10px;"
        "}"
        "QPushButton:hover { background-color: rgba(255, 255, 255, 40); }"
        "QListWidget { background: transparent; border: none; outline: none; }"
        "QListWidget::item { "
        "  background-color: rgba(255, 255, 255, 10); "
        "  border-radius: 4px; "
        "  padding: 5px; "
        "  margin-bottom: 2px; "
        "  color: rgba(255, 255, 255, 200);"
        "}"
        "QListWidget::item:hover { background-color: rgba(255, 255, 255, 30); }"
        "QListWidget::item:selected { background-color: rgba(255, 255, 255, 50); color: white; }"
    );
}

void ClipboardWidget::updateData() {
    // Not used for polling, but could be used to clean up old items if we stored timestamps
}

void ClipboardWidget::onClipboardChanged() {
    if (m_isInternalUpdate) {
        m_isInternalUpdate = false;
        return;
    }

    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if (!mimeData) return;

    ClipboardItem newItem;
    newItem.timestamp = QDateTime::currentDateTime();

    if (mimeData->hasImage()) {
        newItem.type = ClipboardItem::Image;
        newItem.image = qvariant_cast<QPixmap>(mimeData->imageData());
        if (newItem.image.isNull()) return;
        
        // Calculate hash for deduplication
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        newItem.image.save(&buffer, "PNG"); // Save as PNG to get consistent bytes
        newItem.imageHash = QCryptographicHash::hash(bytes, QCryptographicHash::Md5);
        
    } else if (mimeData->hasText()) {
        newItem.type = ClipboardItem::Text;
        newItem.text = mimeData->text();
        if (newItem.text.isEmpty()) return;
    } else {
        return; // Unsupported type
    }

    addItemToHistory(newItem);
}

void ClipboardWidget::addItemToHistory(const ClipboardItem &item) {
    // Check for duplicate at the top
    if (!m_history.isEmpty()) {
        const ClipboardItem &top = m_history.first();
        if (item == top) return;
    }

    // Remove if exists elsewhere (move to top)
    for (int i = 0; i < m_history.size(); ++i) {
        if (m_history.at(i) == item) {
            m_history.removeAt(i);
            break;
        }
    }

    m_history.prepend(item);

    // Limit
    while (m_history.size() > m_historyLimit) {
        m_history.removeLast();
    }

    refreshList();
}

void ClipboardWidget::refreshList() {
    m_listWidget->clear();
    
    for (const ClipboardItem &item : m_history) {
        QString labelText;
        QIcon icon;

        if (item.type == ClipboardItem::Text) {
            labelText = item.text;
            // Truncate for display
            if (labelText.length() > 50) {
                labelText = labelText.left(50) + "...";
            }
            labelText.replace("\n", " "); // Single line
        } else {
            labelText = QString("Image [%1x%2]").arg(item.image.width()).arg(item.image.height());
            icon = QIcon(item.image.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }

        QListWidgetItem *listItem = new QListWidgetItem(icon, labelText);
        listItem->setToolTip(item.type == ClipboardItem::Text ? item.text : "Image");
        
        // Store index in data to retrieve from m_history
        // Actually, m_history matches list order (0 is top)
        m_listWidget->addItem(listItem);
    }
}

void ClipboardWidget::onItemClicked(QListWidgetItem *item) {
    int index = m_listWidget->row(item);
    if (index < 0 || index >= m_history.size()) return;

    const ClipboardItem &historyItem = m_history.at(index);
    QClipboard *clipboard = QApplication::clipboard();

    m_isInternalUpdate = true; // We are setting it, so we might want to ignore the echo...
    // BUT, we actually want it to move to top.
    // If we set m_isInternalUpdate = true, onClipboardChanged returns early.
    // Then we need to manually move it to top in m_history.
    // If we set m_isInternalUpdate = false, onClipboardChanged will be called, and it will handle moving to top.
    // Let's try letting onClipboardChanged handle it.
    m_isInternalUpdate = false; 

    if (historyItem.type == ClipboardItem::Text) {
        clipboard->setText(historyItem.text);
    } else {
        clipboard->setPixmap(historyItem.image);
    }
}

void ClipboardWidget::clearHistory() {
    m_history.clear();
    refreshList();
}

void ClipboardWidget::setCustomSetting(const QString &key, const QVariant &value) {
    if (key == "historyLimit") {
        m_historyLimit = value.toInt();
        if (m_historyLimit < 1) m_historyLimit = 1;
        
        // Prune
        while (m_history.size() > m_historyLimit) {
            m_history.removeLast();
        }
        refreshList();
    }
}
