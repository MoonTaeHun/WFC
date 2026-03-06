import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window {
    width: 800
    height: 600
    visible: true
    title: "Wave Function Collapse 시뮬레이터"
    color: "#1e1e2e"

    // 맵을 그릴 GridView
    GridView {
        id: mapGrid
        anchors.centerIn: parent
        width: 20 * 30  // 가로 20칸 * 셀 너비 30
        height: 15 * 30 // 세로 15칸 * 셀 높이 30
        cellWidth: 30
        cellHeight: 30

        model: wfcModel // C++에서 등록한 모델

        delegate: Rectangle {
            width: 28
            height: 28
            radius: 4

            // 붕괴(결정)되지 않았으면 회색, 결정되었으면 타일 색상 부여
            color: {
                if (!model.isCollapsed) return "#313244"; // 결정 전

                if (model.tileType === 0) return "#89b4fa"; // 바다 (파란색)
                if (model.tileType === 1) return "#f9e2af"; // 해변 (모래색)
                if (model.tileType === 2) return "#a6e3a1"; // 육지 (초록색)

                return "red"; // 에러 방지용
            }

            // 결정 전일 때, 가능한 후보(엔트로피) 개수를 텍스트로 표시
            Text {
                anchors.centerIn: parent
                text: model.isCollapsed ? "" : model.entropy
                color: "#cdd6f4"
                font.pixelSize: 10
                visible: !model.isCollapsed
            }

            // 타일이 결정될 때 스르륵 나타나는 애니메이션 효과
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
        }
    }

    Button {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 20
        text: "맵 다시 생성하기 (Reset)"
        onClicked: wfcModel.resetMap()
    }
}
