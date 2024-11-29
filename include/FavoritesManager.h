#ifndef LMMS_GUI_FAVORITES_MANAGER_H
#define LMMS_GUI_FAVORITES_MANAGER_H

#include <QObject>
#include <QSet>
#include <QString>
#include "Plugin.h"
#include "lmms_export.h"

namespace lmms::gui
{

class LMMS_EXPORT FavoritesManager : public QObject
{
    Q_OBJECT

public:
    static FavoritesManager* instance();
    ~FavoritesManager() override = default;

    bool isFavorite(const QString& pluginId) const;
    void addFavorite(const QString& pluginId);
    void removeFavorite(const QString& pluginId);
    void toggleFavorite(const QString& pluginId);
    QStringList getFavorites() const;

signals:
    void favoritesChanged(const QString& pluginId);

private:
    FavoritesManager();
    void loadFavorites();
    void saveFavorites();

    static FavoritesManager* s_instance;
    QSet<QString> m_favorites;
};

} // namespace lmms::gui

#endif // LMMS_GUI_FAVORITES_MANAGER_H
