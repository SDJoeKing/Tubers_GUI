#ifndef TMATH_H
#define TMATH_H

#include <QList>
#include <vector>
#include <QString>
#include <QtMath>
#include <cmath>
#include <eigen3/Eigen/Dense>

using namespace Eigen;
namespace MATH{

bool populateMAT(QList<ArrayXXf> &dataArr, QString path);

void generateLookTable(QList<ArrayXXf> &arr,
                       float speed, float pitch,
                       float offsetX, float offsetY, float resolution);
void TFM(QList<ArrayXXf> &src,
         ArrayXXf &dest,
         QList<ArrayXXf> &lookUpTable, float fs);

}

#endif // TMATH_H


