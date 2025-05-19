#include "simulation_model.h"
#include <QAbstractTableModel>
#include <QVariant>
#include <vector>

class SimulationModel : public QAbstractTableModel {
    Q_OBJECT

public:
    SimulationModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}

    void setSimulationResults(const std::vector<std::vector<double>>& results) {
        beginResetModel();
        simulationResults = results;
        endResetModel();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return simulationResults.size();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return simulationResults.empty() ? 0 : simulationResults[0].size();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (role == Qt::DisplayRole && index.isValid()) {
            return simulationResults[index.row()][index.column()];
        }
        return QVariant();
    }

private:
    std::vector<std::vector<double>> simulationResults;
};
#include "simulation_model.moc"