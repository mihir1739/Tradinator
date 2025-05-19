#ifndef SIMULATION_MODEL_H
#define SIMULATION_MODEL_H

#include <QAbstractTableModel>
#include <vector>

class SimulationModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit SimulationModel(QObject *parent = nullptr);
    
    // Override row and column count
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Override data retrieval
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Method to update simulation results
    void updateResults(const std::vector<double> &results);

private:
    std::vector<double> simulationResults; // Store simulation results
};

#endif // SIMULATION_MODEL_H