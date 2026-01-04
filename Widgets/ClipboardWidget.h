#ifndef CLIPBOARDWIDGET_H
#define CLIPBOARDWIDGET_H

#include "../Core/BaseComponent.h"
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QDateTime>
#include <QPixmap>

struct ClipboardItem {
    enum Type { Text, Image };
    Type type;
    QString text;
    QPixmap image;
    QByteArray imageHash; // Store hash for comparison
    QDateTime timestamp;
    
    // Helper to check equality for deduplication
    bool operator==(const ClipboardItem& other) const {
        if (type != other.type) return false;
        if (type == Text) return text == other.text;
        if (type == Image) return imageHash == other.imageHash;
        return false; 
    }
};

class ClipboardWidget : public BaseComponent {
    Q_OBJECT

public:
    explicit ClipboardWidget(QWidget *parent = nullptr);
    ~ClipboardWidget();

    void initStyle() override;
    void updateData() override; 
    void setCustomSetting(const QString &key, const QVariant &value) override;
    
    int getHistoryLimit() const { return m_historyLimit; }

private slots:
    void onClipboardChanged();
    void onItemClicked(QListWidgetItem *item);
    void clearHistory();

private:
    QLabel *m_titleLabel;
    QListWidget *m_listWidget;
    QPushButton *m_clearButton;
    
    int m_historyLimit = 30;
    QList<ClipboardItem> m_history;
    bool m_isInternalUpdate = false; 

    void addItemToHistory(const ClipboardItem &item);
    void refreshList();
};

#endif // CLIPBOARDWIDGET_H
