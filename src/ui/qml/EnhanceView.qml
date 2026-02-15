import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: "transparent"
    property var viewModel
    property var theme

    function phaseTone() {
        if (!viewModel || !theme) {
            return "#98ADC9"
        }
        if (viewModel.phase === "success") {
            return theme.success
        }
        if (viewModel.phase === "error") {
            return theme.danger
        }
        if (viewModel.phase === "running") {
            return theme.warn
        }
        if (viewModel.phase === "ready") {
            return theme.accent
        }
        return theme.inkMuted
    }

    function phaseLabel() {
        if (!viewModel) {
            return "Not Ready"
        }
        if (viewModel.phase === "success") {
            return "Result Ready"
        }
        if (viewModel.phase === "error") {
            return "Action Needed"
        }
        if (viewModel.phase === "running") {
            return "Enhancing"
        }
        if (viewModel.phase === "ready") {
            return "Ready"
        }
        return "Drop Input"
    }

    function shortPath(pathValue) {
        if (!pathValue || pathValue.length < 64) {
            return pathValue
        }
        return pathValue.substring(0, 28) + "..." + pathValue.substring(pathValue.length - 30)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 96
            radius: 18
            color: theme.panel
            border.width: 1
            border.color: theme.stroke

            RowLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        text: "Lumos Enhance Studio"
                        color: theme.inkPrimary
                        font.pixelSize: 30
                        font.family: theme.titleFont
                    }

                    Label {
                        text: "Import -> Enhance -> Compare -> Save output path"
                        color: theme.inkMuted
                        font.pixelSize: 14
                        font.family: theme.bodyFont
                    }
                }

                Rectangle {
                    Layout.preferredWidth: 190
                    Layout.preferredHeight: 42
                    radius: 21
                    color: "#0E1A30"
                    border.color: phaseTone()
                    border.width: 1

                    Label {
                        anchors.centerIn: parent
                        text: phaseLabel()
                        color: phaseTone()
                        font.pixelSize: 14
                        font.bold: true
                        font.family: theme.bodyFont
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            Rectangle {
                id: stagePanel
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 18
                color: theme.panel
                border.color: theme.stroke
                border.width: 1

                Rectangle {
                    id: dragBorder
                    anchors.fill: parent
                    anchors.margins: 10
                    radius: 14
                    color: "transparent"
                    border.color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.5)
                    border.width: 2
                    opacity: 0.0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 140
                        }
                    }
                }

                DropArea {
                    anchors.fill: parent
                    onEntered: dragBorder.opacity = 1.0
                    onExited: dragBorder.opacity = 0.0
                    onDropped: function (drop) {
                        dragBorder.opacity = 0.0
                        if (!viewModel || !drop.hasUrls || drop.urls.length === 0) {
                            return
                        }

                        var localPath = viewModel.localPathFromUrl(drop.urls[0].toString())
                        pathInput.text = localPath
                        viewModel.setInputPath(localPath)
                    }
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        TextField {
                            id: pathInput
                            Layout.fillWidth: true
                            placeholderText: "Drop a local .ppm file or paste its absolute path"
                            text: viewModel ? viewModel.inputPath : ""
                            color: theme.inkPrimary
                            placeholderTextColor: theme.inkMuted
                            font.family: theme.bodyFont
                            background: Rectangle {
                                radius: 10
                                color: theme.panelRaised
                                border.color: theme.stroke
                                border.width: 1
                            }
                            onAccepted: {
                                if (viewModel) {
                                    viewModel.setInputPath(text)
                                }
                            }
                        }

                        Button {
                            text: "Load"
                            onClicked: {
                                if (viewModel) {
                                    viewModel.setInputPath(pathInput.text)
                                }
                            }
                        }
                    }

                    Rectangle {
                        id: compareFrame
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 14
                        color: "#0A1324"
                        border.width: 1
                        border.color: theme.stroke

                        Item {
                            anchors.fill: parent
                            anchors.margins: 14
                            visible: viewModel && viewModel.inputPath !== ""

                            Rectangle {
                                anchors.fill: parent
                                radius: 12
                                color: "#0C182D"
                            }

                            Image {
                                id: beforeImage
                                anchors.fill: parent
                                source: viewModel ? viewModel.inputFileUrl : ""
                                fillMode: Image.PreserveAspectFit
                                asynchronous: true
                                cache: false
                            }

                            Item {
                                anchors.fill: parent
                                width: parent.width * compareSlider.value
                                clip: true

                                Image {
                                    anchors.fill: parent
                                    source: (viewModel && viewModel.hasResult) ? viewModel.resultFileUrl : viewModel.inputFileUrl
                                    fillMode: Image.PreserveAspectFit
                                    asynchronous: true
                                    cache: false
                                }
                            }

                            Rectangle {
                                width: 2
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                x: parent.width * compareSlider.value - 1
                                color: theme.accent
                            }
                        }

                        Column {
                            anchors.centerIn: parent
                            spacing: 8
                            visible: !viewModel || viewModel.inputPath === ""

                            Label {
                                text: "Drop an image to start"
                                color: theme.inkPrimary
                                font.pixelSize: 28
                                font.family: theme.titleFont
                            }

                            Label {
                                text: "Current pipeline accepts .ppm while we harden broader format support."
                                color: theme.inkMuted
                                font.pixelSize: 14
                                font.family: theme.bodyFont
                            }
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: "#0B1831"
                            opacity: viewModel && viewModel.busy ? 0.76 : 0.0
                            visible: opacity > 0.01
                            Behavior on opacity {
                                NumberAnimation {
                                    duration: 150
                                }
                            }

                            Column {
                                anchors.centerIn: parent
                                spacing: 10

                                BusyIndicator {
                                    running: viewModel && viewModel.busy
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }

                                Label {
                                    text: "Enhancing..."
                                    color: theme.inkPrimary
                                    font.pixelSize: 16
                                    font.bold: true
                                    font.family: theme.bodyFont
                                }
                            }
                        }
                    }

                    Slider {
                        id: compareSlider
                        Layout.fillWidth: true
                        from: 0.05
                        to: 0.95
                        value: 0.5
                        enabled: viewModel && viewModel.inputPath !== ""
                    }

                    Label {
                        Layout.fillWidth: true
                        text: viewModel ? viewModel.statusText : ""
                        color: phaseTone()
                        wrapMode: Text.WordWrap
                        font.pixelSize: 13
                        font.family: theme.bodyFont
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: 350
                Layout.fillHeight: true
                radius: 18
                color: theme.panel
                border.width: 1
                border.color: theme.stroke

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 14

                    Label {
                        text: "Enhancement Controls"
                        color: theme.inkPrimary
                        font.pixelSize: 22
                        font.family: theme.titleFont
                    }

                    Label {
                        text: "Input"
                        color: theme.inkMuted
                        font.pixelSize: 12
                        font.family: theme.bodyFont
                    }

                    Label {
                        Layout.fillWidth: true
                        text: viewModel && viewModel.inputPath !== "" ? shortPath(viewModel.inputPath) : "No file selected"
                        color: theme.inkPrimary
                        wrapMode: Text.WordWrap
                        font.pixelSize: 13
                        font.family: theme.bodyFont
                    }

                    Label {
                        text: "Scale Factor"
                        color: theme.inkMuted
                        font.pixelSize: 12
                        font.family: theme.bodyFont
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Repeater {
                            model: [2, 4, 8]
                            delegate: Button {
                                readonly property bool active: viewModel && viewModel.scaleFactor === modelData
                                Layout.fillWidth: true
                                text: modelData + "x"
                                highlighted: active
                                onClicked: {
                                    if (viewModel) {
                                        viewModel.setScaleFactor(modelData)
                                    }
                                }
                            }
                        }
                    }

                    Switch {
                        text: "Apply denoise pass"
                        checked: viewModel ? viewModel.denoiseEnabled : false
                        onToggled: {
                            if (viewModel) {
                                viewModel.setDenoiseEnabled(checked)
                            }
                        }
                    }

                    Button {
                        Layout.fillWidth: true
                        text: viewModel && viewModel.busy ? "Enhancing..." : "Enhance"
                        enabled: viewModel && viewModel.canEnhance
                        onClicked: {
                            if (viewModel) {
                                viewModel.startEnhancement()
                            }
                        }
                    }

                    Button {
                        Layout.fillWidth: true
                        text: "Reset Session"
                        enabled: viewModel && !viewModel.busy
                        onClicked: {
                            if (viewModel) {
                                viewModel.resetSession()
                                pathInput.text = ""
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 12
                        color: theme.panelRaised
                        border.width: 1
                        border.color: theme.stroke

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10

                            Label {
                                text: "Output Path"
                                color: theme.inkMuted
                                font.pixelSize: 12
                                font.family: theme.bodyFont
                            }

                            Label {
                                Layout.fillWidth: true
                                text: viewModel ? viewModel.outputPath : ""
                                color: theme.inkPrimary
                                wrapMode: Text.WordWrap
                                font.pixelSize: 12
                                font.family: theme.bodyFont
                            }

                            Label {
                                Layout.fillWidth: true
                                visible: viewModel && viewModel.hasResult
                                text: viewModel ? viewModel.resultSummary : ""
                                color: theme.success
                                wrapMode: Text.WordWrap
                                font.pixelSize: 13
                                font.bold: true
                                font.family: theme.bodyFont
                            }
                        }
                    }
                }
            }
        }
    }
}

