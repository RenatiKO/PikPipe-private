#include "qdxfviewer.h"
#include <QDebug>
#include <QString>
#include <QGraphicsTextItem>

QDxfViewer::QDxfViewer()
{
    scene_.reset(new QGraphicsScene);
//    mose_event_.reset(new QMouseEvent(QEvent::MouseTrackingChange));

    setScene(scene_.get());

    setBackgroundBrush(Qt::darkGray);

    scale(0.5, 0.5);

//    scene_->addEllipse(2000, 50, 100, 100);
//    scene_->addEllipse(50, 100, 100, 100);
}

void QDxfViewer::addLine(qreal x1, qreal y1, qreal x2, qreal y2, int w, QColor c)
{
    auto pen = QPen(c);
    pen.setWidth(w);
    scene_->addLine(x1, y1, x2, y2, pen);

//    scene_->addEllipse(x1 + (x2 - x1) / 4, y1 + (y2 - y1) / 4, 10, 10, pen);
}

void QDxfViewer::addPoint(qreal x, qreal y, qreal r, QColor c)
{
    auto pen = QPen(c);
    pen.setWidth(1);

    scene_->addEllipse(x - r, y - r, 2*r, 2*r, pen, QBrush(c, Qt::SolidPattern));
}

void QDxfViewer::clear()
{
    scene_->clear();
//    auto pen = QPen(Qt::red);
//    pen.setWidth(100);
//    scene_->addLine(0, 0, 5000, 0, pen);
//    pen.setColor(Qt::blue);
//    scene_->addLine(0, 0, 0, 5000, pen);
}

void QDxfViewer::wheelEvent(QWheelEvent *event)
{
    int angle = event->angleDelta().y();

    if (angle > 0) {
        zoomIn();
    }

    if (angle < 0) {
        zoomOut();
    }

    qDebug() << transform();
}

void QDxfViewer::mousePressEvent(QMouseEvent *event)
{
    QPointF mousePoint = this->mapToScene(event->pos());
//    qreal x;
//    qreal y;
//    mousePoint.setX(x);
//    mousePoint.setY(y);

    QPen pen(Qt::black);
    pen.setWidth(3);
    QGraphicsRectItem *rectangle;
//    rectangle = scene_->addRect(mousePoint.x(),mousePoint.y(),10,10,pen);//x,y,width,height


    QGraphicsTextItem* text = scene_->addText(QString::number(mousePoint.x()) + ";" + QString::number(mousePoint.y()));
//    scale(1, -1);

    text->setPos(mousePoint);

    emit workspaceClicked(mousePoint);

//    rectangle->setFlag(QGraphicsItem::ItemIsMovable);
}

void QDxfViewer::zoomIn(double z)
{
    scale(1. + z, 1. + z);
}

void QDxfViewer::zoomOut(double z)
{
    scale(1. - z, 1. - z);
}

//void QDxfViewer::paintEvent(QPaintEvent *event)
//{
////    QPainter p(scene_.get());

////    p.setPen(Qt::red);

////    p.drawEllipse(50, 50, 100, 100);
//}
