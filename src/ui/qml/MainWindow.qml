import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    id: root
    width: 1360
    height: 860
    minimumWidth: 980
    minimumHeight: 680
    visible: true
    title: "Lumos AI Studio"
    color: "#080F1E"

    QtObject {
        id: theme
        property color shellTop: "#12233E"
        property color shellBottom: "#060B18"
        property color panel: "#101D33"
        property color panelRaised: "#172A45"
        property color stroke: "#2B3F61"
        property color inkPrimary: "#ECF3FF"
        property color inkMuted: "#98ADC9"
        property color accent: "#2CC5B0"
        property color warn: "#F3B55D"
        property color danger: "#F07878"
        property color success: "#6EE7A8"
        property string titleFont: "Bahnschrift"
        property string bodyFont: "Segoe UI"
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: theme.shellTop
            }
            GradientStop {
                position: 0.55
                color: "#0A1429"
            }
            GradientStop {
                position: 1.0
                color: theme.shellBottom
            }
        }
    }

    Rectangle {
        width: 500
        height: 500
        radius: 250
        x: -160
        y: -180
        color: "#1C3152"
        opacity: 0.25
    }

    Rectangle {
        width: 380
        height: 380
        radius: 190
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: -110
        anchors.topMargin: -80
        color: "#204D54"
        opacity: 0.18
    }

    EnhanceView {
        anchors.fill: parent
        anchors.margins: 20
        viewModel: enhanceViewModel
        theme: theme
    }
}

