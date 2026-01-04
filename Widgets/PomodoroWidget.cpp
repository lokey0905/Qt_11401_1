#include "PomodoroWidget.h"
#include <QDebug>

PomodoroWidget::PomodoroWidget(QWidget *parent) : BaseComponent(parent) {
    m_currentMode = Work;
    m_isRunning = false;
    m_remainingTime = m_workDuration * 60;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 10, 15, 10);
    mainLayout->setSpacing(5);

    // Title
    m_titleLabel = new QLabel("POMODORO", this);
    m_titleLabel->setObjectName("titleLabel");
    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignLeft);

    // Time Display
    m_timeLabel = new QLabel(formatTime(m_remainingTime), this);
    m_timeLabel->setObjectName("timeLabel");
    m_timeLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_timeLabel);

    // Status Display
    m_statusLabel = new QLabel("Work Time", this);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);

    // Controls
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    btnLayout->setContentsMargins(0, 5, 0, 0);

    m_toggleButton = new QPushButton("Start", this);
    m_toggleButton->setCursor(Qt::PointingHandCursor);
    connect(m_toggleButton, &QPushButton::clicked, this, &PomodoroWidget::toggleTimer);

    m_resetButton = new QPushButton("Reset", this);
    m_resetButton->setCursor(Qt::PointingHandCursor);
    connect(m_resetButton, &QPushButton::clicked, this, &PomodoroWidget::resetTimer);

    m_modeButton = new QPushButton("Mode", this);
    m_modeButton->setCursor(Qt::PointingHandCursor);
    connect(m_modeButton, &QPushButton::clicked, this, &PomodoroWidget::switchMode);

    btnLayout->addWidget(m_toggleButton);
    btnLayout->addWidget(m_resetButton);
    btnLayout->addWidget(m_modeButton);

    mainLayout->addLayout(btnLayout);

    // Timer
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &PomodoroWidget::updateData);

    initStyle();
}

PomodoroWidget::~PomodoroWidget() {
}

void PomodoroWidget::initStyle() {
    BaseComponent::initStyle();
    this->setStyleSheet(this->styleSheet() +
        "PomodoroWidget {"
        "  background-color: rgba(0, 0, 0, 150);"
        "  border: 1px solid rgba(255, 255, 255, 30);"
        "  border-radius: 8px;"
        "}"
        "QLabel { color: white; background: transparent; font-family: 'Segoe UI', 'Microsoft JhengHei'; }"
        "#titleLabel { font-size: 14px; font-weight: bold; color: rgba(255, 255, 255, 220); }"
        "#timeLabel { font-size: 32px; font-weight: bold; color: #FFFFFF; margin: 5px 0; }"
        "#statusLabel { font-size: 12px; color: rgba(255, 255, 255, 180); }"
        "QPushButton { "
        "  background-color: rgba(255, 255, 255, 20);"
        "  border: 1px solid rgba(255, 255, 255, 50);"
        "  border-radius: 4px;"
        "  color: white;"
        "  padding: 4px 8px;"
        "  font-size: 11px;"
        "}"
        "QPushButton:hover { background-color: rgba(255, 255, 255, 40); }"
        "QPushButton:pressed { background-color: rgba(255, 255, 255, 60); }"
    );
}

void PomodoroWidget::updateData() {
    if (m_isRunning && m_remainingTime > 0) {
        m_remainingTime--;
        updateDisplay();
        
        if (m_remainingTime == 0) {
            m_isRunning = false;
            m_timer->stop();
            m_toggleButton->setText("Start");
            // Could play a sound here or flash window
            m_statusLabel->setText("Time's Up!");
        }
    }
}

void PomodoroWidget::toggleTimer() {
    if (m_isRunning) {
        m_isRunning = false;
        m_timer->stop();
        m_toggleButton->setText("Resume");
    } else {
        if (m_remainingTime == 0) {
            resetTimer(); // Auto reset if starting from 0
        }
        m_isRunning = true;
        m_timer->start(1000);
        m_toggleButton->setText("Pause");
    }
}

void PomodoroWidget::resetTimer() {
    m_isRunning = false;
    m_timer->stop();
    m_remainingTime = getCurrentDuration() * 60;
    m_toggleButton->setText("Start");
    updateDisplay();
    updateStatusText();
}

void PomodoroWidget::switchMode() {
    if (m_currentMode == Work) {
        m_currentMode = ShortBreak;
    } else if (m_currentMode == ShortBreak) {
        m_currentMode = LongBreak;
    } else {
        m_currentMode = Work;
    }
    resetTimer();
}

void PomodoroWidget::updateDisplay() {
    m_timeLabel->setText(formatTime(m_remainingTime));
}

void PomodoroWidget::updateStatusText() {
    switch (m_currentMode) {
        case Work:
            m_statusLabel->setText("Work Time");
            m_timeLabel->setStyleSheet("#timeLabel { color: #FF5252; }"); // Red for work
            break;
        case ShortBreak:
            m_statusLabel->setText("Short Break");
            m_timeLabel->setStyleSheet("#timeLabel { color: #4CAF50; }"); // Green for break
            break;
        case LongBreak:
            m_statusLabel->setText("Long Break");
            m_timeLabel->setStyleSheet("#timeLabel { color: #2196F3; }"); // Blue for long break
            break;
    }
}

int PomodoroWidget::getCurrentDuration() const {
    switch (m_currentMode) {
        case Work: return m_workDuration;
        case ShortBreak: return m_shortBreakDuration;
        case LongBreak: return m_longBreakDuration;
        default: return 25;
    }
}

QString PomodoroWidget::formatTime(int seconds) const {
    int m = seconds / 60;
    int s = seconds % 60;
    return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}

void PomodoroWidget::setCustomSetting(const QString &key, const QVariant &value) {
    bool changed = false;
    if (key == "workDuration") {
        m_workDuration = value.toInt();
        changed = (m_currentMode == Work);
    } else if (key == "shortBreakDuration") {
        m_shortBreakDuration = value.toInt();
        changed = (m_currentMode == ShortBreak);
    } else if (key == "longBreakDuration") {
        m_longBreakDuration = value.toInt();
        changed = (m_currentMode == LongBreak);
    }

    if (changed && !m_isRunning) {
        resetTimer();
    }
}
