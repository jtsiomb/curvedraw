#ifndef APP_H_
#define APP_H_

bool app_init(int argc, char **argv);
void app_cleanup();

void app_draw();
void app_reshape(int x, int y);
void app_keyboard(int key, bool pressed);
void app_mouse_button(int bn, bool pressed, int x, int y);
void app_mouse_motion(int x, int y);

void post_redisplay();	// in main.cc

#endif	// APP_H_
