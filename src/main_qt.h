#ifndef MAIN_QT_H_
#define MAIN_QT_H_

#include <QMainWindow>
#include <QOpenGLWidget>

class GLView;
struct Actions;

class MainWindow : public QMainWindow {
private:
	Q_OBJECT

private slots:
	void clear_curves();
	void open_curvefile();
	void save_curvefile();
	void snap_grid();
	void snap_pt();
	void curve_type(int type);

public:
	Actions *act;
	GLView *glview;

	MainWindow();
};

class GLView : public QOpenGLWidget {
	Q_OBJECT
protected:
	void initializeGL();
	void resizeGL(int x, int y);
	void paintGL();

	void keyPressEvent(QKeyEvent *ev);
	void keyReleaseEvent(QKeyEvent *ev);
	void mousePressEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);
	void wheelEvent(QWheelEvent *ev);

public:
	GLView();

	QSize minimumSizeHint() const;
	QSize sizeHint() const;
};

#endif	// MAIN_QT_H_
