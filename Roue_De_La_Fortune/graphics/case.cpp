#include "case.h"
#include <QString>

int Case::width = 80;
int Case::height = 80;

#include <iostream>
Case::Case(int x, int y, int id, bool isWaittingLetter) : x(x), y(y), id(id), isWaittingLetter(isWaittingLetter)
{}

Case::~Case()
{
    if(text != nullptr) {
        delete text;
    }

    if(rect != nullptr) {
        delete rect;
    }

    if(rectAnimation != nullptr) {
        delete rectAnimation;
    }
}

void Case::drawBox(QGraphicsScene* scene) {

    if(isWaittingLetter) {
        scene->addRect(x, y, width, height);

        if(animateLetter && rectAnimation == nullptr) {
            QPen pen;
            QBrush brush(Qt::blue);
            brush.setStyle(Qt::SolidPattern);
            rectAnimation = scene->addRect(x, y, width, height, pen, brush);
        }else if(showLetter && rectAnimation != nullptr) {
            delete rectAnimation;
            rectAnimation = nullptr;
        }

        if(text == nullptr && showLetter) {
            text = scene->addText(QString(letter));
            text->setScale(4);
            text->setPos(x + text->boundingRect().width() / 2,
                         y);
        }


    }else if(rect == nullptr) {
        QPen pen;
        QBrush brush(Qt::cyan);
        brush.setStyle(Qt::SolidPattern);
        rect = scene->addRect(x, y, width, height, pen, brush);
    }
}

void Case::setLetter(char letter) {   
    this->letter = letter;
}

void Case::displayLetter() {
    this->showLetter = true;
    this->animateLetter = false;
}

void Case::displayLetterAnimation()
{
    this->animateLetter = true;
}

int Case::getId()
{
    return id;
}

char Case::getLetter() const
{
    return letter;
}
