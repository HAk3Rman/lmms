#ifndef LMMS_GUI_THEME_CUSTOMIZE_DIALOG_H
#define LMMS_GUI_THEME_CUSTOMIZE_DIALOG_H

#include <QDialog>
#include <QMap>
#include <QVariant>

class QColorDialog;
class QSpinBox;
class QComboBox;
class QLineEdit;

namespace lmms::gui
{

class LMMS_EXPORT ThemeCustomizeDialog : public QDialog
{
    Q_OBJECT

public:
    ThemeCustomizeDialog(QWidget* parent = nullptr);
    ~ThemeCustomizeDialog() override = default;

private slots:
    void propertyChanged();
    void resetToDefaults();
    void applyCustomizations();
    void showColorDialog(const QString& property);

private:
    void setupUi();
    void loadCurrentCustomizations();
    void updatePreview();

    QMap<QString, QVariant> m_customizations;
    QMap<QString, QWidget*> m_propertyWidgets;
    QColorDialog* m_colorDialog;
};

} // namespace lmms::gui

#endif // LMMS_GUI_THEME_CUSTOMIZE_DIALOG_H
