#ifndef QDXFVIEWER_H
#define QDXFVIEWER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <memory>

class QDxfViewer : public QGraphicsView
{
    Q_OBJECT
public:
    QDxfViewer();

    void addLine(qreal x1, qreal y1, qreal x2, qreal y2, int w, QColor c);
    void addPoint(qreal x, qreal y, qreal r, QColor c);
    void clear();
Q_SIGNALS:
    void workspaceClicked(QPointF&);
private:
//    void paintEvent(QPaintEvent *event) override;
    std::shared_ptr<QGraphicsScene> scene_;

    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    void zoomIn(double z = 0.1);
    void zoomOut(double z = 0.1);
};

#endif // QDXFVIEWER_H
