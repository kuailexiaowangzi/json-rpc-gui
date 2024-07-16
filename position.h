#ifndef POSITION_H
#define POSITION_H

#include <QObject>

class Position : public QObject
{
    Q_OBJECT
public:
    explicit Position(QObject *parent = nullptr);

    enum class positionType
    {
        none,
        top,
        left,
        right,
        bottom,
        topLeft,
        topRight,
        bottomLeft,
        bottomRight,
        topBottom,
        maximize
    };
    Q_ENUM(positionType);
};

#endif // POSITION_H
