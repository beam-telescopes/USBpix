#ifdef __VISUALC__
#include "stdafx.h"
#endif
#include <QtCore>
#include "main.h"
#include "MainForm.h"
//#include "debug.h"



int main(int argc, char *argv[])
{
#ifdef __VISUALC__
    MyQApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif
    MainForm MyForm;
#ifdef __VISUALC__
    app.myMainForm = &MyForm;
    app.doRegisterForDevNotification();
#endif
    MyForm.show();
    return app.exec();
}

