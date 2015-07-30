#include "Mis2x265.h"

#include <QString>
#include <QMessageBox>
#include <QFileDialog>

Mis2x265::Mis2x265(QWidget *parent)
    : QMainWindow(parent), m_simple(), m_special(), m_mediainfo(), m_process(nullptr),
        m_mediainfoOutput()
{
  ui.setupUi(this);
  this->setWindowTitle("Mis2x265 " + QString::fromLocal8Bit(BUILDVERSION) + " - by Selur");
  m_special.insert("interlace", "interlaceMode");
  m_simple << "wpp" << "ctu" << "min-cu-size" << "max-tu-size" << "tu-intra-depth"
      << "tu-inter-depth" << "me" << "subme" << "merange" << "rect" << "amp" << "max-merge"
      << "temporal-mvp" << "early-skip" << "rdpenalty" << "tskip" << "tskip-fast"
      << "strong-intra-smoothing" << "lossless" << "cu-lossless" << "constrained-intra"
      << "fast-intra" << "open-gop" << "temporal-layers" << "keyint" << "min-keyint" << "scenecut"
      << "rc-lookahead" << "lookahead-slices" << "bframes" << "bframe-bias" << "b-adapt" << "ref"
      << "limit-refs" << "weightp" << "weightb" << "aq-mode" << "aq-strength" << "cbqpoffs"
      << "crqpoffs" << "rd" << "hrd" << "psy-rd" << "rdoq-level" << "psy-rdoq" << "signhide"
      << "deblock" << "sao" << "sao-non-deblock" << "b-pyramid" << "cutree" << "bitrate" << "qcomp"
      << "qpmin" << "qpmax" << "qpstep" << "ipratio" << "pbratio" << "pme" << "qp";

  QString path = qApp->applicationDirPath();
  path += QDir::separator();
#ifdef Q_OS_WIN
  path += "mediainfo.exe";
#else
  path += "mediainfo";
#endif
  path = QDir::toNativeSeparators(path);
  if (QFile::exists(path)) {
    m_mediainfo = path;
    ui.textBrowser->append("m_mediaInfo: " + m_mediainfo);
  }
  ui.selectMediaInfoPushButton->setEnabled(m_mediainfo.isEmpty());
}

Mis2x265::~Mis2x265()
{

}

void removeStartOfLine(QString &line)
{
  line = line.remove(0, line.indexOf(":") + 1);
  line = line.trimmed();
}

void Mis2x265::analyseMediaInfoOutput()
{
  QStringList lines = m_mediainfoOutput.split("\n", QString::SkipEmptyParts);
  QString addition, tmp;
  bool video = false;
  foreach(QString line, lines)
  {
    if (!video) {
      if (line.startsWith("Video #") || (line.startsWith("Video") && line.trimmed().size() == 5)) {
        video = true;
      }
      continue;
    }
    ui.textBrowser->append(line);
    if (line.startsWith("Format profile")) {
      removeStartOfLine(line);
      QStringList elems = line.split("@");
      if (elems.count() == 3) {
        addition += " --profile " + elems.at(0).toLower();
        tmp = elems.at(1);
        tmp = tmp.remove("L");
        addition += " --level " + tmp;
        tmp = elems.at(2).trimmed();
        if (tmp.toLower() == "main") {
          addition += " --no-high-tier";
        } else {
          addition += " --high-tier";
        }
      }
      continue;
    }
    if (line.startsWith("Frame rate") && line.contains("fps")) {
      removeStartOfLine(line);
      line = line.remove("fps");
      line = line.trimmed();
      addition += " --fps " + line;
      continue;
    }
    if (line.startsWith("Color range")) {
      removeStartOfLine(line);
      if (line.toLower() == "limited") {
        addition += " --range limited";
      } else {
        addition += " --range full";
      }
      continue;
    }
    if (line.startsWith("Chroma subsampling")) {
      removeStartOfLine(line);
      line = line.remove(":");
      addition += " --input-csp i" + line;
      continue;
    }
    if (line.startsWith("Bit depth") && !line.contains("bits")) {
      removeStartOfLine(line);
      addition += " --output-depth " + line;
      continue;
    }
    if (line.startsWith("Encoding settings")) {
      removeStartOfLine(line);
      ui.x265SettingsLineEdit->setText(line);
      tmp = this->buildCall();
      if (!addition.isEmpty()) {
        tmp = tmp.insert(4, addition);
      }
      ui.textBrowser->append("x265 call:\n" + tmp);
      return;
    }
  }
  QString message = tr("MediaInfo output didn't contain any 'Encoding setting'(2)!");
  QMessageBox::critical(this, tr("Error"), message);
  ui.textBrowser->append("<b>" + message + "</b>");
  ui.textBrowser->append(lines.join("<b/>"));
}

void Mis2x265::on_fileInputRadioButton_toggled(bool checked)
{
  if (checked && ui.manualRadioButton->isChecked()) {
    ui.manualRadioButton->setChecked(false);
  }
  ui.x265SettingsLineEdit->setEnabled(!checked);
  ui.convertPushButton->setEnabled(!checked);
  ui.selectAndImportPushButton->setEnabled(checked);
  if (m_mediainfo.isEmpty()) {
    ui.selectMediaInfoPushButton->setEnabled(checked);
  }
}

void Mis2x265::on_manualRadioButton_toggled(bool checked)
{
  if (checked && ui.fileInputRadioButton->isChecked()) {
    ui.fileInputRadioButton->setChecked(false);
  }
  ui.x265SettingsLineEdit->setEnabled(checked);
  ui.convertPushButton->setEnabled(checked);
  ui.selectAndImportPushButton->setEnabled(!checked);
  if (m_mediainfo.isEmpty()) {
    ui.selectMediaInfoPushButton->setEnabled(!checked);
  }
}

QString Mis2x265::buildCall()
{
  ui.textBrowser->clear();
  QStringList toConvert = ui.x265SettingsLineEdit->text().split("/");
  QStringList call;
  QString line, value, tmp;
  bool no, hasValue;
  int equalIndex;
  for (int i = 0, c = toConvert.count(); i < c; ++i) {
    line = toConvert.at(i).trimmed();
    no = line.startsWith("no-");
    if (no) {
      line = line.remove(0, 3);
    }
    equalIndex = line.indexOf("=");
    hasValue = equalIndex != -1;
    value = QString();
    if (hasValue) {
      value = line;
      value = value.remove(0, equalIndex+1);
      line = line.remove(equalIndex, line.size());
    }
    if (line == "rc") {
      if (value == "abr" || value == "cqp" || value == "crf") {
        continue;
      } else {
        ui.textBrowser->append("ignored: " + line + ", which probably indicates 2pass encoding. "+ value);
      }
    } else if (!m_simple.contains(line)) {
      tmp = m_special.value(line);
      if (!tmp.isEmpty()) {
        line = tmp;
      } else {
        ui.textBrowser->append("ignored: " + line);
        continue;
      }
    }
    if (no) {
      line = "no-" + line;
    }
    line = "--" + line;
    if (hasValue) {
      line += " "+value;
    }
    call << line;
  }
  call.insert(0, "x265");
  return call.join(" ");
}

void Mis2x265::on_convertPushButton_clicked()
{
  ui.textBrowser->append("x265 call:\n" + this->buildCall());
}

void Mis2x265::on_selectAndImportPushButton_clicked()
{
  if (m_mediainfo.isEmpty()) {
    m_mediainfo = ui.mediaInfoFileLabel->text();
    if (m_mediainfo.isEmpty()) {
      ui.textBrowser->clear();
      ui.textBrowser->append(
          tr("You first need to set MediaInfo before you can import settings from a file!"));
      return;
    }
  }
  m_inputFile = QFileDialog::getOpenFileName(this, tr("Select input file"), QString());

  if (m_inputFile.isEmpty()) {
    ui.textBrowser->append(tr("Nothing to do, input is empty,.."));
    return;
  }
  if (!QFile::exists(m_inputFile)) {
    QMessageBox::information(this, tr("Info"), tr("File %1 doesn't exist!").arg(m_inputFile));
    return;
  }
  m_mediainfoOutput.clear();
  delete m_process;
  m_process = new QProcess(this);
  QObject::connect(m_process, SIGNAL(finished( int, QProcess::ExitStatus)), this,
      SLOT(processFinished(int, QProcess::ExitStatus)));
  QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(collectOutput()));
  QObject::connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(collectOutput()));
  QString call = "\"" + m_mediainfo + "\" --full \"" + m_inputFile + "\"";
  ui.textBrowser->append(call);
  m_process->start(call);
}

void Mis2x265::on_selectMediaInfoPushButton_clicked()
{
  QString input = QFileDialog::getOpenFileName(this, tr("Select the MediaInfo binary"), QString());
  if (input.isEmpty()) {
    return;
  }
  ui.mediaInfoFileLabel->setText(input);
  m_mediainfo = input;
}

void Mis2x265::collectOutput()
{
  m_mediainfoOutput += QString::fromUtf8(m_process->readAllStandardOutput().data());
  m_mediainfoOutput += QString::fromUtf8(m_process->readAllStandardError().data());
}

void Mis2x265::processFinished(const int exitCode, const QProcess::ExitStatus exitStatus)
{
  ui.textBrowser->append(tr("MediaInfo finished,.."));
  if (exitCode < 0) {
    QMessageBox::critical(this, tr("MediaInfo crash"),
        tr("MediaInfo crashed with exitCode: %1 and status: %2 while analysing the input!").arg(
            exitCode).arg(exitStatus));
    return;
  }
  m_process->disconnect();
  m_process->kill();
  delete m_process;
  m_process = 0;
  this->analyseMediaInfoOutput();
}

