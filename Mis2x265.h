#ifndef MIS2X265_H
#define MIS2X265_H

#include <QMainWindow>
#include <QStringList>
#include <QString>
#include <QHash>
#include <QProcess>
#include "ui_Mis2x265.h"

class Mis2x265 : public QMainWindow
{
  Q_OBJECT
  public:
    Mis2x265(QWidget *parent = 0);
    ~Mis2x265();

  private:
    Ui::Mis2x265Class ui;
    QStringList m_simple;
    QHash<QString, QString> m_special;
    QString m_mediainfo;
    QString m_mediainfoOutput;
    QProcess* m_process;
    QString m_inputFile;
    QString buildCall();
    void analyseMediaInfoOutput();

  private slots:
    void on_fileInputRadioButton_toggled(bool checked);
    void on_manualRadioButton_toggled(bool checked);
    void on_convertPushButton_clicked();
    void on_selectAndImportPushButton_clicked();
    void on_selectMediaInfoPushButton_clicked();
    void collectOutput();
    void processFinished(const int exitCode, const QProcess::ExitStatus exitStatus);
};

#endif // MIS2X265_H
