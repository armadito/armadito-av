#include "counter.h"

void Counter::set(int value)
{
  if (value != _value) {
    _value = value;
    emit changed(QString::number(_value));
    emit changed(_value);
  }
}
