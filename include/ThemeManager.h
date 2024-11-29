#ifndef LMMS_GUI_THEME_MANAGER_H
#define LMMS_GUI_THEME_MANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QVariant>
#include <QMap>
#include "lmms_export.h"

namespace lmms::gui
{

class LMMS_EXPORT ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager* instance();
    ~ThemeManager() override = default;

    // Theme management
    bool loadTheme(const QString& themeName);
    void applyTheme();
    QString currentTheme() const;
    QStringList availableThemes() const;

    // Theme preview
    void previewTheme(const QString& themeName);
    void cancelPreview();
    bool isPreviewActive() const;

    // Theme customization
    void setThemeProperty(const QString& property, const QVariant& value);
    QVariant themeProperty(const QString& property) const;
    QStringList customizableProperties() const;

    // Theme properties
    QString themeStyleSheet() const;
    QJsonObject themeConfig() const;
    bool isDarkMode() const;
    QString themeDir() const;

    // Theme metadata
    QString themeAuthor() const;
    QString themeVersion() const;
    QString themeDescription() const;
    QStringList themeCompatibility() const;

signals:
    void themeChanged(const QString& themeName);
    void themePreviewStarted(const QString& themeName);
    void themePreviewEnded();
    void themePropertyChanged(const QString& property, const QVariant& value);

private:
    ThemeManager();
    static ThemeManager* s_instance;

    bool loadThemeConfig(const QString& themePath);
    bool loadThemeStyleSheet(const QString& themePath);
    void initializeDefaultTheme();
    void saveThemeCustomizations();
    void loadThemeCustomizations();

    QString m_currentTheme;
    QString m_previewTheme;
    QString m_themeStyleSheet;
    QJsonObject m_themeConfig;
    QMap<QString, QVariant> m_customProperties;
    bool m_isDarkMode;
    bool m_isPreviewActive;
};

} // namespace lmms::gui

#endif // LMMS_GUI_THEME_MANAGER_H
