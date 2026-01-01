#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>

class ThemeManager {
public:
    ThemeManager() {
        // 設定主題儲存路徑
        themePath = QCoreApplication::applicationDirPath() + "../../Themes";
        QDir dir(themePath);
        if (!dir.exists()) dir.mkpath(".");
    }

    // 儲存 JSON
    void saveTheme(const QString &name, const QJsonObject &data) {
        QFile file(themePath + "/" + name + ".json");
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(data).toJson());
            file.close();
        }
    }

    // --- 新增遺失的 loadTheme 函式，解決 ControlPanel.cpp 的紅字 ---
    QJsonObject loadTheme(const QString &name) {
        QFile file(themePath + "/" + name + ".json");
        if (!file.open(QIODevice::ReadOnly)) {
            return QJsonObject();
        }

        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        return doc.object();
    }

    // 讀取清單
    QStringList getThemeList() {
        QDir dir(themePath);
        return dir.entryList(QStringList() << "*.json", QDir::Files).replaceInStrings(".json", "");
    }

    // 刪除檔案
    void deleteTheme(const QString &name) {
        QFile::remove(themePath + "/" + name + ".json");
    }

private:
    QString themePath;
};

#endif
