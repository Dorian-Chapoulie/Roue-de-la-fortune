#ifndef WHEEL_H
#define WHEEL_H
#include <vector>
#include <string>
#include <QPixmap>
#include <QGraphicsTextItem>

//The wheel
class Wheel
{
public:
    Wheel(std::string imagePath, int nombreCase, std::vector<std::string> cases);
    ~Wheel();

    //Returns the value of the case from the current wheel angle
    std::string getCaseFromRotation();
    //This is the picture of the wheel
    QGraphicsPixmapItem* getItem();
    //We can rotate the wheel
    void rotate(int value);
    void setPosition(int x, int y);
    int getRotationStep();

private:
    int nombreCase = 0;
    int tailleCase = 0;
    int rotationStep = 0;

    const int SCALE_WIDTH = 250;
    const int SCALE_HEIGHT = 250;

    std::vector<std::string> cases;

    QPixmap* pixmap = nullptr;
    QGraphicsPixmapItem* item = nullptr;

};

#endif // WHEEL_H
