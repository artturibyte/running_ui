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
    
    // Draw background track (dark grey)
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(60, 60, 60));
    
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
    
    // Draw required pace marker (cyan line with glow)
    if (requiredPercent > 0 && requiredPercent <= 100) {
        QPointF outerPos = getPositionOnTrack(requiredPercent, centerX, centerY, trackWidth, trackHeight, trackThickness, true);
        QPointF innerPos = getPositionOnTrack(requiredPercent, centerX, centerY, trackWidth, trackHeight, trackThickness, false);
        
        // two lines for glow effect
        painter.setPen(QPen(QColor(0, 255, 255, 100), 3));
        painter.drawLine(outerPos, innerPos);
        painter.setPen(QPen(QColor(0, 255, 255), 1));
        painter.drawLine(outerPos, innerPos);
    }
    
    // Draw current progress marker (yellow line with glow)
    if (m_progress_percent > 0 && m_progress_percent <= 100) {
        QPointF outerPos = getPositionOnTrack(m_progress_percent, centerX, centerY, trackWidth, trackHeight, trackThickness, true);
        QPointF innerPos = getPositionOnTrack(m_progress_percent, centerX, centerY, trackWidth, trackHeight, trackThickness, false);
        
        painter.setPen(QPen(QColor(170, 170, 0, 100), 3));
        painter.drawLine(outerPos, innerPos);
        painter.setPen(QPen(QColor(170, 170, 0), 1));
        painter.drawLine(outerPos, innerPos);
    }
    
    // Draw center text
    painter.setPen(QColor(170, 170, 0));  // Toned-down yellow
    QFont font = painter.font();
    font.setPointSize(24);
    font.setBold(true);
    font.setFamily("Monospace");
    painter.setFont(font);
    
    QString progressText = QString::number(m_progress_percent, 'f', 1) + "%";
    QRect textRect(centerX - 100, centerY - 40, 200, 50);
    painter.drawText(textRect, Qt::AlignCenter, progressText);
    
    // Draw km text
    font.setPointSize(12);
    font.setBold(false);
    painter.setFont(font);
    painter.setPen(QColor(0, 255, 136));  // Cyan-green
    QString kmText = QString::number(m_current_km, 'f', 1) + " / " + 
                     QString::number(m_total_km, 'f', 0) + " km";
    QRect kmRect(centerX - 100, centerY + 10, 200, 30);
    painter.drawText(kmRect, Qt::AlignCenter, kmText);
    
    // Draw color legend at bottom left
    int legendX = 15;
    int dotSize = 12;
    int lineHeight = 25;
    int legendY = height - 2 * lineHeight - 20;  // Position from bottom
    
    font.setPointSize(10);
    font.setBold(false);
    painter.setFont(font);
    
    // Yellow runner (current progress)
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(170, 170, 0));
    painter.drawEllipse(QPointF(legendX + dotSize/2, legendY + dotSize/2), dotSize/2, dotSize/2);
    painter.setPen(QColor(200, 200, 200));
    painter.drawText(legendX + dotSize + 8, legendY + dotSize + 2, "Your progress");
    
    // Cyan runner (required pace)
    legendY += lineHeight;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 255, 255));
    painter.drawEllipse(QPointF(legendX + dotSize/2, legendY + dotSize/2), dotSize/2, dotSize/2);
    painter.setPen(QColor(200, 200, 200));
    painter.drawText(legendX + dotSize + 8, legendY + dotSize + 2, "Required pace");
    
    // Draw startline
    painter.setPen(QPen(QColor(255, 255, 255, 150), 5));
    painter.drawLine(centerX, centerY + trackHeight/2 - trackThickness,
                     centerX, centerY + trackHeight/2 );
    
    // Draw dimension lines
    int dimOffset = 20;  // Distance from track edge to dimension line
    int tickSize = 8;    // Size of end ticks
    
    // Calculate track perimeter to scale dimensions to 1000km
    double straightLength = trackWidth - trackHeight;
    double semiCircleLength = M_PI * (trackHeight / 2.0);
    double totalPerimeter = 2 * straightLength + 2 * semiCircleLength;
    double kmPerPixel = 1000.0 / totalPerimeter;
    
    // Top horizontal dimension line (showing width)
    int topLineY = centerY - trackHeight/2 - dimOffset;
    int leftX = centerX - trackWidth/2;
    int rightX = centerX + trackWidth/2;
    
    painter.setPen(QPen(QColor(85, 85, 85), 1));
    // Main horizontal line
    painter.drawLine(leftX, topLineY, rightX, topLineY);
    // Left tick
    painter.drawLine(leftX, topLineY - tickSize/2, leftX, topLineY + tickSize/2);
    // Right tick
    painter.drawLine(rightX, topLineY - tickSize/2, rightX, topLineY + tickSize/2);
    
    // Draw width measurement text (scaled to km)
    font.setPointSize(9);
    painter.setFont(font);
    painter.setPen(QColor(120, 120, 120));
    double widthKm = trackWidth * kmPerPixel;
    QString widthText = QString::number(widthKm, 'f', 2) + " km";
    QRect widthRect(centerX - 40, topLineY - 20, 80, 15);
    painter.drawText(widthRect, Qt::AlignCenter, widthText);
    
    // Right vertical dimension line (showing height)
    int rightLineX = centerX + trackWidth/2 + dimOffset;
    int topY = centerY - trackHeight/2;
    int bottomY = centerY + trackHeight/2;
    
    painter.setPen(QPen(QColor(85, 85, 85), 1));
    // Main vertical line
    painter.drawLine(rightLineX, topY, rightLineX, bottomY);
    // Top tick
    painter.drawLine(rightLineX - tickSize/2, topY, rightLineX + tickSize/2, topY);
    // Bottom tick
    painter.drawLine(rightLineX - tickSize/2, bottomY, rightLineX + tickSize/2, bottomY);
    
    // Draw height measurement text (rotated, scaled to km)
    painter.save();
    painter.translate(rightLineX + 25, centerY);
    painter.rotate(-90);
    painter.setPen(QColor(120, 120, 120));
    double heightKm = trackHeight * kmPerPixel;
    QString heightText = QString::number(heightKm, 'f', 2) + " km";
    QRect heightRect(-40, -8, 80, 15);
    painter.drawText(heightRect, Qt::AlignCenter, heightText);
    painter.restore();
}
