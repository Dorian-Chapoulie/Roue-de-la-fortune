#ifndef CASE_H
#define CASE_H

#include <QGraphicsScene>
#include <QGraphicsTextItem>


class Case
{
public:
    Case(int x, int y, int id, bool isWaittingLetter);
    ~Case();

    void drawBox(QGraphicsScene* scene);
    void setLetter(char letter);
    void displayLetter();
    void displayLetterAnimation();    
    int getId();
    char getLetter() const;

    static int width;
    static int height;

private:
    bool isWaittingLetter = true;
    bool showLetter = false;
    bool animateLetter = false;
    char letter;

    int x = 0;
    int y = 0;
    int id = -1;

    QGraphicsTextItem* text = nullptr;
    QGraphicsRectItem* rect = nullptr;
};

#endif // CASE_H
