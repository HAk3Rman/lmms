#include "ThemeManager.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>
#include <QStyle>
#include <QStyleFactory>

#include "ConfigManager.h"
#include "LmmsStyle.h"
#include "LmmsPalette.h"

namespace lmms::gui
{

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager* ThemeManager::instance()
{
    if (!s_instance)
    {
        s_instance = new ThemeManager();
    }
    return s_instance;
}

ThemeManager::ThemeManager()
    : QObject()
    , m_currentTheme("default")
    , m_previewTheme("")
    , m_isDarkMode(true)
    , m_isPreviewActive(false)
{
    initializeDefaultTheme();
    loadThemeCustomizations();
}

void ThemeManager::initializeDefaultTheme()
{
    // Load the default theme
    loadTheme("default");
}

bool ThemeManager::loadTheme(const QString& themeName)
{
    QString themePath = ConfigManager::inst()->dataDir() + "themes/" + themeName + "/";
    
    if (!QDir(themePath).exists())
    {
        qWarning() << "Theme directory not found:" << themePath;
        return false;
    }

    if (!loadThemeConfig(themePath) || !loadThemeStyleSheet(themePath))
    {
        qWarning() << "Failed to load theme:" << themeName;
        return false;
    }

    m_currentTheme = themeName;
    ConfigManager::inst()->setThemeDir(themePath);
    
    // Update artwork search paths
    QDir::addSearchPath("artwork", themePath + "artwork/");
    
    emit themeChanged(themeName);
    return true;
}

bool ThemeManager::loadThemeConfig(const QString& themePath)
{
    QFile configFile(themePath + "theme.json");
    if (!configFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Could not open theme config file:" << configFile.fileName();
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
    if (doc.isNull())
    {
        qWarning() << "Invalid theme config JSON:" << configFile.fileName();
        return false;
    }

    m_themeConfig = doc.object();
    m_isDarkMode = m_themeConfig["darkMode"].toBool(true);
    return true;
}

bool ThemeManager::loadThemeStyleSheet(const QString& themePath)
{
    QFile styleFile(themePath + "style.css");
    if (!styleFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Could not open theme stylesheet:" << styleFile.fileName();
        return false;
    }

    m_themeStyleSheet = QString::fromUtf8(styleFile.readAll());
    return true;
}

void ThemeManager::applyTheme()
{
    // Apply stylesheet
    qApp->setStyleSheet(m_themeStyleSheet);

    // Create and apply LMMS style
    auto lmmsStyle = new LmmsStyle();
    qApp->setStyle(lmmsStyle);

    // Create and apply palette
    auto lmmsPalette = new LmmsPalette(nullptr, lmmsStyle);
    auto palette = new QPalette(lmmsPalette->palette());
    qApp->setPalette(*palette);
    LmmsStyle::s_palette = palette;

    // Force update of all widgets
    for (QWidget* widget : qApp->allWidgets())
    {
        widget->update();
    }
}

void ThemeManager::previewTheme(const QString& themeName)
{
    if (themeName == m_currentTheme || themeName.isEmpty())
    {
        return;
    }

    m_previewTheme = themeName;
    m_isPreviewActive = true;

    QString themePath = ConfigManager::inst()->dataDir() + "themes/" + themeName + "/";
    if (loadThemeConfig(themePath) && loadThemeStyleSheet(themePath))
    {
        emit themePreviewStarted(themeName);
        applyTheme();
    }
}

void ThemeManager::cancelPreview()
{
    if (!m_isPreviewActive)
    {
        return;
    }

    m_isPreviewActive = false;
    m_previewTheme = "";

    // Restore original theme
    QString themePath = ConfigManager::inst()->dataDir() + "themes/" + m_currentTheme + "/";
    if (loadThemeConfig(themePath) && loadThemeStyleSheet(themePath))
    {
        emit themePreviewEnded();
        applyTheme();
    }
}

bool ThemeManager::isPreviewActive() const
{
    return m_isPreviewActive;
}

void ThemeManager::setThemeProperty(const QString& property, const QVariant& value)
{
    if (!m_customProperties.contains(property) || m_customProperties[property] != value)
    {
        m_customProperties[property] = value;
        emit themePropertyChanged(property, value);
        saveThemeCustomizations();
        applyTheme();
    }
}

QVariant ThemeManager::themeProperty(const QString& property) const
{
    return m_customProperties.value(property);
}

QStringList ThemeManager::customizableProperties() const
{
    return {
        "primaryColor",
        "secondaryColor",
        "backgroundColor",
        "textColor",
        "accentColor",
        "borderRadius",
        "fontSize",
        "spacing"
    };
}

QString ThemeManager::themeAuthor() const
{
    return m_themeConfig["author"].toString();
}

QString ThemeManager::themeVersion() const
{
    return m_themeConfig["version"].toString();
}

QString ThemeManager::themeDescription() const
{
    return m_themeConfig["description"].toString();
}

QStringList ThemeManager::themeCompatibility() const
{
    QStringList compat;
    for (const auto& item : m_themeConfig["compatibility"].toArray())
    {
        compat << item.toString();
    }
    return compat;
}

void ThemeManager::saveThemeCustomizations()
{
    QJsonObject customizations;
    for (auto it = m_customProperties.begin(); it != m_customProperties.end(); ++it)
    {
        customizations[it.key()] = QJsonValue::fromVariant(it.value());
    }

    QJsonDocument doc(customizations);
    QString themePath = ConfigManager::inst()->dataDir() + "themes/" + m_currentTheme + "/";
    QFile customFile(themePath + "custom.json");
    if (customFile.open(QIODevice::WriteOnly))
    {
        customFile.write(doc.toJson());
    }
}

void ThemeManager::loadThemeCustomizations()
{
    QString themePath = ConfigManager::inst()->dataDir() + "themes/" + m_currentTheme + "/";
    QFile customFile(themePath + "custom.json");
    if (customFile.open(QIODevice::ReadOnly))
    {
        QJsonDocument doc = QJsonDocument::fromJson(customFile.readAll());
        QJsonObject customizations = doc.object();
        
        m_customProperties.clear();
        for (auto it = customizations.begin(); it != customizations.end(); ++it)
        {
            m_customProperties[it.key()] = it.value().toVariant();
        }
    }
}

QString ThemeManager::currentTheme() const
{
    return m_currentTheme;
}

QStringList ThemeManager::availableThemes() const
{
    QDir themesDir(ConfigManager::inst()->dataDir() + "themes/");
    return themesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
}

QString ThemeManager::themeStyleSheet() const
{
    return m_themeStyleSheet;
}

QJsonObject ThemeManager::themeConfig() const
{
    return m_themeConfig;
}

bool ThemeManager::isDarkMode() const
{
    return m_isDarkMode;
}

QString ThemeManager::themeDir() const
{
    return ConfigManager::inst()->themeDir();
}

} // namespace lmms::gui
