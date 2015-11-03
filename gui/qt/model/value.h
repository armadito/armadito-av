#ifndef _MODEL_VALUE_H_
#define _MODEL_VALUE_H_

#include <QObject>

class Value : public QObject
{
  Q_OBJECT

 public:
  Value() : _value(0) {}
  
  int value() const { return _value; }

  void set(int value);

  void increment() { set(_value + 1); }

 signals:
  void changed(const QString &newValue);
  void changed(int newValue);

 private:
  int _value;
};

#endif
