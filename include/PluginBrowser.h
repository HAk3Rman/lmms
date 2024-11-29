/*
 * PluginBrowser.h - include file for PluginBrowser
 *
 * Copyright (c) 2005-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LMMS - https://lmms.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#ifndef LMMS_GUI_PLUGIN_BROWSER_H
#define LMMS_GUI_PLUGIN_BROWSER_H

#include <QPixmap>
#include <QStyledItemDelegate>
#include "SideBarWidget.h"
#include "Plugin.h"
#include "FavoritesManager.h"

class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;

namespace lmms::gui
{

class PluginBrowser : public SideBarWidget
{
    Q_OBJECT
public:
    PluginBrowser(QWidget* parent);
    ~PluginBrowser() override = default;

private slots:
    void onFilterChanged(const QString& filter);
    void onFavoritesChanged(const QString& pluginId);
    void showFavoritesOnly(bool checked);
    void showPluginDetails(QTreeWidgetItem* item, int column);

private:
    void addPlugins();
    void updateRootVisibility(int index);
    void updateRootVisibilities();
    void createCategoryIcons();
    void setupUi();
    void createFavoritesSection();

    QWidget* m_view;
    QTreeWidget* m_descTree;
    QLineEdit* m_searchBar;
    QMap<QString, QIcon> m_categoryIcons;
    QTreeWidgetItem* m_favoritesRoot;
    bool m_showFavoritesOnly;
};

class PluginDescWidget : public QWidget
{
    Q_OBJECT
public:
    using PluginKey = Plugin::Descriptor::SubPluginFeatures::Key;
    PluginDescWidget(const PluginKey& pk, QWidget* parent);
    QString name() const;
    void openInNewInstrumentTrack(QString value);
    QString pluginId() const;
    bool isFavorite() const;

protected:
    void enterEvent(QEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void mousePressEvent(QMouseEvent* me) override;
    void paintEvent(QPaintEvent* pe) override;
    void contextMenuEvent(QContextMenuEvent* e) override;

private slots:
    void toggleFavorite();

private:
    void loadPluginIcon();
    void createThumbnail();

    PluginKey m_pluginKey;
    QPixmap m_logo;
    QPixmap m_thumbnail;
    bool m_mouseOver;
    QString m_pluginId;

    friend class PluginBrowser;
};

} // namespace lmms::gui

#endif // LMMS_GUI_PLUGIN_BROWSER_H
