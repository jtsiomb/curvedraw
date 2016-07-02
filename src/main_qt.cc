#include <stdlib.h>
#include <QApplication>
#include <QtWidgets>
/*#include <QMouseEvent>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStyle>
#include <QFileDialog>
*/
#include "main_qt.h"
#include "app.h"

static MainWindow *win;

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	win = new MainWindow;
	win->show();
	int res = app.exec();
	app_cleanup();
	delete win;
	return res;
}

MainWindow::MainWindow()
{
	setWindowTitle("Curvedraw");

	glview = new GLView;
	setCentralWidget(glview);

	// actions
	QAction *act_open = new QAction(qApp->style()->standardIcon(QStyle::SP_DialogOpenButton), "&Open...", this);
	act_open->setStatusTip("Open a curve file");
	act_open->setShortcut(QKeySequence::Open);
	QObject::connect(act_open, SIGNAL(triggered()), this, SLOT(open_curvefile()));

	QAction *act_quit = new QAction(qApp->style()->standardIcon(QStyle::SP_DialogCloseButton), "&Quit", this);
	act_quit->setShortcut(QKeySequence(tr("Ctrl+Q", "File|Quit")));
	QObject::connect(act_quit, SIGNAL(triggered()), this, SLOT(close()));

	// menus
	QMenu *mfile = menuBar()->addMenu("&File");
	mfile->addAction(act_open);
	mfile->addAction(act_quit);

	// toolbars
	QToolBar *tfile = addToolBar("&File");
	tfile->addAction(act_open);

	statusBar();
	show();
}

void MainWindow::open_curvefile()
{
	QString fname = QFileDialog::getOpenFileName(this, "Open curve file", QString(), "Curves (*.curves)");
	if(!fname.isNull()) {
		if(app_tool_load(qPrintable(fname))) {
			glview->update();
		} else {
			QMessageBox::critical(this, "Failed to open file!", "Failed to load curves from: " + fname);
		}
	}
}

// ---- GLView implementation ----

GLView::GLView()
{
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
}

QSize GLView::minimumSizeHint() const
{
	return QSize(80, 60);
}

QSize GLView::sizeHint() const
{
	return QSize(1280, 800);
}

void GLView::initializeGL()
{
	if(!app_init(0, 0)) {
		exit(0);
	}
}

void GLView::resizeGL(int x, int y)
{
	app_reshape(x, y);
}

void GLView::paintGL()
{
	app_draw();
}

static int key_code(int qkey)
{
	switch(qkey) {
	case Qt::Key_Escape:
		return 27;
	case Qt::Key_Tab:
		return '\t';
	case Qt::Key_Return:
	case Qt::Key_Enter:
		return '\n';
	case Qt::Key_Delete:
		return 127;
	case Qt::Key_Backspace:
		return '\b';
	}

	if(qkey > 0 && qkey < 256) {
		return qkey;
	}
	return -1;
}

void GLView::keyPressEvent(QKeyEvent *ev)
{
	int key = key_code(ev->key());
	if(key >= 0) {
		app_keyboard(key, true);
	} else {
		QWidget::keyPressEvent(ev);
	}
}

void GLView::keyReleaseEvent(QKeyEvent *ev)
{
	int key = key_code(ev->key());
	if(key >= 0) {
		app_keyboard(key, false);
	} else {
		QWidget::keyPressEvent(ev);
	}
}

static int button_number(Qt::MouseButton bn)
{
	switch(bn) {
	case Qt::LeftButton:
		return 0;
	case Qt::MidButton:
		return 1;
	case Qt::RightButton:
		return 2;
	default:
		break;
	}
	return -1;
}

void GLView::mousePressEvent(QMouseEvent *ev)
{
	int bn = button_number(ev->button());
	if(bn >= 0) {
		app_mouse_button(bn, true, ev->x(), ev->y());
	}
}

void GLView::mouseReleaseEvent(QMouseEvent *ev)
{
	int bn = button_number(ev->button());
	if(bn >= 0) {
		app_mouse_button(bn, false, ev->x(), ev->y());
	}
}

void GLView::mouseMoveEvent(QMouseEvent *ev)
{
	app_mouse_motion(ev->x(), ev->y());
}

void post_redisplay()
{
	if(win && win->glview) {
		win->glview->update();
	}
}
