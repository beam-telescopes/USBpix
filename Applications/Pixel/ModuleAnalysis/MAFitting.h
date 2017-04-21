#ifndef MAFITTING_H
#define MAFITTING_H

#include <qobject.h>
#include <qthread.h>

class QApplication;
class QString;
class optClass;
class ModItem;
class PixDBData;

class MAFitting : public QObject {

  Q_OBJECT

 public:

  class FitThread : public QThread {

  friend class MAFitting;

  public:
    FitThread(PixDBData &data, int fittype, int chip, float chicut, float xmin, float xmax, float fracErr, bool useNdof);
    void run();
  private:
    PixDBData &m_data;
    int m_fittype;
    int m_chip;
    float m_chicut;
    float m_xmin;
    float m_xmax;
    float m_fracErr;
    bool m_useNdof;
  };

  MAFitting(QApplication *app, optClass *opts);
  ~MAFitting(){};

 public slots:
  bool rootdbFit(ModItem *item, int loop, int chip);
  void rootdbFitAll(int chip=-1);
 
 signals:
  void sendText(QString);

 private:
  QApplication *m_app;
  optClass *m_opts;

};

#endif // FITTING_H
