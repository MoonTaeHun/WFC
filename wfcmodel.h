#pragma once
#include <QAbstractListModel>
#include <QTimer>
#include <vector>
#include <set>

// 타일 종류: 0=바다, 1=해변, 2=육지
enum TileType { Sea = 0, Coast = 1, Land = 2, Uncollapsed = -1 };

struct Cell {
    bool collapsed = false;
    int finalTile = Uncollapsed;
    std::set<int> possibleTiles = { Sea, Coast, Land }; // 초기엔 모든 가능성이 열려있음(엔트로피 최대)
};

class WfcModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum WfcRoles {
        CollapsedRole = Qt::UserRole + 1,
        TileTypeRole,
        EntropyRole
    };

    explicit WfcModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void resetMap();

private slots:
    void stepWfc(); // 1스텝(붕괴->전파) 진행

private:
    int getLowestEntropyCell();
    void collapseCell(int index);
    void propagate(int startIndex);
    bool checkRules(int tile1, int tile2);

    int m_width = 20;
    int m_height = 15;
    std::vector<Cell> m_grid;
    QTimer m_timer;
};
