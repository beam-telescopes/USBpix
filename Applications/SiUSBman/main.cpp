#include "MainForm.h"
#include "main.h"

#include <QFont>
#include <QPushButton>
#include <QWidget>

MyQApplication *app;
MainForm *MyForm;


#ifndef __CPL__
int main(int argc, char *argv[])
{
  Q_INIT_RESOURCE(SiUSBman);
  //  MyQApplication qapp(argc, argv); 
  QApplication qapp(argc, argv); 
  MainForm *MyForm = new MainForm();
// #ifdef WIN32
//   qapp.myMainForm = MyForm; 
//   qapp.doRegisterForDevNotification();
//   #if QT_VERSION > 0x050000
//     qapp.installNativeEventFilter(&qapp);
//   #endif	 
// #endif
  MyForm->show();
  return qapp.exec();
}

#else

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  reason, LPVOID lpReserved)
{
	OutputDebugStringA("DllMain() called\n");
  switch (reason)
  {
    case DLL_PROCESS_ATTACH:
        MessageBox( NULL, L"Init() called.", L"Info", MB_OK|MB_ICONINFORMATION );
				app = new MyQApplication(0, NULL); 
				MyForm = new MainForm();

				app->setWindowIcon(QPixmap(":/Icons/resources/silab.ico"));
				app->myMainForm = MyForm; 
//				MyForm->show();  // test
				app->doRegisterForDevNotification();
        break;
    case DLL_PROCESS_DETACH:
			  delete app;
        MessageBox( NULL, L"CleanUp() called.", L"Info", MB_OK|MB_ICONINFORMATION );
        break;
  }
  return 1;
}

//extern "C"  __declspec(dllexport) 
int __stdcall CPlApplet(HWND hwndCPL, int uMsg, ULONG lParam1, ULONG lParam2)
{
   LPCPLINFO lpCPlInfo;

   MessageBox( NULL, L"CPlApplet() called.", L"Info", MB_OK|MB_ICONINFORMATION );

   switch (uMsg) {
       case CPL_INIT:      // first message, sent once
           return TRUE;

       case CPL_GETCOUNT:  // second message, sent once
           return 1;
           break;

       case CPL_INQUIRE: // third message, sent once per item
           lpCPlInfo = (LPCPLINFO) lParam2;
           lpCPlInfo->lData = 0;
           lpCPlInfo->idIcon = 101;
           lpCPlInfo->idName = 102;
           lpCPlInfo->idInfo = 103;
           break;

       case CPL_DBLCLK:    // item icon double-clicked
           MessageBox( NULL, L"Double clicked", L"Info", MB_OK|MB_ICONINFORMATION );
					 MyForm->show();
           return app->exec();

       case CPL_STOP:      // sent once per item before CPL_EXIT
           break;

       case CPL_EXIT:    // sent once before FreeLibrary is called
           break;

       default:
           break;
   }
   return 0;
}
#endif

