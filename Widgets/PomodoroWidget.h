#ifndef POMODOROWIDGET_H
#define POMODOROWIDGET_H

#include "../Core/BaseComponent.h"
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>

class PomodoroWidget : public BaseComponent {
    Q_OBJECT

public:
    explicit PomodoroWidget(QWidget *parent = nullptr);
    ~PomodoroWidget();

    void initStyle() override;
    void updateData() override; // Used for timer updates
    void setCustomSetting(const QString &key, const QVariant &value) override;

    int getWorkDuration() const { return m_workDuration; }
    int getShortBreakDuration() const { return m_shortBreakDuration; }
    int getLongBreakDuration() const { return m_longBreakDuration; }

private slots:
    void toggleTimer();
    void resetTimer();
    void switchMode(); // Cycle through Work -> Short Break -> Long Break -> Work

private:
    enum TimerMode {
        Work,
        ShortBreak,
        LongBreak
    };

    TimerMode m_currentMode;
    int m_remainingTime; // in seconds
    bool m_isRunning;

    // Settings (in minutes)
    int m_workDuration = 25;
    int m_shortBreakDuration = 5;
    int m_longBreakDuration = 15;

    // UI Elements
    QLabel *m_titleLabel;
    QLabel *m_timeLabel;
    QLabel *m_statusLabel;
    QPushButton *m_toggleButton;
    QPushButton *m_resetButton;
    QPushButton *m_modeButton;

    QTimer *m_timer;

    void updateDisplay();
    void updateStatusText();
    int getCurrentDuration() const;
    QString formatTime(int seconds) const;
};

#endif // POMODOROWIDGET_H
