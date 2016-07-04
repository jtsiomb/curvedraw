#include <stdlib.h>
#include <QtWidgets>
#include "main_qt.h"
#include "app.h"

struct Actions {
	QAction *clear;
	QAction *open;
	QAction *save;
	QAction *quit;
	QAction *snap_grid;
	QAction *snap_pt;
};

static void snap_changed(SnapMode s, void *cls);

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
	act = new Actions;

	act->clear = new QAction(QIcon(":icon_new"), "&New", this);
	act->clear->setStatusTip("Start new curve set");
	act->clear->setShortcut(QKeySequence::New);
	QObject::connect(act->clear, SIGNAL(triggered()), this, SLOT(clear_curves()));

	act->open = new QAction(style->standardIcon(QStyle::SP_DialogOpenButton), "&Open...", this);
	act->open->setStatusTip("Open a curve file");
	act->open->setShortcut(QKeySequence::Open);
	QObject::connect(act->open, SIGNAL(triggered()), this, SLOT(open_curvefile()));

	act->save = new QAction(style->standardIcon(QStyle::SP_DialogSaveButton), "&Save...", this);
	act->save->setStatusTip("Save to a curve file");
	act->save->setShortcut(QKeySequence::Save);
	QObject::connect(act->save, SIGNAL(triggered()), this, SLOT(save_curvefile()));

	act->quit = new QAction(style->standardIcon(QStyle::SP_DialogCloseButton), "&Quit", this);
	act->quit->setShortcut(QKeySequence(tr("Ctrl+Q", "File|Quit")));
	QObject::connect(act->quit, SIGNAL(triggered()), this, SLOT(close()));

	act->snap_grid = new QAction(QIcon(":icon_snap_grid"), "Snap to grid", this);
	act->snap_grid->setStatusTip("Enable grid snapping");
	act->snap_grid->setCheckable(true);
	QObject::connect(act->snap_grid, SIGNAL(triggered()), this, SLOT(snap_grid()));

	act->snap_pt = new QAction(QIcon(":icon_snap_pt"), "Snap to points", this);
	act->snap_pt->setStatusTip("Enable point snapping");
	act->snap_pt->setCheckable(true);
	QObject::connect(act->snap_pt, SIGNAL(triggered()), this, SLOT(snap_pt()));
	app_tool_snap_callback(snap_changed, act);

	// menus
	QMenu *mfile = menuBar()->addMenu("&File");
	mfile->addAction(act->clear);
	mfile->addAction(act->open);
	mfile->addAction(act->save);
	mfile->addSeparator();
	mfile->addAction(act->quit);

	QMenu *medit = menuBar()->addMenu("&Edit");
	medit->addAction(act->snap_grid);
	medit->addAction(act->snap_pt);

	// toolbars
	QToolBar *tbar = addToolBar("Toolbar");
	tbar->addAction(act->clear);
	tbar->addAction(act->open);
	tbar->addAction(act->save);
	tbar->addSeparator();
	tbar->addAction(act->snap_grid);
	tbar->addAction(act->snap_pt);

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

void MainWindow::snap_grid()
{
	if(act->snap_grid->isChecked()) {
		app_tool_snap(SNAP_GRID);
		if(act->snap_pt->isChecked()) {
			act->snap_pt->setChecked(false);
		}
	} else {
		app_tool_snap(SNAP_NONE);
	}
}

void MainWindow::snap_pt()
{
	if(act->snap_pt->isChecked()) {
		app_tool_snap(SNAP_POINT);
		if(act->snap_grid->isChecked()) {
			act->snap_grid->setChecked(false);
		}
	} else {
		app_tool_snap(SNAP_NONE);
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

static void snap_changed(SnapMode s, void *cls)
{
	Actions *act = (Actions*)cls;
	act->snap_grid->setChecked(s == SNAP_GRID);
	act->snap_pt->setChecked(s == SNAP_POINT);
}
