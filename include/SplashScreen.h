#ifndef LMMS_GUI_SPLASH_SCREEN_H
#define LMMS_GUI_SPLASH_SCREEN_H

#include <QSplashScreen>
#include <QProgressBar>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include "lmms_export.h"

namespace lmms::gui
{

class LMMS_EXPORT SplashScreen : public QSplashScreen
{
    Q_OBJECT
    Q_PROPERTY(qreal messageOpacity READ messageOpacity WRITE setMessageOpacity)
    Q_PROPERTY(qreal progressOpacity READ progressOpacity WRITE setProgressOpacity)

public:
    explicit SplashScreen(const QPixmap& pixmap);
    ~SplashScreen() override;

    void setProgress(int value);
    void showStatusMessage(const QString& message);
    void finish(QWidget* mainWindow);

    // Property accessors
    qreal messageOpacity() const { return m_messageOpacity; }
    void setMessageOpacity(qreal opacity);
    qreal progressOpacity() const { return m_progressOpacity; }
    void setProgressOpacity(qreal opacity);

protected:
    void drawContents(QPainter* painter) override;
    void showEvent(QShowEvent* event) override;

private:
    void setupUi();
    void createAnimations();
    void updateProgressBarStyle();
    void startAnimations();
    void cleanupAnimations();

    QProgressBar* m_progressBar;
    QString m_message;
    QTimer* m_animationTimer;
    qreal m_glowOpacity;
    bool m_glowIncreasing;

    // New animation properties
    qreal m_messageOpacity;
    qreal m_progressOpacity;
    QParallelAnimationGroup* m_showAnimations;
    QPropertyAnimation* m_messageAnimation;
    QPropertyAnimation* m_progressAnimation;
    QGraphicsOpacityEffect* m_progressEffect;

    // Theme colors
    QColor m_primaryColor;
    QColor m_accentColor;
    QColor m_textColor;
    QColor m_glowColor;
};

} // namespace lmms::gui

#endif // LMMS_GUI_SPLASH_SCREEN_H
