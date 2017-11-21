import QtQuick 2.5
Item {
    id: rootButton
    width: 60
    height: 30
    opacity: disabled ? 0.6 : 1
    enabled: !disabled
    property var color: "#f5f5f5"
    property string text: ""
    property int fontsize: 12
    property bool fontbold: false
    //property Font font:{bold: false; pointSize: 12}
    property bool checkable: false
    property bool checkstate: false
    property string background:""
    signal clicked()
    signal entered(var idfy)
    signal exited(var idfy)
    property int idfy: 0
    property bool disabled: false

    Rectangle {
        id: buttonRect
        width: parent.width
        height: parent.height
        radius: 2
        color: checkstate == false ? rootButton.color : Qt.darker(rootButton.color, 1.05)
        border.color: Qt.darker(rootButton.color, 1.05)//"#cbcbcb"
        border.width: rootButton.background == "" ? 2 : 0
        Text {
             text: rootButton.text
             anchors.centerIn: parent
             font.pointSize: rootButton.fontsize
             font.bold: rootButton.fontbold
        }
        Image {
            anchors.fill: parent
            source: background
            z: 100
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                if(disabled)
                    return
                if(rootButton.checkable == true)
                {
                    checkstate = (checkstate == true ? false : true)
                }
                else
                    clickedAnim.start()

                emit: rootButton.clicked()
            }
            onEntered: {
                 if(disabled)
                    return
                emit: rootButton.entered(rootButton.idfy)
            }
            onExited: {
                if(disabled)
                    return
                emit: rootButton.exited(rootButton.idfy)
            }
        }


        SequentialAnimation{
            id: clickedAnim
            property int duration: 90
            ColorAnimation {
                target: buttonRect
                properties: "color"
                from: rootButton.color
                to: "white"//"lightgray"
                duration: 0.5 * clickedAnim.duration
            }
            ColorAnimation {
                target: buttonRect
                properties: "color"
                to: rootButton.color
                from: "white"//"lightgray"
                duration: 0.5 * clickedAnim.duration
            }
        }
    }
}
