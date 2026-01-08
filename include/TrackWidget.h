#pragma once

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <cmath>

class TrackWidget : public QWidget {
    Q_OBJECT

public:
    explicit TrackWidget(QWidget *parent = nullptr);
    
    void setProgress(double current, double total);
    
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPointF getPositionOnTrack(double percent, int centerX, int centerY, 
                               int trackWidth, int trackHeight, int trackThickness, bool outer);
    
    double m_current_km;
    double m_total_km;
    double m_progress_percent;
};
