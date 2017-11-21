import QtQuick 2.6
import QtQuick.Controls 2.0

Item {
    id: rootComboBox
    width: 100
    height: 30
    property color color: "#e0e0e0"
//    property var color: "#e0e0e0"
//    property var model
    property font font
    property variant model
    property int numberStartV: 0
    property int currentIndex: 0
    property string currentText: modelType == 0 ? rootComboBox.model[currentIndex] : (currentIndex  + numberStartV)
    property string displayText: modelType == 0 ? rootComboBox.model[currentIndex] : numberStartV

    property int modelType: isNaN(model) ? 0 : 1
    property int modelLen: modelType == 0 ? model.length: model
    property int numOfDisplay: modelType == 0 ? 5 : 8
    property int popLen: (height - 6) * (modelLen > numOfDisplay ? numOfDisplay: modelLen) + 18 * 2
    signal selected(var index)

    Rectangle {
        id: selectorRect
        width: parent.width
        height: parent.height
        color: rootComboBox.color
        border.width: 2
        border.color: Qt.darker(rootComboBox.color, 1.04)

        Text {
            id: selectedText
            height: parent.height
            width: parent.width - directionImage.width - 10
            text: rootComboBox.displayText
            //font.pointSize: rootComboBox.font.pointSize
            //font.bold: rootComboBox.font.bold
            font: rootComboBox.font
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            verticalAlignment: Text.AlignVCenter
        }

        Image {
            id: directionImage
            height: parent.height * 0.5
            width: height
            anchors.right: parent.right
            anchors.rightMargin: 2
            anchors.verticalCenter: parent.verticalCenter
            source: "res/arrowDown01.png"
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                dropDown.open()
            }
        }
    }

    Popup {
        id: dropDown
        width: parent.width
        height: rootComboBox.popLen
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        //parent: choseRect
        //x: rootComboBox.width
        y: modelType == 0 ? rootComboBox.height : -rootComboBox.popLen/2
        clip: true
        padding: 0
        onOpened: {
            directionImage.source = "res/arrowUp01.png"
        }
        onClosed: {
            directionImage.source = "res/arrowDown01.png"
        }
        contentItem: Rectangle {
            id: dropDownArea
            //height: 0
            //width: parent.width
            clip: true
            radius: 2
            //anchors.horizontalCenter: parent.horizontalCenter
            //anchors.topMargin: 2
            //color: "#f0f0f0"
            border.color: "lightgray"
            border.width: 1

            // drop down
            ListView {
                id: listview
                width: parent.width
                height: dropDownArea.height
                anchors.top: parent.top
                anchors.topMargin: 3
                anchors.left: parent.left

                model:  rootComboBox.model
                spacing: 3.5
                currentIndex: 0
                delegate: itemDelegate
                header: headerComponent
                footer: footerComponent
                //headerPositioning: ListView.OverlayHeader
                //footerPositioning: ListView.PullBackFooter
            }
            Component {
                id: headerComponent
                Item {
                    width: parent.width
                    height: 18
                    Image {
                        //anchors.centerIn: parent
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                        height: parent.height - 4
                        width: height
                        source: "res/arrowDown02.png"
                    }
                }
            }
            Component {
                id: footerComponent
                Item {
                    width: parent.width
                    height: 18
                    Image {
     //                   anchors.centerIn: parent
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                        height: parent.height - 4
                        width: height
                        source: "res/arrowUp02.png"
                    }
                }
            }
            Component {
                id: itemDelegate
                Item {
                    width: ListView.view.width
                    height: selectorRect.height - 6
                    //anchors.centerIn:
                    Text{
                        text: isNaN(rootComboBox.model) ? modelData : index + numberStartV
                        font.pointSize: rootComboBox.font.pointSize - 1
                        z: 1
                        verticalAlignment: Text.AlignVCenter
                        //anchors.horizontalCenter: parent.horizontalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: parent.width * 0.05
                    }
                    //selected click
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            //rootComboBox.state = "" //"noDrop"
                            dropDown.close()
                            //selectedText.text = modelData
                            listview.currentIndex = index
                            rootComboBox.currentIndex = index
                            emit: rootComboBox.selected(index)
                            //console.log("index: " + index + "currentIndex: " + listview.currentIndex)
                        }
                    }
                }
            }

        }
    }
}
