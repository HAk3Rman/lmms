/*
 * GuiApplication.cpp
 *
 * Copyright (c) 2014 Lukas W <lukaswhl/at/gmail.com>
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

#include "GuiApplication.h"

#include "lmmsversion.h"

#include "LmmsStyle.h"
#include "LmmsPalette.h"
#include "ThemeManager.h"

#include "AutomationEditor.h"
#include "ConfigManager.h"
#include "ControllerRackView.h"
#include "MixerView.h"
#include "MainWindow.h"
#include "MicrotunerConfig.h"
#include "PatternEditor.h"
#include "PianoRoll.h"
#include "ProjectNotes.h"
#include "SongEditor.h"

#include "SplashScreen.h"

#include <QApplication>
#include <QDir>
#include <QtGlobal>
#include <QLabel>
#include <QMessageBox>
#include <QSplashScreen>

#ifdef LMMS_BUILD_WIN32
#include <windows.h>
#endif

namespace lmms
{


namespace gui
{

GuiApplication* getGUI()
{
	return GuiApplication::instance();
}


GuiApplication* GuiApplication::s_instance = nullptr;

GuiApplication* GuiApplication::instance()
{
	return s_instance;
}



GuiApplication::GuiApplication()
{
	// prompt the user to create the LMMS working directory (e.g. ~/Documents/lmms) if it doesn't exist
	if ( !ConfigManager::inst()->hasWorkingDir() &&
		QMessageBox::question( nullptr,
				tr( "Working directory" ),
				tr( "The LMMS working directory %1 does not "
				"exist. Create it now? You can change the directory "
				"later via Edit -> Settings." ).arg( ConfigManager::inst()->workingDir() ),
					QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes ) == QMessageBox::Yes)
	{
		ConfigManager::inst()->createWorkingDir();
	}
	// Init style and palette
	QDir::addSearchPath("artwork", ConfigManager::inst()->themeDir());
	QDir::addSearchPath("artwork", ConfigManager::inst()->defaultThemeDir());
	QDir::addSearchPath("artwork", ":/artwork");

	// Initialize and apply theme
	ThemeManager::instance()->loadTheme(ConfigManager::inst()->value("app", "theme", "default"));
	ThemeManager::instance()->applyTheme();

#ifdef LMMS_BUILD_APPLE
	QApplication::setAttribute(Qt::AA_DontShowIconsInMenus, true);
#endif

    initialize();
}

GuiApplication::~GuiApplication()
{
	s_instance = nullptr;
}

void GuiApplication::initialize()
{
    // Create and show splash screen
    QPixmap splashPixmap(":/themes/prism/artwork/splash.svg");
    SplashScreen* splash = new SplashScreen(splashPixmap);
    splash->show();

    // Process events to ensure splash is visible
    qApp->processEvents();

    splash->showStatusMessage(tr("Initializing theme system..."));
    splash->setProgress(10);
    
    // Initialize theme system
    ThemeManager::instance();
    qApp->processEvents();

    splash->showStatusMessage(tr("Loading configuration..."));
    splash->setProgress(20);
    
    // Load configuration
    ConfigManager::inst()->loadConfigFile();
    qApp->processEvents();

    splash->showStatusMessage(tr("Preparing audio system..."));
    splash->setProgress(40);
    
    // Initialize audio
    bool audioEngineInit = AudioEngine::init();
    qApp->processEvents();

    splash->showStatusMessage(tr("Creating main window..."));
    splash->setProgress(60);
    
    // Create main window
    m_mainWindow = new MainWindow;
    qApp->processEvents();

    splash->showStatusMessage(tr("Loading plugins..."));
    splash->setProgress(80);
    
    // Load plugins
    PluginFactory::instance();
    qApp->processEvents();

    splash->showStatusMessage(tr("Finalizing..."));
    splash->setProgress(100);
    qApp->processEvents();

    // Show main window and finish splash
    m_mainWindow->show();
    splash->finish(m_mainWindow);
    delete splash;

    // Check audio initialization
    if (!audioEngineInit)
    {
        QMessageBox::critical(m_mainWindow, tr("Audio Error"),
            tr("Audio interface could not be initialized.\n"
               "Please check your audio configuration."),
            QMessageBox::Ok);
    }
}

void GuiApplication::displayInitProgress(const QString &msg)
{
	Q_ASSERT(m_loadingProgressLabel != nullptr);
	
	m_loadingProgressLabel->setText(msg);
	// must force a UI update and process events, as there may be long gaps between processEvents() calls during init
	m_loadingProgressLabel->repaint();
	qApp->processEvents();
}

void GuiApplication::childDestroyed(QObject *obj)
{
	// when any object that can be reached via getGUI()->mainWindow(), getGUI()->mixerView(), etc
	//   is destroyed, ensure that their accessor functions will return null instead of a garbage pointer.
	if (obj == m_mainWindow)
	{
		m_mainWindow = nullptr;
	}
	else if (obj == m_mixerView)
	{
		m_mixerView = nullptr;
	}
	else if (obj == m_songEditor)
	{
		m_songEditor = nullptr;
	}
	else if (obj == m_automationEditor)
	{
		m_automationEditor = nullptr;
	}
	else if (obj == m_patternEditor)
	{
		m_patternEditor = nullptr;
	}
	else if (obj == m_pianoRoll)
	{
		m_pianoRoll = nullptr;
	}
	else if (obj == m_projectNotes)
	{
		m_projectNotes = nullptr;
	}
	else if (obj == m_microtunerConfig)
	{
		m_microtunerConfig = nullptr;
	}
	else if (obj == m_controllerRackView)
	{
		m_controllerRackView = nullptr;
	}
}

#ifdef LMMS_BUILD_WIN32
/*!
 * @brief Returns the Windows System font.
 */
QFont GuiApplication::getWin32SystemFont()
{
	NONCLIENTMETRICS metrics = { sizeof( NONCLIENTMETRICS ) };
	SystemParametersInfo( SPI_GETNONCLIENTMETRICS, sizeof( NONCLIENTMETRICS ), &metrics, 0 );
	int pointSize = metrics.lfMessageFont.lfHeight;
	if ( pointSize < 0 )
	{
		// height is in pixels, convert to points
		HDC hDC = GetDC( nullptr );
		pointSize = MulDiv( abs( pointSize ), 72, GetDeviceCaps( hDC, LOGPIXELSY ) );
		ReleaseDC( nullptr, hDC );
	}

	return QFont( QString::fromUtf8( metrics.lfMessageFont.lfFaceName ), pointSize );
}
#endif


} // namespace gui

} // namespace lmms
