#ifndef LMMS_GUI_THEME_PREVIEW_DIALOG_H
#define LMMS_GUI_THEME_PREVIEW_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QTimer>
#include "lmms_export.h"

class QComboBox;
class QPushButton;
class QScrollArea;
class QVBoxLayout;

namespace lmms::gui
{

class LMMS_EXPORT ThemePreviewDialog : public QDialog
{
    Q_OBJECT

public:
    ThemePreviewDialog(QWidget* parent = nullptr);
    ~ThemePreviewDialog() override = default;

private slots:
    void themeSelectionChanged(int index);
    void applyTheme();
    void cancelPreview();
    void updatePreview();

private:
    void setupUi();
    void createPreviewWidgets();
    void updateMetadata();

    QComboBox* m_themeComboBox;
    QPushButton* m_applyButton;
    QPushButton* m_cancelButton;
    QScrollArea* m_previewArea;
    QWidget* m_previewWidget;
    QVBoxLayout* m_previewLayout;

    QLabel* m_authorLabel;
    QLabel* m_versionLabel;
    QLabel* m_descriptionLabel;
    QLabel* m_compatibilityLabel;

    QTimer m_previewTimer;
    QString m_selectedTheme;
};

} // namespace lmms::gui

#endif // LMMS_GUI_THEME_PREVIEW_DIALOG_H
