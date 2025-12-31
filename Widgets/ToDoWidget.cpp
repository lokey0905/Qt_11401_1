#include "ToDoWidget.h"
#include <QHBoxLayout>
#include <QCheckBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

ToDoWidget::ToDoWidget(QWidget *parent) : BaseComponent(parent) {
    // Setup Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(5);

    // Title
    m_titleLabel = new QLabel("TO DO LIST", this);
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setAlignment(Qt::AlignLeft);
    mainLayout->addWidget(m_titleLabel);

    // List
    m_listWidget = new QListWidget(this);
    m_listWidget->setFrameShape(QFrame::NoFrame);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // Enable drag and drop for reordering if needed, but keep simple for now
    mainLayout->addWidget(m_listWidget);

    // Input Area
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(5);

    m_inputEdit = new QLineEdit(this);
    m_inputEdit->setPlaceholderText("Add new task...");
    
    m_addButton = new QPushButton("+", this);
    m_addButton->setFixedSize(24, 24);
    m_addButton->setCursor(Qt::PointingHandCursor);

    inputLayout->addWidget(m_inputEdit);
    inputLayout->addWidget(m_addButton);
    mainLayout->addLayout(inputLayout);

    // Connections
    connect(m_addButton, &QPushButton::clicked, this, &ToDoWidget::onAddClicked);
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &ToDoWidget::onAddClicked);

    // Initial Load
    loadTodos();
    initStyle();
    
    // Resize to a reasonable default
    this->resize(250, 300);
}

ToDoWidget::~ToDoWidget() {
    saveTodos();
}

void ToDoWidget::initStyle() {
    BaseComponent::initStyle();

    this->setStyleSheet(this->styleSheet() +
        "ToDoWidget {"
        "  background-color: rgba(0, 0, 0, 150);"
        "  border: 1px solid rgba(255, 255, 255, 30);"
        "  border-radius: 8px;"
        "}"
        "QLabel { color: white; background: transparent; font-family: 'Segoe UI', 'Microsoft JhengHei'; }"
        "#titleLabel { font-size: 14px; font-weight: bold; color: rgba(255, 255, 255, 220); margin-bottom: 2px; }"
        "QListWidget { background: transparent; outline: none; border: none; }"
        "QListWidget::item { border-bottom: 1px solid rgba(255,255,255,10); padding: 4px; }"
        "QListWidget::item:selected { background: transparent; }"
        "QListWidget::item:hover { background: rgba(255,255,255,10); }"
        "QLineEdit { "
        "  background: rgba(0,0,0,50); "
        "  border: 1px solid rgba(255,255,255,30); border-radius: 4px; "
        "  color: white; padding: 4px; font-family: 'Segoe UI'; font-size: 12px;"
        "}"
        "QPushButton { "
        "  background: rgba(255, 255, 255, 20); "
        "  color: white; border: 1px solid rgba(255,255,255,30); border-radius: 4px; font-weight: bold;"
        "}"
        "QPushButton:hover { background: rgba(255, 255, 255, 40); }"
        "QCheckBox { color: rgba(255,255,255,220); spacing: 8px; font-family: 'Segoe UI'; font-size: 13px; background: transparent; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border: 1px solid rgba(255,255,255,100); border-radius: 3px; background: transparent; }"
        "QCheckBox::indicator:hover { border-color: white; }"
        "QCheckBox::indicator:checked { background: #4CAF50; border: 1px solid #4CAF50; }"
    );
}

void ToDoWidget::updateData() {
    // No periodic update needed for Todo list
}

void ToDoWidget::onAddClicked() {
    QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty()) return;

    addNewItem(text, false);
    m_inputEdit->clear();
    saveTodos();
}

void ToDoWidget::addNewItem(const QString &text, bool isDone) {
    QListWidgetItem *item = new QListWidgetItem(m_listWidget);
    
    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(5, 2, 5, 2);
    layout->setSpacing(10);

    QCheckBox *checkBox = new QCheckBox(text, widget);
    checkBox->setChecked(isDone);
    // Strikeout effect if done
    QFont font = checkBox->font();
    font.setStrikeOut(isDone);
    checkBox->setFont(font);

    QPushButton *delBtn = new QPushButton("×", widget);
    delBtn->setFixedSize(20, 20);
    delBtn->setStyleSheet("QPushButton { background: transparent; color: #ff5555; font-size: 16px; font-weight: bold; } QPushButton:hover { color: #ff0000; background: rgba(255,0,0,20); }");
    delBtn->setCursor(Qt::PointingHandCursor);

    layout->addWidget(checkBox, 1);
    layout->addWidget(delBtn);

    item->setSizeHint(widget->sizeHint());
    m_listWidget->setItemWidget(item, widget);

    // Logic Connections
    connect(checkBox, &QCheckBox::toggled, this, [this, checkBox, item](bool checked){
        QFont f = checkBox->font();
        f.setStrikeOut(checked);
        checkBox->setFont(f);
        saveTodos();
    });

    connect(delBtn, &QPushButton::clicked, this, [this, item](){
        // Remove item
        int row = m_listWidget->row(item);
        delete m_listWidget->takeItem(row); // This deletes the item and the widget
        saveTodos();
    });
}

void ToDoWidget::saveTodos() {
    QJsonArray arr;
    for(int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        QWidget *w = m_listWidget->itemWidget(item);
        if (w) {
            QCheckBox *chk = w->findChild<QCheckBox*>();
            if (chk) {
                QJsonObject obj;
                obj["text"] = chk->text();
                obj["done"] = chk->isChecked();
                arr.append(obj);
            }
        }
    }

    QDir dir(QCoreApplication::applicationDirPath());
    dir.mkpath("user_assets");
    QFile file(dir.filePath("user_assets/todos.json"));
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(arr);
        file.write(doc.toJson());
    }
}

void ToDoWidget::loadTodos() {
    QDir dir(QCoreApplication::applicationDirPath());
    QFile file(dir.filePath("user_assets/todos.json"));
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isArray()) {
            QJsonArray arr = doc.array();
            for (const QJsonValue &val : arr) {
                QJsonObject obj = val.toObject();
                addNewItem(obj["text"].toString(), obj["done"].toBool());
            }
        }
    }
}

void ToDoWidget::onItemChanged(QListWidgetItem *item) {
    Q_UNUSED(item);
    saveTodos();
}
