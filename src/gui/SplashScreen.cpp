#include "SplashScreen.h"

#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include "ThemeManager.h"

namespace lmms::gui
{

SplashScreen::SplashScreen(const QPixmap& pixmap)
    : QSplashScreen(pixmap)
    , m_progressBar(nullptr)
    , m_glowOpacity(0.0)
    , m_glowIncreasing(true)
    , m_messageOpacity(0.0)
    , m_progressOpacity(0.0)
    , m_showAnimations(nullptr)
    , m_messageAnimation(nullptr)
    , m_progressAnimation(nullptr)
    , m_progressEffect(nullptr)
{
    // Load theme colors
    auto theme = ThemeManager::instance();
    m_primaryColor = theme->color("primary");
    m_accentColor = theme->color("accent");
    m_textColor = theme->color("text");
    m_glowColor = theme->color("accent");

    setupUi();
    createAnimations();
}

SplashScreen::~SplashScreen()
{
    cleanupAnimations();
}

void SplashScreen::setupUi()
{
    // Create progress bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setTextVisible(false);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setFixedHeight(4);
    
    // Apply opacity effect
    m_progressEffect = new QGraphicsOpacityEffect(m_progressBar);
    m_progressEffect->setOpacity(0.0);
    m_progressBar->setGraphicsEffect(m_progressEffect);

    updateProgressBarStyle();

    // Position progress bar at the bottom
    int margin = 20;
    m_progressBar->setGeometry(
        margin,
        height() - margin - m_progressBar->height(),
        width() - 2 * margin,
        m_progressBar->height()
    );
}

void SplashScreen::updateProgressBarStyle()
{
    QString style = QString(
        "QProgressBar {"
        "   background-color: rgba(255, 255, 255, 0.2);"
        "   border: none;"
        "   border-radius: 2px;"
        "}"
        "QProgressBar::chunk {"
        "   background-color: %1;"
        "   border-radius: 2px;"
        "}"
    ).arg(m_accentColor.name());

    m_progressBar->setStyleSheet(style);
}

void SplashScreen::createAnimations()
{
    // Create animation group
    m_showAnimations = new QParallelAnimationGroup(this);

    // Message fade in animation
    m_messageAnimation = new QPropertyAnimation(this, "messageOpacity", this);
    m_messageAnimation->setDuration(800);
    m_messageAnimation->setStartValue(0.0);
    m_messageAnimation->setEndValue(1.0);
    m_messageAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_showAnimations->addAnimation(m_messageAnimation);

    // Progress bar fade in animation
    m_progressAnimation = new QPropertyAnimation(this, "progressOpacity", this);
    m_progressAnimation->setDuration(800);
    m_progressAnimation->setStartValue(0.0);
    m_progressAnimation->setEndValue(1.0);
    m_progressAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_showAnimations->addAnimation(m_progressAnimation);

    // Create glow animation timer
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, [this]() {
        // Update glow effect
        if (m_glowIncreasing)
        {
            m_glowOpacity += 0.05;
            if (m_glowOpacity >= 1.0)
            {
                m_glowOpacity = 1.0;
                m_glowIncreasing = false;
            }
        }
        else
        {
            m_glowOpacity -= 0.05;
            if (m_glowOpacity <= 0.0)
            {
                m_glowOpacity = 0.0;
                m_glowIncreasing = true;
            }
        }
        repaint();
    });
}

void SplashScreen::startAnimations()
{
    m_showAnimations->start();
    m_animationTimer->start(50);
}

void SplashScreen::cleanupAnimations()
{
    if (m_animationTimer)
    {
        m_animationTimer->stop();
    }
    if (m_showAnimations)
    {
        m_showAnimations->stop();
    }
}

void SplashScreen::setProgress(int value)
{
    m_progressBar->setValue(value);
}

void SplashScreen::showStatusMessage(const QString& message)
{
    m_message = message;
    repaint();
}

void SplashScreen::setMessageOpacity(qreal opacity)
{
    m_messageOpacity = opacity;
    repaint();
}

void SplashScreen::setProgressOpacity(qreal opacity)
{
    m_progressOpacity = opacity;
    m_progressEffect->setOpacity(opacity);
}

void SplashScreen::showEvent(QShowEvent* event)
{
    QSplashScreen::showEvent(event);
    startAnimations();
}

void SplashScreen::drawContents(QPainter* painter)
{
    // Draw base splash screen
    QSplashScreen::drawContents(painter);

    // Draw status message with glow and fade effects
    if (!m_message.isEmpty())
    {
        painter->save();
        
        // Set up font
        QFont font = painter->font();
        font.setPointSize(10);
        font.setFamily("Inter");  // Use our modern font
        painter->setFont(font);

        // Calculate text position
        QRect textRect = rect();
        textRect.setBottom(m_progressBar->y() - 10);
        
        // Draw glow effect
        QColor glowColor = m_glowColor;
        glowColor.setAlphaF(m_glowOpacity * m_messageOpacity);
        painter->setPen(glowColor);
        painter->drawText(textRect, Qt::AlignHCenter | Qt::AlignBottom, m_message);
        
        // Draw main text
        QColor textColor = m_textColor;
        textColor.setAlphaF(m_messageOpacity);
        painter->setPen(textColor);
        painter->drawText(textRect, Qt::AlignHCenter | Qt::AlignBottom, m_message);
        
        painter->restore();
    }
}

void SplashScreen::finish(QWidget* mainWindow)
{
    cleanupAnimations();
    QSplashScreen::finish(mainWindow);
}

} // namespace lmms::gui
