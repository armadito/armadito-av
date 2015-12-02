#include "ui/app.h"

int main(int argc, char *argv[])
{
  try {
    UhuruApplication::init(argc, argv);
  }
  catch (UhuruApplicationInitializationFailure &e) {
    return 1;
  }

  return UhuruApplication::instance()->exec();
}
