#include "wheel.h"

Wheel::Wheel(std::string imagePath, int nombreCase, std::vector<std::string> cases)
    : nombreCase(nombreCase)
{
    tailleCase = 360 / nombreCase;
    this->cases = cases;

    rotationStep = tailleCase / 2;//???

    pixmap = new QPixmap(QString::fromStdString(imagePath));
    item = new QGraphicsPixmapItem(pixmap->scaled(SCALE_WIDTH, SCALE_HEIGHT));
    item->setTransformationMode(Qt::TransformationMode::SmoothTransformation);
}

Wheel::~Wheel() {
    delete pixmap;
    delete item;
}

std::string Wheel::getCaseFromRotation()
{
    return cases.at(std::floor((float)rotationStep / (float)tailleCase));
}

QGraphicsPixmapItem* Wheel::getItem() {
    return item;
}

void Wheel::rotate(int value) {
    rotationStep += value;
    if(rotationStep >= 360) rotationStep = 0;
    item->setRotation(-rotationStep);
}

void Wheel::setPosition(int x, int y) {
    item->setPos(x - SCALE_WIDTH / 2, y - SCALE_HEIGHT / 2);
    item->setTransformOriginPoint(item->boundingRect().center());
}

int Wheel::getRotationStep()
{
    return rotationStep;
}
