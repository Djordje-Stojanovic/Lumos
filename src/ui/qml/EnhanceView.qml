import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: "#111827"
    property string selectedPath: ""

    RowLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 10
            color: "#1f2937"
            border.color: "#374151"
            border.width: 1

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 8

                Label {
                    text: "Drop Image To Start"
                    color: "#f9fafb"
                    font.pixelSize: 26
                }

                Label {
                    text: "Vertical-slice preview shell wired for future backend binding."
                    color: "#9ca3af"
                    font.pixelSize: 14
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: 320
            Layout.fillHeight: true
            radius: 10
            color: "#0f172a"
            border.color: "#374151"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                Label {
                    text: "Enhance"
                    color: "#f9fafb"
                    font.pixelSize: 22
                }

                Label {
                    text: "Scale Factor"
                    color: "#d1d5db"
                }

                ComboBox {
                    model: ["2x", "4x", "8x"]
                    currentIndex: 0
                }

                CheckBox {
                    text: "Denoise"
                    checked: false
                }

                Button {
                    text: "Enhance"
                    Layout.fillWidth: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 8
                    color: "#111827"
                    border.color: "#374151"
                    border.width: 1

                    Column {
                        anchors.centerIn: parent
                        spacing: 6

                        Label {
                            text: "Before / After"
                            color: "#f3f4f6"
                        }
                        Slider {
                            width: 220
                            from: 0
                            to: 1
                            value: 0.5
                        }
                    }
                }
            }
        }
    }
}

