Compilation on Windows
======================

The UhuruAV VisualStudio solution is configured in order that all generated binary files belong to "VS/UhuruAV/Debug" folder.
Be sure to compile these following projects as parts of the whole solution :
- libuhuru_core -> libuhuru_core.lib
- libuhuru_ipc -> libuhuru_ipc.lib
- scand -> scand.exe
- uhuru_scan -> uhuru_scan.exe

(.lib = static library)

You need some external libraries(DLLs) in order to execute uhuru-av binaries :
- intl.dll ( =libintl3.dll )
- libiconv2.dll

And, from glib compilation :
- libgio-2.0-0.dll
- libglib-2.0-0.dll
- libgmodule-2.0-0.dll
- libgobject-2.0-0.dll
- libgthread-2.0-0.dll

Note :
These external libraries have to be copied in "VS/UhuruAV/Debug" folder.
Be aware of to have the exact names described ++here or change projects properties by yourself.

Usage on Windows
================
In directory, call uhuru_scan.exe or/and scand.exe.




