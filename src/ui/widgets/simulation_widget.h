#ifndef SIMULATIONWIDGET_H
#define SIMULATIONWIDGET_H

#include <QWidget>

class SimulationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SimulationWidget(QWidget *parent = nullptr);
    ~SimulationWidget();

    void setInputParameters(double parameter1, double parameter2);
    void startSimulation();
    void displayResults();

private:
    double m_parameter1;
    double m_parameter2;
    // Add other necessary member variables for simulation results

signals:
    void simulationCompleted(double result);

private slots:
    void onSimulationCompleted(double result);
};

#endif // SIMULATIONWIDGET_H