#include "TrackWidget.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QDate>

TrackWidget::TrackWidget(QWidget *parent)
    : QWidget(parent),
      m_current_km(0.0),
      m_total_km(1000.0),
      m_progress_percent(0.0)
{
    setMinimumSize(400, 350);
}

void TrackWidget::setProgress(double current, double total) {
    m_current_km = current;
    m_total_km = total;
    m_progress_percent = (total > 0) ? (current / total) * 100.0 : 0.0;
    update();
}

QSize TrackWidget::sizeHint() const {
    return QSize(500, 450);
}

QSize TrackWidget::minimumSizeHint() const {
    return QSize(400, 350);
}

QPointF TrackWidget::getPositionOnTrack(double percent, int centerX, int centerY, 
                                        int trackWidth, int trackHeight, int trackThickness, bool outer) {
    // Calculate position along the track perimeter
    double straightLength = trackWidth - trackHeight;
    double semiCircleLength = M_PI * (trackHeight / 2.0);
    double totalPerimeter = 2 * straightLength + 2 * semiCircleLength;
    double distance = (percent / 100.0) * totalPerimeter;
    
    double remaining = distance;
    int radius = outer ? trackHeight/2 : (trackHeight/2 - trackThickness);
    
    // Section 1: Bottom center to right (going right along bottom straight)
    double halfBottomStraight = straightLength / 2.0;
    if (remaining <= halfBottomStraight) {
        return QPointF(centerX + remaining, centerY + radius);
    }
    remaining -= halfBottomStraight;
    
    // Section 2: Right semicircle (going clockwise up the right side)
    if (remaining <= semiCircleLength) {
        // Start at 90 degrees (bottom in Qt coords), go clockwise (decreasing angle) to 270 degrees (top)
        double angle = 90 - (remaining / semiCircleLength) * 180.0;
        double rad = angle * M_PI / 180.0;
        int arcRadius = outer ? trackHeight/2 : (trackHeight/2 - trackThickness);
        return QPointF(centerX + trackWidth/2 - trackHeight/2 + arcRadius * cos(rad),
                      centerY + arcRadius * sin(rad));
    }
    remaining -= semiCircleLength;
    
    // Section 3: Top straight (right to left)
    if (remaining <= straightLength) {
        return QPointF(centerX + trackWidth/2 - trackHeight/2 - remaining, centerY - radius);
    }
    remaining -= straightLength;
    
    // Section 4: Left semicircle (going clockwise down the left side)
    if (remaining <= semiCircleLength) {
        // Start at 270 degrees (top), go clockwise (increasing back to 90) to bottom
        double angle = 270 - (remaining / semiCircleLength) * 180.0;
        double rad = angle * M_PI / 180.0;
        int arcRadius = outer ? trackHeight/2 : (trackHeight/2 - trackThickness);
        return QPointF(centerX - trackWidth/2 + trackHeight/2 + arcRadius * cos(rad),
                      centerY + arcRadius * sin(rad));
    }
    remaining -= semiCircleLength;
    
    // Section 5: Bottom straight from left to center
    return QPointF(centerX - trackWidth/2 + trackHeight/2 + remaining, centerY + radius);
}

void TrackWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int width = this->width();
    int height = this->height();
    int size = std::min(width, height) - 20;
    int centerX = width / 2;
    int centerY = height / 2;
    
    // Track dimensions - stadium shape (larger)
    int trackWidth = size * 1.4;
    int trackHeight = size * 0.8;
    int trackThickness = 35;
    int cornerRadius = trackHeight / 2;  // Semicircular ends
    
    QRect outerRect(centerX - trackWidth/2, centerY - trackHeight/2, trackWidth, trackHeight);
    QRect innerRect(centerX - trackWidth/2 + trackThickness, 
                    centerY - trackHeight/2 + trackThickness, 
                    trackWidth - 2*trackThickness, 
                    trackHeight - 2*trackThickness);
    
    // Draw background track (gray)
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(220, 220, 220));
    
    QPainterPath outerPath;
    outerPath.addRoundedRect(outerRect, cornerRadius, cornerRadius);
    
    QPainterPath innerPath;
    innerPath.addRoundedRect(innerRect, cornerRadius - trackThickness, cornerRadius - trackThickness);
    
    QPainterPath trackPath = outerPath.subtracted(innerPath);
    painter.drawPath(trackPath);
    
    // Calculate required pace position (based on day of year)
    QDate startOfYear(QDate::currentDate().year(), 1, 1);
    int dayOfYear = startOfYear.daysTo(QDate::currentDate()) + 1;
    double requiredKm = (dayOfYear / 365.0) * m_total_km;
    double requiredPercent = (m_total_km > 0) ? (requiredKm / m_total_km) * 100.0 : 0.0;
    
    // Draw required pace marker (red line)
    if (requiredPercent > 0 && requiredPercent <= 100) {
        QPointF outerPos = getPositionOnTrack(requiredPercent, centerX, centerY, trackWidth, trackHeight, trackThickness, true);
        QPointF innerPos = getPositionOnTrack(requiredPercent, centerX, centerY, trackWidth, trackHeight, trackThickness, false);
        
        painter.setPen(QPen(QColor(244, 67, 54), 5));  // Red
        painter.drawLine(outerPos, innerPos);
    }
    
    // Draw current progress marker (green line)
    if (m_progress_percent > 0 && m_progress_percent <= 100) {
        QPointF outerPos = getPositionOnTrack(m_progress_percent, centerX, centerY, trackWidth, trackHeight, trackThickness, true);
        QPointF innerPos = getPositionOnTrack(m_progress_percent, centerX, centerY, trackWidth, trackHeight, trackThickness, false);
        
        painter.setPen(QPen(QColor(76, 175, 80), 5));  // Green
        painter.drawLine(outerPos, innerPos);
    }
    
    // Draw center text
    painter.setPen(QColor(60, 60, 60));
    QFont font = painter.font();
    font.setPointSize(24);
    font.setBold(true);
    painter.setFont(font);
    
    QString progressText = QString::number(m_progress_percent, 'f', 1) + "%";
    QRect textRect(centerX - 100, centerY - 40, 200, 50);
    painter.drawText(textRect, Qt::AlignCenter, progressText);
    
    // Draw km text
    font.setPointSize(12);
    font.setBold(false);
    painter.setFont(font);
    QString kmText = QString::number(m_current_km, 'f', 1) + " / " + 
                     QString::number(m_total_km, 'f', 0) + " km";
    QRect kmRect(centerX - 100, centerY + 10, 200, 30);
    painter.drawText(kmRect, Qt::AlignCenter, kmText);
    
    // Draw start marker at bottom center
    painter.setPen(QPen(QColor(255, 255, 255), 4));
    painter.drawLine(centerX - 8, centerY + trackHeight/2 - trackThickness - 2,
                     centerX + 8, centerY + trackHeight/2 - trackThickness - 2);
    painter.setPen(QPen(QColor(100, 100, 100), 2));
    painter.drawLine(centerX - 8, centerY + trackHeight/2 - trackThickness - 2,
                     centerX + 8, centerY + trackHeight/2 - trackThickness - 2);
}
