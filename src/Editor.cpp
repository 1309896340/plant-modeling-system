#include "Editor.h"

constexpr int TOOLBAR_WIDTH = 140;

Toolbar::Toolbar(QWidget *parent) : QWidget(parent) {}

Editor::Editor(std::string title) {
  this->setWindowTitle(QString::fromStdString(title));
  // 默认大小配置
  this->setMinimumWidth(1000);
  this->setMinimumHeight(800);
  // 窗口布局
  QWidget *centralWidget = new QWidget(this);
  this->layout = new QVBoxLayout(centralWidget);
  this->splitter = new QSplitter(this);
  this->toolbar = new Toolbar(this->splitter);
  this->canvas = new Canvas(this->splitter);
  this->statusBar = new QStatusBar(this);

  this->setAttribute(Qt::WA_StyledBackground, true);
  this->setWindowIcon(QIcon(QString("Debug/assets/favicon/favicon1.png")));
  this->setCentralWidget(centralWidget);
  centralWidget->setLayout(this->layout);
  this->layout->addWidget(this->splitter);
  this->layout->addWidget(this->statusBar);
  this->splitter->addWidget(this->toolbar);
  this->splitter->addWidget(this->canvas);

  this->toolbar->setFixedWidth(TOOLBAR_WIDTH);
  this->statusBar->setFixedHeight(20);

  this->splitter->show();
  this->show();
}

void Editor::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {
  case Qt::Key_A:

    break;
  case Qt::Key_B:

    break;
  }
}
