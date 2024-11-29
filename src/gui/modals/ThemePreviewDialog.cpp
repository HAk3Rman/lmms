#include "ThemePreviewDialog.h"

#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include "ThemeManager.h"
#include "ConfigManager.h"
#include "embed.h"

namespace lmms::gui
{

ThemePreviewDialog::ThemePreviewDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
    m_previewTimer.setSingleShot(true);
    m_previewTimer.setInterval(300);
    connect(&m_previewTimer, &QTimer::timeout, this, &ThemePreviewDialog::updatePreview);
}

void ThemePreviewDialog::setupUi()
{
    setWindowTitle(tr("Theme Preview"));
    setWindowIcon(embed::getIconPixmap("setup_theme"));
    setMinimumSize(800, 600);

    auto mainLayout = new QVBoxLayout(this);

    // Theme selection area
    auto selectionLayout = new QHBoxLayout;
    m_themeComboBox = new QComboBox(this);
    m_themeComboBox->addItems(ThemeManager::instance()->availableThemes());
    m_themeComboBox->setCurrentText(ThemeManager::instance()->currentTheme());
    connect(m_themeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(themeSelectionChanged(int)));

    selectionLayout->addWidget(new QLabel(tr("Theme:"), this));
    selectionLayout->addWidget(m_themeComboBox);
    selectionLayout->addStretch();

    // Theme metadata
    auto metadataGroup = new QGroupBox(tr("Theme Information"), this);
    auto metadataLayout = new QVBoxLayout(metadataGroup);

    m_authorLabel = new QLabel(this);
    m_versionLabel = new QLabel(this);
    m_descriptionLabel = new QLabel(this);
    m_compatibilityLabel = new QLabel(this);

    m_descriptionLabel->setWordWrap(true);

    metadataLayout->addWidget(m_authorLabel);
    metadataLayout->addWidget(m_versionLabel);
    metadataLayout->addWidget(m_descriptionLabel);
    metadataLayout->addWidget(m_compatibilityLabel);

    // Preview area
    m_previewArea = new QScrollArea(this);
    m_previewArea->setWidgetResizable(true);
    m_previewWidget = new QWidget(m_previewArea);
    m_previewLayout = new QVBoxLayout(m_previewWidget);
    m_previewArea->setWidget(m_previewWidget);

    createPreviewWidgets();

    // Buttons
    auto buttonLayout = new QHBoxLayout;
    m_applyButton = new QPushButton(tr("Apply"), this);
    m_cancelButton = new QPushButton(tr("Cancel"), this);

    connect(m_applyButton, &QPushButton::clicked, this, &ThemePreviewDialog::applyTheme);
    connect(m_cancelButton, &QPushButton::clicked, this, &ThemePreviewDialog::cancelPreview);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_applyButton);
    buttonLayout->addWidget(m_cancelButton);

    // Main layout
    mainLayout->addLayout(selectionLayout);
    mainLayout->addWidget(metadataGroup);
    mainLayout->addWidget(m_previewArea);
    mainLayout->addLayout(buttonLayout);

    updateMetadata();
}

void ThemePreviewDialog::createPreviewWidgets()
{
    // Create sample widgets to showcase theme
    auto buttonsGroup = new QGroupBox(tr("Buttons"), m_previewWidget);
    auto buttonsLayout = new QHBoxLayout(buttonsGroup);
    buttonsLayout->addWidget(new QPushButton(tr("Normal"), buttonsGroup));
    auto hoveredBtn = new QPushButton(tr("Hover"), buttonsGroup);
    hoveredBtn->setProperty("class", "hover");
    buttonsLayout->addWidget(hoveredBtn);
    auto pressedBtn = new QPushButton(tr("Pressed"), buttonsGroup);
    pressedBtn->setProperty("class", "pressed");
    buttonsLayout->addWidget(pressedBtn);
    m_previewLayout->addWidget(buttonsGroup);

    // Add more sample widgets for other UI elements
    auto inputsGroup = new QGroupBox(tr("Input Fields"), m_previewWidget);
    auto inputsLayout = new QVBoxLayout(inputsGroup);
    inputsLayout->addWidget(new QComboBox(inputsGroup));
    m_previewLayout->addWidget(inputsGroup);

    m_previewLayout->addStretch();
}

void ThemePreviewDialog::updateMetadata()
{
    auto* theme = ThemeManager::instance();
    m_authorLabel->setText(tr("Author: %1").arg(theme->themeAuthor()));
    m_versionLabel->setText(tr("Version: %1").arg(theme->themeVersion()));
    m_descriptionLabel->setText(theme->themeDescription());
    m_compatibilityLabel->setText(tr("Compatibility: %1").arg(theme->themeCompatibility().join(", ")));
}

void ThemePreviewDialog::themeSelectionChanged(int index)
{
    m_selectedTheme = m_themeComboBox->itemText(index);
    m_previewTimer.start();
}

void ThemePreviewDialog::updatePreview()
{
    if (!m_selectedTheme.isEmpty())
    {
        ThemeManager::instance()->previewTheme(m_selectedTheme);
        updateMetadata();
    }
}

void ThemePreviewDialog::applyTheme()
{
    if (!m_selectedTheme.isEmpty())
    {
        ThemeManager::instance()->loadTheme(m_selectedTheme);
        ThemeManager::instance()->applyTheme();
        ConfigManager::inst()->setValue("app", "theme", m_selectedTheme);
    }
    accept();
}

void ThemePreviewDialog::cancelPreview()
{
    ThemeManager::instance()->cancelPreview();
    reject();
}

} // namespace lmms::gui
