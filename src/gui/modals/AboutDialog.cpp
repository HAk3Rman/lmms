/*
 * AboutDialog.cpp - implementation of about-dialog
 *
 * Copyright (c) 2004-2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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


#include "AboutDialog.h"
#include "embed.h"
#include "versioninfo.h"
#include "ThemeManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QTextBrowser>
#include <QFontDatabase>
#include <QApplication>
#include <QScreen>

namespace lmms::gui
{

AboutDialog::AboutDialog(QWidget* parent) :
    QDialog(parent)
{
    setupModernUi();
}

void AboutDialog::setupModernUi()
{
    // Set window properties
    setWindowTitle(tr("About LMMS"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumSize(800, 600);

    // Load Inter font
    int fontId = QFontDatabase::addApplicationFont(":/fonts/Inter-Regular.ttf");
    QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont modernFont(fontFamily, 10);
    QApplication::setFont(modernFont);

    // Create main layout
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Header section with logo and version
    auto headerLayout = new QHBoxLayout;
    
    // Logo
    auto logoLabel = new QLabel(this);
    logoLabel->setPixmap(QPixmap(":/themes/prism/artwork/prism_logo.svg").scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    headerLayout->addWidget(logoLabel);

    // Version info
    auto versionLayout = new QVBoxLayout;
    
    auto titleLabel = new QLabel("LMMS - Prism Studio", this);
    titleLabel->setFont(QFont(fontFamily, 24, QFont::Bold));
    versionLayout->addWidget(titleLabel);

    QString versionText = tr("Version %1 (%2, %3-bit)")
        .arg(LMMS_VERSION)
        .arg(LMMS_BUILDCONF_PLATFORM)
        .arg(LMMS_BUILDCONF_MACHINE);
    auto versionLabel = new QLabel(versionText, this);
    versionLabel->setFont(QFont(fontFamily, 12));
    versionLayout->addWidget(versionLabel);

    QString buildText = tr("Built with Qt %1 (%2)")
        .arg(QT_VERSION_STR)
        .arg(LMMS_BUILDCONF_COMPILER_VERSION);
    auto buildLabel = new QLabel(buildText, this);
    buildLabel->setFont(QFont(fontFamily, 10));
    versionLayout->addWidget(buildLabel);

    headerLayout->addLayout(versionLayout);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // Tabs
    auto tabWidget = new QTabWidget(this);
    tabWidget->setFont(modernFont);

    // About tab
    auto aboutWidget = new QWidget(this);
    auto aboutLayout = new QVBoxLayout(aboutWidget);
    
    auto aboutText = new QTextBrowser(this);
    aboutText->setOpenExternalLinks(true);
    aboutText->setFont(modernFont);
    aboutText->setHtml(tr(
        "<p>LMMS Prism Studio is a modern digital audio workstation for creating music.</p>"
        "<p>Copyright %1</p>"
        "<p>This program is free software; you can redistribute it and/or modify it "
        "under the terms of the GNU General Public License as published by the Free "
        "Software Foundation.</p>"
        "<p>Visit <a href='https://lmms.io'>lmms.io</a> for more information.</p>"
    ).arg(LMMS_PROJECT_COPYRIGHT));
    aboutLayout->addWidget(aboutText);
    tabWidget->addTab(aboutWidget, tr("About"));

    // Authors tab
    auto authorsText = new QTextBrowser(this);
    authorsText->setPlainText(embed::getText("AUTHORS"));
    authorsText->setFont(modernFont);
    tabWidget->addTab(authorsText, tr("Authors"));

    // Contributors tab
    auto contributorsText = new QTextBrowser(this);
    contributorsText->setPlainText(embed::getText("CONTRIBUTORS"));
    contributorsText->setFont(modernFont);
    tabWidget->addTab(contributorsText, tr("Contributors"));

    // License tab
    auto licenseText = new QTextBrowser(this);
    licenseText->setPlainText(embed::getText("LICENSE.txt"));
    licenseText->setFont(modernFont);
    tabWidget->addTab(licenseText, tr("License"));

    mainLayout->addWidget(tabWidget);

    // Close button
    auto buttonLayout = new QHBoxLayout;
    auto closeButton = new QPushButton(tr("Close"), this);
    closeButton->setFont(modernFont);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    mainLayout->addLayout(buttonLayout);

    // Center on screen
    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    // Apply theme
    QString styleSheet = ThemeManager::instance()->themeStyleSheet();
    setStyleSheet(styleSheet);
}

} // namespace lmms::gui