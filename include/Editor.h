#pragma once

#include <QKeyEvent>
#include <QMainWindow>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>

#include <cstdint>
#include <memory>
#include <string>

#include "Canvas.h"

class Toolbar : public QWidget {
public:
  Toolbar(QWidget *parent = nullptr);
};

class Editor : public QMainWindow {
  Q_OBJECT
public:
  enum ToolMode { CURSOR, PEN };

private:
  ToolMode mode;
  QVBoxLayout *layout{nullptr};
  QSplitter *splitter{nullptr};
  Canvas *canvas{nullptr};
  Toolbar *toolbar{nullptr};
  QStatusBar *statusBar{nullptr};
private slots:
public:
  Editor(std::string title);

  void keyPressEvent(QKeyEvent *event) override;
};
