#include "ThemeCustomizeDialog.h"

#include <QColorDialog>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "ThemeManager.h"
#include "ConfigManager.h"
#include "embed.h"

namespace lmms::gui
{

ThemeCustomizeDialog::ThemeCustomizeDialog(QWidget* parent)
    : QDialog(parent)
    , m_colorDialog(nullptr)
{
    setupUi();
    loadCurrentCustomizations();
}

void ThemeCustomizeDialog::setupUi()
{
    setWindowTitle(tr("Customize Theme"));
    setWindowIcon(embed::getIconPixmap("setup_theme"));
    setMinimumWidth(400);

    auto mainLayout = new QVBoxLayout(this);

    // Colors group
    auto colorsGroup = new QGroupBox(tr("Colors"), this);
    auto colorsLayout = new QFormLayout(colorsGroup);

    const QStringList colorProperties = {
        "primaryColor",
        "secondaryColor",
        "backgroundColor",
        "textColor",
        "accentColor"
    };

    for (const auto& prop : colorProperties)
    {
        auto colorButton = new QPushButton(this);
        colorButton->setFixedSize(30, 30);
        connect(colorButton, &QPushButton::clicked, this, [this, prop]() {
            showColorDialog(prop);
        });

        m_propertyWidgets[prop] = colorButton;
        colorsLayout->addRow(tr(prop.toUtf8().constData()), colorButton);
    }

    // Dimensions group
    auto dimensionsGroup = new QGroupBox(tr("Dimensions"), this);
    auto dimensionsLayout = new QFormLayout(dimensionsGroup);

    auto borderRadiusSpinner = new QSpinBox(this);
    borderRadiusSpinner->setRange(0, 20);
    connect(borderRadiusSpinner, SIGNAL(valueChanged(int)), this, SLOT(propertyChanged()));
    m_propertyWidgets["borderRadius"] = borderRadiusSpinner;
    dimensionsLayout->addRow(tr("Border Radius:"), borderRadiusSpinner);

    auto fontSizeSpinner = new QSpinBox(this);
    fontSizeSpinner->setRange(8, 24);
    connect(fontSizeSpinner, SIGNAL(valueChanged(int)), this, SLOT(propertyChanged()));
    m_propertyWidgets["fontSize"] = fontSizeSpinner;
    dimensionsLayout->addRow(tr("Font Size:"), fontSizeSpinner);

    auto spacingSpinner = new QSpinBox(this);
    spacingSpinner->setRange(0, 20);
    connect(spacingSpinner, SIGNAL(valueChanged(int)), this, SLOT(propertyChanged()));
    m_propertyWidgets["spacing"] = spacingSpinner;
    dimensionsLayout->addRow(tr("Spacing:"), spacingSpinner);

    // Buttons
    auto buttonLayout = new QHBoxLayout;
    auto resetButton = new QPushButton(tr("Reset to Defaults"), this);
    auto applyButton = new QPushButton(tr("Apply"), this);
    auto cancelButton = new QPushButton(tr("Cancel"), this);

    connect(resetButton, &QPushButton::clicked, this, &ThemeCustomizeDialog::resetToDefaults);
    connect(applyButton, &QPushButton::clicked, this, &ThemeCustomizeDialog::applyCustomizations);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(cancelButton);

    // Main layout
    mainLayout->addWidget(colorsGroup);
    mainLayout->addWidget(dimensionsGroup);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
}

void ThemeCustomizeDialog::loadCurrentCustomizations()
{
    auto* theme = ThemeManager::instance();
    for (const auto& prop : theme->customizableProperties())
    {
        QVariant value = theme->themeProperty(prop);
        m_customizations[prop] = value;

        auto* widget = m_propertyWidgets[prop];
        if (!widget) continue;

        if (auto* colorBtn = qobject_cast<QPushButton*>(widget))
        {
            QColor color(value.toString());
            QString style = QString("background-color: %1;").arg(color.name());
            colorBtn->setStyleSheet(style);
        }
        else if (auto* spinBox = qobject_cast<QSpinBox*>(widget))
        {
            spinBox->setValue(value.toInt());
        }
    }
}

void ThemeCustomizeDialog::propertyChanged()
{
    auto* sender = qobject_cast<QWidget*>(QObject::sender());
    if (!sender) return;

    QString property;
    for (auto it = m_propertyWidgets.begin(); it != m_propertyWidgets.end(); ++it)
    {
        if (it.value() == sender)
        {
            property = it.key();
            break;
        }
    }

    if (property.isEmpty()) return;

    if (auto* spinBox = qobject_cast<QSpinBox*>(sender))
    {
        m_customizations[property] = spinBox->value();
    }

    updatePreview();
}

void ThemeCustomizeDialog::showColorDialog(const QString& property)
{
    if (!m_colorDialog)
    {
        m_colorDialog = new QColorDialog(this);
        m_colorDialog->setOption(QColorDialog::ShowAlphaChannel);
    }

    QColor currentColor(m_customizations[property].toString());
    m_colorDialog->setCurrentColor(currentColor);

    if (m_colorDialog->exec() == QDialog::Accepted)
    {
        QColor newColor = m_colorDialog->currentColor();
        m_customizations[property] = newColor.name(QColor::HexArgb);

        auto* colorBtn = qobject_cast<QPushButton*>(m_propertyWidgets[property]);
        if (colorBtn)
        {
            QString style = QString("background-color: %1;").arg(newColor.name());
            colorBtn->setStyleSheet(style);
        }

        updatePreview();
    }
}

void ThemeCustomizeDialog::resetToDefaults()
{
    auto* theme = ThemeManager::instance();
    for (const auto& prop : theme->customizableProperties())
    {
        auto* widget = m_propertyWidgets[prop];
        if (!widget) continue;

        if (auto* colorBtn = qobject_cast<QPushButton*>(widget))
        {
            QColor defaultColor(theme->themeConfig()[prop].toString());
            QString style = QString("background-color: %1;").arg(defaultColor.name());
            colorBtn->setStyleSheet(style);
            m_customizations[prop] = defaultColor.name(QColor::HexArgb);
        }
        else if (auto* spinBox = qobject_cast<QSpinBox*>(widget))
        {
            int defaultValue = theme->themeConfig()[prop].toInt();
            spinBox->setValue(defaultValue);
            m_customizations[prop] = defaultValue;
        }
    }

    updatePreview();
}

void ThemeCustomizeDialog::updatePreview()
{
    auto* theme = ThemeManager::instance();
    for (auto it = m_customizations.begin(); it != m_customizations.end(); ++it)
    {
        theme->setThemeProperty(it.key(), it.value());
    }
}

void ThemeCustomizeDialog::applyCustomizations()
{
    auto* theme = ThemeManager::instance();
    for (auto it = m_customizations.begin(); it != m_customizations.end(); ++it)
    {
        theme->setThemeProperty(it.key(), it.value());
    }
    accept();
}

} // namespace lmms::gui
