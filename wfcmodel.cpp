#include "wfcmodel.h"
#include <QRandomGenerator>
#include <queue>

WfcModel::WfcModel(QObject *parent) : QAbstractListModel(parent) {
    resetMap();
    connect(&m_timer, &QTimer::timeout, this, &WfcModel::stepWfc);
    m_timer.start(50); // 0.05초마다 한 칸씩 붕괴시킴 (시각화 목적)
}

void WfcModel::resetMap() {
    beginResetModel();
    m_grid.assign(m_width * m_height, Cell());
    endResetModel();
    if (!m_timer.isActive()) {
        m_timer.start(50);
    }
}

// 스도쿠 규칙처럼, 두 타일이 인접할 수 있는지 정의합니다.
// (바다-육지는 바로 붙을 수 없고, 중간에 해변이 있어야 함)
bool WfcModel::checkRules(int tile1, int tile2) {
    if (tile1 == Sea) return tile2 == Sea || tile2 == Coast;
    if (tile1 == Coast) return tile2 == Sea || tile2 == Coast || tile2 == Land;
    if (tile1 == Land) return tile2 == Coast || tile2 == Land;
    return true;
}

void WfcModel::stepWfc() {
    int targetIndex = getLowestEntropyCell();

    // 더 이상 붕괴할 칸이 없으면 완성!
    if (targetIndex == -1) {
        m_timer.stop();
        return;
    }

    collapseCell(targetIndex);
    propagate(targetIndex);

    emit dataChanged(index(0), index(m_grid.size() - 1));
}

int WfcModel::getLowestEntropyCell() {
    int minEntropy = 999;
    std::vector<int> candidates;

    for (int i = 0; i < m_grid.size(); ++i) {
        if (m_grid[i].collapsed) continue;

        int entropy = m_grid[i].possibleTiles.size();
        if (entropy == 0) continue; // 모순 발생 (Contradiction)

        if (entropy < minEntropy) {
            minEntropy = entropy;
            candidates.clear();
            candidates.push_back(i);
        } else if (entropy == minEntropy) {
            candidates.push_back(i); // 동일한 엔트로피면 후보에 추가
        }
    }

    if (candidates.empty()) return -1;
    // 후보군 중 랜덤하게 하나 선택
    return candidates[QRandomGenerator::global()->bounded(candidates.size())];
}

void WfcModel::collapseCell(int index) {
    Cell& cell = m_grid[index];
    cell.collapsed = true;

    // 남은 가능성 중 랜덤으로 하나를 최종 타일로 확정!
    auto it = cell.possibleTiles.begin();
    std::advance(it, QRandomGenerator::global()->bounded(cell.possibleTiles.size()));
    cell.finalTile = *it;

    cell.possibleTiles.clear();
    cell.possibleTiles.insert(cell.finalTile);
}

void WfcModel::propagate(int startIndex) {
    // 큐를 이용해 주변 칸으로 제약 조건을 연쇄적으로 전파합니다.
    std::queue<int> q;
    q.push(startIndex);

    while (!q.empty()) {
        int curr = q.front();
        q.pop();

        int x = curr % m_width;
        int y = curr / m_width;

        // 상하좌우 인덱스
        int neighbors[4] = {
            (y > 0) ? curr - m_width : -1,             // 상
            (y < m_height - 1) ? curr + m_width : -1,  // 하
            (x > 0) ? curr - 1 : -1,                   // 좌
            (x < m_width - 1) ? curr + 1 : -1          // 우
        };

        for (int ni : neighbors) {
            if (ni == -1 || m_grid[ni].collapsed) continue;

            bool changed = false;
            auto it = m_grid[ni].possibleTiles.begin();

            // 인접한 칸의 가능성들을 체크해서, 현재 칸과 어울리지 않으면 제거
            while (it != m_grid[ni].possibleTiles.end()) {
                bool canFit = false;
                for (int myTile : m_grid[curr].possibleTiles) {
                    if (checkRules(myTile, *it)) {
                        canFit = true;
                        break;
                    }
                }

                if (!canFit) {
                    it = m_grid[ni].possibleTiles.erase(it);
                    changed = true;
                } else {
                    ++it;
                }
            }

            // 가능성이 줄어들었다면, 이 칸의 변경사항도 주변으로 전파해야 함
            if (changed) {
                q.push(ni);
            }
        }
    }
}

// QAbstractListModel 필수 함수 구현부
int WfcModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_grid.size();
}

QVariant WfcModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_grid.size()) return QVariant();

    const Cell &cell = m_grid[index.row()];

    if (role == CollapsedRole) {
        return cell.collapsed;
    } else if (role == TileTypeRole) {
        return cell.finalTile;
    } else if (role == EntropyRole) {
        return (int)cell.possibleTiles.size();
    }

    return QVariant();
}

QHash<int, QByteArray> WfcModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[CollapsedRole] = "isCollapsed";
    roles[TileTypeRole] = "tileType";
    roles[EntropyRole] = "entropy";
    return roles;
}
