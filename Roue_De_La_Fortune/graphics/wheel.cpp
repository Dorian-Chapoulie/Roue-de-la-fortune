#include "wheel.h"

Wheel::Wheel(std::string imagePath, int nombreCase, std::vector<std::string> cases)
    : nombreCase(nombreCase)
{
    tailleCase = 360 / nombreCase;
    this->cases = cases;

    //rotationStep = nombreCase / 2;//???

    pixmap = new QPixmap(QString::fromStdString(imagePath));
    item = new QGraphicsPixmapItem(pixmap->scaled(SCALE_WIDTH, SCALE_HEIGHT));
    item->setTransformationMode(Qt::TransformationMode::SmoothTransformation);
}
#include <iostream>
Wheel::~Wheel() {
    delete pixmap;
    if(!item) {
        std::cout << "1" << std::endl;
    }
    if(item == nullptr) {
        std::cout << "2" << std::endl;
    }
    //delete item;
}

std::string Wheel::getCaseFromRotation()
{    
    int pos = (float)(rotationStep + 7) / (float)tailleCase;

    if(pos >= cases.size() - 1) {
        pos = cases.size() - 1;
    }
    if(rotationStep >= 353) {
        pos = 0;
    }
    return cases.at(pos);
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
