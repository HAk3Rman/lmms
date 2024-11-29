/*
 * Knob.cpp - powerful knob-widget
 *
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "Knob.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QMenu>
#include <QAction>
#include <QClipboard>

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif

#include "lmms_math.h"
#include "DeprecationHelper.h"
#include "embed.h"
#include "FontHelper.h"


namespace lmms::gui
{

Knob::Knob( KnobType _knob_num, QWidget * _parent, const QString & _name ) :
	FloatModelEditorBase(DirectionOfManipulation::Vertical, _parent, _name),
	m_label( "" ),
	m_isHtmlLabel(false),
	m_tdRenderer(nullptr),
	m_angle( -10 ),
	m_lineWidth( 0 ),
	m_textColor( 255, 255, 255 ),
	m_knobNum( _knob_num ),
	m_valueAnimation(nullptr)
{
	initUi( _name );
}

Knob::Knob( QWidget * _parent, const QString & _name ) :
	Knob( KnobType::Bright26, _parent, _name )
{
}

void Knob::initUi( const QString & _name )
{
	onKnobNumUpdated();
	setTotalAngle( 270.0f );
	setInnerRadius( 1.0f );
	setOuterRadius( 10.0f );

	// Modern styling with theme-aware colors
	auto pal = QApplication::palette();
	
	// Base colors from theme
	QColor primary = pal.color(QPalette::Active, QPalette::Highlight);
	QColor text = pal.color(QPalette::Active, QPalette::WindowText);
	QColor background = pal.color(QPalette::Active, QPalette::Window);
	
	// Enhanced colors with alpha
	m_lineActiveColor = primary;
	m_lineInactiveColor = text;
	m_lineInactiveColor.setAlpha(60);
	
	m_arcActiveColor = primary;
	m_arcActiveColor.setAlpha(40);
	m_arcInactiveColor = text; 
	m_arcInactiveColor.setAlpha(20);
	
	// Modern styling
	setLineWidth(2.5);
	
	// Set up animations
	m_valueAnimation = new QPropertyAnimation(this, "value", this);
	m_valueAnimation->setDuration(150); // 150ms for smooth transition
	m_valueAnimation->setEasingCurve(QEasingCurve::OutCubic);
	
	// Enhanced tooltip
	QString tooltip = _name;
	if (!tooltip.isEmpty()) {
		tooltip += "\n";
	}
	tooltip += tr("Right-click to reset");
	tooltip += "\n" + tr("Mouse wheel for fine-adjustment");
	setToolTip(tooltip);
	
	// Set up context menu
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
			this, SLOT(showContextMenu(const QPoint &)));
}

void Knob::onKnobNumUpdated()
{
	if( m_knobNum != KnobType::Styled )
	{
		QString knobFilename;
		switch (m_knobNum)
		{
		case KnobType::Dark28:
			knobFilename = "knob01";
			break;
		case KnobType::Bright26:
			knobFilename = "knob02";
			break;
		case KnobType::Small17:
			knobFilename = "knob03";
			break;
		case KnobType::Vintage32:
			knobFilename = "knob05";
			break;
		case KnobType::Styled: // only here to stop the compiler from complaining
			break;
		}

		// If knobFilename is still empty here we should get the fallback pixmap of size 1x1
		m_knobPixmap = std::make_unique<QPixmap>(QPixmap(embed::getIconPixmap(knobFilename.toUtf8().constData())));
		if (!this->isEnabled())
		{
			convertPixmapToGrayScale(*m_knobPixmap.get());
		}
		setFixedSize( m_knobPixmap->width(), m_knobPixmap->height() );
	}
}

void Knob::setLabel( const QString & txt )
{
	m_label = txt;
	m_isHtmlLabel = false;
	if( m_knobPixmap )
	{
		setFixedSize(qMax<int>( m_knobPixmap->width(),
					horizontalAdvance(QFontMetrics(adjustedToPixelSize(font(), SMALL_FONT_SIZE)), m_label)),
						m_knobPixmap->height() + 10);
	}

	update();
}

void Knob::setHtmlLabel(const QString &htmltxt)
{
	m_label = htmltxt;
	m_isHtmlLabel = true;
	// Put the rendered HTML content into cache
	if (!m_tdRenderer)
	{
		m_tdRenderer = new QTextDocument(this);
	}

	m_tdRenderer->setHtml(QString("<span style=\"color:%1;\">%2</span>").arg(textColor().name(), m_label));

	if (m_knobPixmap)
	{
		setFixedSize(m_knobPixmap->width(),
				m_knobPixmap->height() + 15);
	}

	update();
}

void Knob::setTotalAngle( float angle )
{
	if( angle < 10.0 )
	{
		m_totalAngle = 10.0;
	}
	else
	{
		m_totalAngle = angle;
	}

	update();
}

float Knob::innerRadius() const
{
	return m_innerRadius;
}

void Knob::setInnerRadius( float r )
{
	m_innerRadius = r;
}

float Knob::outerRadius() const
{
	return m_outerRadius;
}

void Knob::setOuterRadius( float r )
{
	m_outerRadius = r;
}

KnobType Knob::knobNum() const
{
	return m_knobNum;
}

void Knob::setknobNum( KnobType k )
{
	if( m_knobNum != k )
	{
		m_knobNum = k;
		onKnobNumUpdated();
	}
}

QPointF Knob::centerPoint() const
{
	return m_centerPoint;
}

float Knob::centerPointX() const
{
	return m_centerPoint.x();
}

void Knob::setCenterPointX( float c )
{
	m_centerPoint.setX( c );
}

float Knob::centerPointY() const
{
	return m_centerPoint.y();
}

void Knob::setCenterPointY( float c )
{
	m_centerPoint.setY( c );
}

float Knob::lineWidth() const
{
	return m_lineWidth;
}

void Knob::setLineWidth( float w )
{
	m_lineWidth = w;
}

QColor Knob::outerColor() const
{
	return m_outerColor;
}

void Knob::setOuterColor( const QColor & c )
{
	m_outerColor = c;
}

QColor Knob::textColor() const
{
	return m_textColor;
}

void Knob::setTextColor( const QColor & c )
{
	m_textColor = c;
}

QLineF Knob::calculateLine( const QPointF & _mid, float _radius, float _innerRadius ) const
{
	const float rarc = m_angle * F_PI / 180.0;
	const float ca = cos( rarc );
	const float sa = -sin( rarc );

	return QLineF( _mid.x() - sa*_innerRadius, _mid.y() - ca*_innerRadius,
					_mid.x() - sa*_radius, _mid.y() - ca*_radius );
}

bool Knob::updateAngle()
{
	int angle = 0;
	if( model() && model()->maxValue() != model()->minValue() )
	{
		angle = angleFromValue( model()->inverseScaledValue( model()->value() ), model()->minValue(), model()->maxValue(), m_totalAngle );
	}
	if( qAbs( angle - m_angle ) > 0 )
	{
		m_angle = angle;
		return true;
	}
	return false;
}

void Knob::drawKnob( QPainter * _p )
{
	bool enabled = this->isEnabled();
	QColor currentArcColor = enabled ? m_arcActiveColor : m_arcInactiveColor;
	QColor currentLineColor = enabled ? m_lineActiveColor : m_lineInactiveColor;

	if( updateAngle() == false && !m_cache.isNull() )
	{
		_p->drawImage( 0, 0, m_cache );
		return;
	}

	m_cache = QImage( size(), QImage::Format_ARGB32 );
	m_cache.fill( qRgba( 0, 0, 0, 0 ) );

	QPainter p( &m_cache );
	p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	// Draw modern styled knob
	const float radius = width() / 2.0f - 2;
	QPointF center(width() / 2.0f, height() / 2.0f);
	
	// Draw background arc
	QPen arcPen(currentArcColor, 3);
	arcPen.setCapStyle(Qt::RoundCap);
	p.setPen(arcPen);
	p.drawArc(QRectF(center.x() - radius, center.y() - radius, 
					 radius * 2, radius * 2), 
			  -45 * 16, -270 * 16);

	// Draw value arc with gradient
	QConicalGradient gradient(center, -45);
	gradient.setColorAt(0, currentLineColor);
	QColor endColor = currentLineColor;
	endColor.setAlpha(120);
	gradient.setColorAt(0.8, endColor);
	
	QPen valuePen(gradient, 3);
	valuePen.setCapStyle(Qt::RoundCap);
	p.setPen(valuePen);
	
	// Calculate angles
	const int startAngle = -45 * 16;
	const int spanAngle = static_cast<int>(-m_angle * 16);
	
	p.drawArc(QRectF(center.x() - radius, center.y() - radius,
					 radius * 2, radius * 2),
			  startAngle, spanAngle);

	// Draw center dot
	p.setPen(Qt::NoPen);
	p.setBrush(currentLineColor);
	p.drawEllipse(center, 2, 2);
	
	// Draw indicator line
	p.setPen(QPen(currentLineColor, 2, Qt::SolidLine, Qt::RoundCap));
	QLineF indicator = calculateLine(center, radius * 0.8);
	p.drawLine(indicator);

	// Draw label with proper text rendering
	if (!m_label.isEmpty())
	{
		p.setFont(adjustedToPixelSize(p.font(), DEFAULT_FONT_SIZE));
		p.setPen(enabled ? m_textColor : m_textColor.darker(150));
		
		if (m_isHtmlLabel)
		{
			if (!m_tdRenderer)
			{
				m_tdRenderer = new QTextDocument;
			}
			m_tdRenderer->setHtml(m_label);
			m_tdRenderer->setDefaultFont(p.font());
			m_tdRenderer->setTextWidth(width());
			
			p.translate(0, height() - m_tdRenderer->size().height());
			m_tdRenderer->drawContents(&p);
		}
		else
		{
			p.drawText(QRect(0, height() - 15, width(), 15),
					  Qt::AlignCenter, m_label);
		}
	}

	p.end();

	_p->drawImage( 0, 0, m_cache );
}

void Knob::paintEvent( QPaintEvent * _me )
{
	QPainter p( this );

	drawKnob( &p );
	if( !m_label.isEmpty() )
	{
		if (!m_isHtmlLabel)
		{
			p.setFont(adjustedToPixelSize(p.font(), SMALL_FONT_SIZE));
			p.setPen(textColor());
			p.drawText(width() / 2 -
				horizontalAdvance(p.fontMetrics(), m_label) / 2,
				height() - 2, m_label);
		}
		else
		{
			// TODO setHtmlLabel is never called so this will never be executed. Remove functionality?
			m_tdRenderer->setDefaultFont(adjustedToPixelSize(p.font(), SMALL_FONT_SIZE));
			p.translate((width() - m_tdRenderer->idealWidth()) / 2, (height() - m_tdRenderer->pageSize().height()) / 2);
			m_tdRenderer->drawContents(&p);
		}
	}
}

void Knob::changeEvent(QEvent * ev)
{
	if (ev->type() == QEvent::EnabledChange)
	{
		onKnobNumUpdated();
		if (!m_label.isEmpty())
		{
			setLabel(m_label);
		}
		m_cache = QImage();
		update();
	}
}

void Knob::mousePressEvent( QMouseEvent * _me )
{
	if( _me->button() == Qt::RightButton )
	{
		// Reset to default value with right click
		model()->reset();
		_me->accept();
		return;
	}
	
	FloatModelEditorBase::mousePressEvent( _me );
	
	// Show value tooltip
	updateValueToolTip(_me->globalPos());
}

void Knob::mouseMoveEvent( QMouseEvent * _me )
{
	FloatModelEditorBase::mouseMoveEvent( _me );
	
	// Update tooltip position
	updateValueToolTip(_me->globalPos());
}

void Knob::wheelEvent( QWheelEvent * _we )
{
	_we->accept();
	const int delta = (_we->angleDelta().y() > 0) ? 1 : -1;
	
	if( _we->modifiers() & Qt::ShiftModifier )
	{
		model()->incValue( delta * model()->step<float>() * 0.1f );
	}
	else
	{
		model()->incValue( delta * model()->step<float>() );
	}
	
	// Update tooltip
	updateValueToolTip(QCursor::pos());
}

void Knob::updateValueToolTip(const QPoint & pos)
{
	QString tooltip = QString::number(model()->value(), 'f', 2);
	if (!m_hintTextBeforeValue.isEmpty())
	{
		tooltip = m_hintTextBeforeValue + " " + tooltip;
	}
	if (!m_hintTextAfterValue.isEmpty())
	{
		tooltip += " " + m_hintTextAfterValue;
	}
	
	QToolTip::showText(pos, tooltip, this);
}

void Knob::showContextMenu(const QPoint & pos)
{
	QMenu menu(this);
	
	QAction *resetAction = menu.addAction(tr("Reset to default"));
	connect(resetAction, SIGNAL(triggered()), model(), SLOT(reset()));
	
	menu.addSeparator();
	
	QAction *copyAction = menu.addAction(tr("Copy value"));
	connect(copyAction, &QAction::triggered, [this]() {
		QApplication::clipboard()->setText(
			QString::number(model()->value()));
	});
	
	QAction *pasteAction = menu.addAction(tr("Paste value"));
	connect(pasteAction, &QAction::triggered, [this]() {
		QString text = QApplication::clipboard()->text();
		bool ok;
		float value = text.toFloat(&ok);
		if (ok) {
			model()->setValue(value);
		}
	});
	
	menu.exec(mapToGlobal(pos));
}

void convertPixmapToGrayScale(QPixmap& pixMap)
{
	QImage temp = pixMap.toImage().convertToFormat(QImage::Format_ARGB32);
	for (int i = 0; i < temp.height(); ++i)
	{
		for (int j = 0; j < temp.width(); ++j)
		{
			const auto pix = temp.pixelColor(i, j);
			const auto gscale = 0.2126 * pix.redF() + 0.7152 * pix.greenF() + 0.0722 * pix.blueF();
			const auto pixGray = QColor::fromRgbF(gscale, gscale, gscale, pix.alphaF());
			temp.setPixelColor(i, j, pixGray);
		}
	}
	pixMap.convertFromImage(temp);
}

} // namespace lmms::gui
