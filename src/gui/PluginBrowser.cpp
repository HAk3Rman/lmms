/*
 * PluginBrowser.cpp - implementation of the plugin-browser
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

#include "PluginBrowser.h"

#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolButton>

#include "embed.h"
#include "Engine.h"
#include "InstrumentTrack.h"
#include "Song.h"
#include "StringPairDrag.h"
#include "TrackContainerView.h"
#include "PluginFactory.h"
#include "ThemeManager.h"

namespace lmms::gui
{

PluginBrowser::PluginBrowser(QWidget* parent)
    : SideBarWidget(tr("Instrument Plugins"),
        embed::getIconPixmap("plugins").transformed(QTransform().rotate(90)), parent)
    , m_showFavoritesOnly(false)
{
    setupUi();
    createCategoryIcons();
    addPlugins();
    createFavoritesSection();

    // Connect to favorites manager
    connect(FavoritesManager::instance(), &FavoritesManager::favoritesChanged,
        this, &PluginBrowser::onFavoritesChanged);
}

void PluginBrowser::setupUi()
{
    setWindowTitle(tr("Instrument Browser"));
    m_view = new QWidget(contentParent());
    addContentWidget(m_view);

    auto viewLayout = new QVBoxLayout(m_view);
    viewLayout->setContentsMargins(5, 5, 5, 5);
    viewLayout->setSpacing(5);

    // Search bar with icon
    auto searchLayout = new QHBoxLayout;
    m_searchBar = new QLineEdit(m_view);
    m_searchBar->setPlaceholderText(tr("Search plugins..."));
    m_searchBar->setClearButtonEnabled(true);
    m_searchBar->addAction(embed::getIconPixmap("zoom"), QLineEdit::LeadingPosition);
    connect(m_searchBar, &QLineEdit::textChanged, this, &PluginBrowser::onFilterChanged);
    searchLayout->addWidget(m_searchBar);

    // Favorites toggle button
    auto favButton = new QToolButton(m_view);
    favButton->setIcon(embed::getIconPixmap("favorite"));
    favButton->setCheckable(true);
    favButton->setToolTip(tr("Show Favorites Only"));
    connect(favButton, &QToolButton::toggled, this, &PluginBrowser::showFavoritesOnly);
    searchLayout->addWidget(favButton);

    viewLayout->addLayout(searchLayout);

    // Tree widget for plugins
    m_descTree = new QTreeWidget(m_view);
    m_descTree->setColumnCount(1);
    m_descTree->header()->setVisible(false);
    m_descTree->setIndentation(10);
    m_descTree->setItemDelegate(new PluginItemDelegate(m_descTree));
    m_descTree->setAnimated(true);
    m_descTree->setAlternatingRowColors(true);
    m_descTree->setSelectionMode(QAbstractItemView::NoSelection);
    m_descTree->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_descTree->setUniformRowHeights(true);

    connect(m_descTree, &QTreeWidget::itemClicked,
        this, &PluginBrowser::showPluginDetails);

    viewLayout->addWidget(m_descTree);

    // Apply theme
    QString styleSheet = ThemeManager::instance()->themeStyleSheet();
    setStyleSheet(styleSheet);
}

void PluginBrowser::createCategoryIcons()
{
    m_categoryIcons = {
        {"Synthesizer", QIcon(embed::getIconPixmap("synth"))},
        {"Instrument", QIcon(embed::getIconPixmap("instrument"))},
        {"Effect", QIcon(embed::getIconPixmap("effect"))},
        {"LADSPA", QIcon(embed::getIconPixmap("ladspa"))},
        {"VST", QIcon(embed::getIconPixmap("vst"))},
        {"Favorites", QIcon(embed::getIconPixmap("favorite"))}
    };
}

void PluginBrowser::createFavoritesSection()
{
    m_favoritesRoot = new QTreeWidgetItem(m_descTree);
    m_favoritesRoot->setText(0, tr("Favorites"));
    m_favoritesRoot->setIcon(0, m_categoryIcons["Favorites"]);
    m_favoritesRoot->setExpanded(true);

    // Add favorite plugins
    auto favorites = FavoritesManager::instance()->getFavorites();
    for (const auto& pluginId : favorites)
    {
        // Find and clone the plugin item
        QTreeWidgetItemIterator it(m_descTree);
        while (*it)
        {
            auto widget = dynamic_cast<PluginDescWidget*>(m_descTree->itemWidget(*it, 0));
            if (widget && widget->pluginId() == pluginId)
            {
                auto clone = (*it)->clone();
                m_favoritesRoot->addChild(clone);
                m_descTree->setItemWidget(clone, 0, 
                    new PluginDescWidget(widget->m_pluginKey, m_descTree));
                break;
            }
            ++it;
        }
    }
}

void PluginBrowser::onFavoritesChanged(const QString& pluginId)
{
    // Update favorites section
    bool isFavorite = FavoritesManager::instance()->isFavorite(pluginId);

    if (isFavorite)
    {
        // Add to favorites section
        QTreeWidgetItemIterator it(m_descTree);
        while (*it)
        {
            auto widget = dynamic_cast<PluginDescWidget*>(m_descTree->itemWidget(*it, 0));
            if (widget && widget->pluginId() == pluginId)
            {
                auto clone = (*it)->clone();
                m_favoritesRoot->addChild(clone);
                m_descTree->setItemWidget(clone, 0, 
                    new PluginDescWidget(widget->m_pluginKey, m_descTree));
                break;
            }
            ++it;
        }
    }
    else
    {
        // Remove from favorites section
        for (int i = 0; i < m_favoritesRoot->childCount(); ++i)
        {
            auto item = m_favoritesRoot->child(i);
            auto widget = dynamic_cast<PluginDescWidget*>(m_descTree->itemWidget(item, 0));
            if (widget && widget->pluginId() == pluginId)
            {
                delete m_favoritesRoot->takeChild(i);
                break;
            }
        }
    }

    // Update visibility if showing favorites only
    if (m_showFavoritesOnly)
    {
        updateRootVisibilities();
    }
}

void PluginBrowser::showFavoritesOnly(bool checked)
{
    m_showFavoritesOnly = checked;
    updateRootVisibilities();
}

void PluginBrowser::onFilterChanged(const QString& filter)
{
    QTreeWidgetItemIterator it(m_descTree);
    while (*it)
    {
        auto widget = dynamic_cast<PluginDescWidget*>(m_descTree->itemWidget(*it, 0));
        if (widget)
        {
            bool matchesFilter = widget->name().contains(filter, Qt::CaseInsensitive);
            bool matchesFavorites = !m_showFavoritesOnly || widget->isFavorite();
            (*it)->setHidden(!matchesFilter || !matchesFavorites);
        }
        ++it;
    }
    updateRootVisibilities();
}

void PluginBrowser::updateRootVisibilities()
{
    for (int i = 0; i < m_descTree->topLevelItemCount(); ++i)
    {
        auto root = m_descTree->topLevelItem(i);
        bool hasVisibleChildren = false;
        for (int j = 0; j < root->childCount(); ++j)
        {
            if (!root->child(j)->isHidden())
            {
                hasVisibleChildren = true;
                break;
            }
        }
        root->setHidden(!hasVisibleChildren);
    }
}

void PluginBrowser::showPluginDetails(QTreeWidgetItem* item, int column)
{
    auto widget = dynamic_cast<PluginDescWidget*>(m_descTree->itemWidget(item, column));
    if (widget)
    {
        // TODO: Show detailed plugin information in a side panel or dialog
    }
}

// PluginDescWidget implementation
PluginDescWidget::PluginDescWidget(const PluginKey& pk, QWidget* parent)
    : QWidget(parent)
    , m_pluginKey(pk)
    , m_mouseOver(false)
{
    loadPluginIcon();
    createThumbnail();
    m_pluginId = QString("%1:%2").arg(pk.desc->name, pk.desc->version);
    
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    setToolTip(pk.desc->subPluginFeatures ? pk.description() : tr(pk.desc->description));
}

void PluginDescWidget::loadPluginIcon()
{
    // Load plugin icon or use default
    QString iconFile = QString(":/plugins/%1.png").arg(m_pluginKey.desc->name.toLower());
    if (QFile::exists(iconFile))
    {
        m_logo = QPixmap(iconFile);
    }
    else
    {
        m_logo = m_pluginKey.logo()->pixmap();
    }
}

void PluginDescWidget::createThumbnail()
{
    // Create a thumbnail preview of the plugin UI
    // This is a placeholder - actual implementation would need to render
    // the plugin's UI to a pixmap
    m_thumbnail = QPixmap(200, 150);
    m_thumbnail.fill(Qt::transparent);
    QPainter p(&m_thumbnail);
    p.drawPixmap(0, 0, m_logo.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QString PluginDescWidget::name() const
{
    return m_pluginKey.displayName();
}

QString PluginDescWidget::pluginId() const
{
    return m_pluginId;
}

bool PluginDescWidget::isFavorite() const
{
    return FavoritesManager::instance()->isFavorite(m_pluginId);
}

void PluginDescWidget::toggleFavorite()
{
    FavoritesManager::instance()->toggleFavorite(m_pluginId);
    update();
}

void PluginDescWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    // Paint background
    QStyleOption o;
    o.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);

    // Calculate sizes
    const int s = 32;
    const QSize logo_size(s, s);
    
    // Draw logo
    QPixmap logo = m_logo.scaled(logo_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p.drawPixmap(4, 4, logo);

    // Draw name
    QFont f = p.font();
    if (m_mouseOver)
    {
        f.setBold(true);
    }
    p.setFont(f);
    p.drawText(10 + logo_size.width(), 15, name());

    // Draw favorite indicator
    if (isFavorite())
    {
        QPixmap star = embed::getIconPixmap("favorite_on");
        p.drawPixmap(width() - 24, 4, 16, 16, star);
    }
}

void PluginDescWidget::enterEvent(QEvent* e)
{
    m_mouseOver = true;
    update();
    QWidget::enterEvent(e);
}

void PluginDescWidget::leaveEvent(QEvent* e)
{
    m_mouseOver = false;
    update();
    QWidget::leaveEvent(e);
}

void PluginDescWidget::mousePressEvent(QMouseEvent* me)
{
    if (me->button() == Qt::LeftButton)
    {
        // Check if click was on favorite icon
        if (me->pos().x() > width() - 24 && me->pos().y() < 20)
        {
            toggleFavorite();
            return;
        }

        // Start drag
        Engine::setDndPluginKey(&m_pluginKey);
        new StringPairDrag("instrument",
            QString::fromUtf8(m_pluginKey.desc->name), m_logo, this);
    }
}

void PluginDescWidget::contextMenuEvent(QContextMenuEvent* e)
{
    QMenu contextMenu(this);
    
    // Add favorite action
    QAction* favAction = contextMenu.addAction(
        isFavorite() ? tr("Remove from Favorites") : tr("Add to Favorites"));
    connect(favAction, &QAction::triggered, this, &PluginDescWidget::toggleFavorite);
    
    // Add track creation action
    contextMenu.addAction(tr("Send to new instrument track"),
        [this] { openInNewInstrumentTrack(m_pluginKey.desc->name); });
    
    contextMenu.exec(e->globalPos());
}

void PluginDescWidget::openInNewInstrumentTrack(QString value)
{
    TrackContainer* tc = Engine::getSong();
    auto it = dynamic_cast<InstrumentTrack*>(Track::create(Track::Type::Instrument, tc));
    auto ilt = new InstrumentLoaderThread(this, it, value);
    ilt->start();
}

} // namespace lmms::gui
