#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QMainWindow>

class Application : public QApplication {
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    void initialize();
    void run();

private:
    QMainWindow *mainWindow;
    void setupMainWindow();
    void loadSettings();
    void saveSettings();
};

#endif // APPLICATION_H