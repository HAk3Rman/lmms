#include "FavoritesManager.h"
#include "ConfigManager.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>

namespace lmms::gui
{

FavoritesManager* FavoritesManager::s_instance = nullptr;

FavoritesManager* FavoritesManager::instance()
{
    if (!s_instance)
    {
        s_instance = new FavoritesManager();
    }
    return s_instance;
}

FavoritesManager::FavoritesManager()
{
    loadFavorites();
}

void FavoritesManager::loadFavorites()
{
    QString favoritesFile = ConfigManager::inst()->userDataDir() + "favorites.json";
    QFile file(favoritesFile);
    
    if (file.open(QIODevice::ReadOnly))
    {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray array = doc.array();
        
        m_favorites.clear();
        for (const auto& value : array)
        {
            m_favorites.insert(value.toString());
        }
    }
}

void FavoritesManager::saveFavorites()
{
    QString favoritesFile = ConfigManager::inst()->userDataDir() + "favorites.json";
    QFile file(favoritesFile);
    
    if (file.open(QIODevice::WriteOnly))
    {
        QJsonArray array;
        for (const auto& favorite : m_favorites)
        {
            array.append(favorite);
        }
        
        QJsonDocument doc(array);
        file.write(doc.toJson());
    }
}

bool FavoritesManager::isFavorite(const QString& pluginId) const
{
    return m_favorites.contains(pluginId);
}

void FavoritesManager::addFavorite(const QString& pluginId)
{
    if (m_favorites.insert(pluginId))
    {
        saveFavorites();
        emit favoritesChanged(pluginId);
    }
}

void FavoritesManager::removeFavorite(const QString& pluginId)
{
    if (m_favorites.remove(pluginId))
    {
        saveFavorites();
        emit favoritesChanged(pluginId);
    }
}

void FavoritesManager::toggleFavorite(const QString& pluginId)
{
    if (isFavorite(pluginId))
    {
        removeFavorite(pluginId);
    }
    else
    {
        addFavorite(pluginId);
    }
}

QStringList FavoritesManager::getFavorites() const
{
    return QStringList(m_favorites.begin(), m_favorites.end());
}

} // namespace lmms::gui
