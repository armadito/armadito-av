#ifndef _MODEL_COUNTER_H_
#define _MODEL_COUNTER_H_

#include <QObject>

class Counter : public QObject
{
  Q_OBJECT

 public:
  Counter() : _value(0) {}
  
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
