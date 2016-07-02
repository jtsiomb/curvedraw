#ifndef MAIN_QT_H_
#define MAIN_QT_H_

#include <QMainWindow>
#include <QOpenGLWidget>

class GLView;

class MainWindow : public QMainWindow {
private:
	Q_OBJECT

private slots:
	void open_curvefile();

public:
	GLView *glview;

	MainWindow();
};

class GLView : public QOpenGLWidget {
	Q_OBJECT
protected:
	void initializeGL();
	void resizeGL(int x, int y);
	void paintGL();

	void mousePressEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);

public:
	GLView();
};

#endif	// MAIN_QT_H_
