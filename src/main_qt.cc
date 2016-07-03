#include <stdlib.h>
#include <QtWidgets>
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
	QSurfaceFormat glfmt;
	glfmt.setSamples(8);
	glview->setFormat(glfmt);
	setCentralWidget(glview);

	QStyle *style = qApp->style();

	// actions
	QAction *act_new = new QAction(QIcon(":icon_new"), "&New", this);
	act_new->setStatusTip("Start new curve set");
	act_new->setShortcut(QKeySequence::New);
	QObject::connect(act_new, SIGNAL(triggered()), this, SLOT(clear_curves()));

	QAction *act_open = new QAction(style->standardIcon(QStyle::SP_DialogOpenButton), "&Open...", this);
	act_open->setStatusTip("Open a curve file");
	act_open->setShortcut(QKeySequence::Open);
	QObject::connect(act_open, SIGNAL(triggered()), this, SLOT(open_curvefile()));

	QAction *act_save = new QAction(style->standardIcon(QStyle::SP_DialogSaveButton), "&Save...", this);
	act_save->setStatusTip("Save to a curve file");
	act_save->setShortcut(QKeySequence::Save);
	QObject::connect(act_save, SIGNAL(triggered()), this, SLOT(save_curvefile()));

	QAction *act_quit = new QAction(style->standardIcon(QStyle::SP_DialogCloseButton), "&Quit", this);
	act_quit->setShortcut(QKeySequence(tr("Ctrl+Q", "File|Quit")));
	QObject::connect(act_quit, SIGNAL(triggered()), this, SLOT(close()));

	// menus
	QMenu *mfile = menuBar()->addMenu("&File");
	mfile->addAction(act_new);
	mfile->addAction(act_open);
	mfile->addAction(act_save);
	mfile->addSeparator();
	mfile->addAction(act_quit);

	// toolbars
	QToolBar *tfile = addToolBar("&File");
	tfile->addAction(act_new);
	tfile->addAction(act_open);
	tfile->addAction(act_save);
	tfile->addSeparator();

	statusBar();
	show();
}

void MainWindow::clear_curves()
{
	app_tool_clear();
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

void MainWindow::save_curvefile()
{
	QString fname = QFileDialog::getSaveFileName(this, "Save curve file", QString(), "Curves (*.curves)");
	if(!fname.isNull()) {
		if(!fname.endsWith(".curves", Qt::CaseInsensitive)) {
			fname += ".curves";
		}
		if(!app_tool_save(qPrintable(fname))) {
			QMessageBox::critical(this, "Failed to save file!", "Failed to save file: " + fname);
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

static int key_code(QKeyEvent *ev)
{
	int qkey = ev->key();
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

	QString text = ev->text();
	if(!text.isEmpty() && text[0].isPrint()) {
		return text[0].unicode();
	}
	return -1;
}

void GLView::keyPressEvent(QKeyEvent *ev)
{
	int key = key_code(ev);
	if(key >= 0) {
		app_keyboard(key, true);
	} else {
		QWidget::keyPressEvent(ev);
	}
}

void GLView::keyReleaseEvent(QKeyEvent *ev)
{
	int key = key_code(ev);
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
