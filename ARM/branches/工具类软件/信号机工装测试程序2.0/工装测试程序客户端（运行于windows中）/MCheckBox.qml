import QtQuick 2.0

Item {
    id: rootCheckbox
    height: 20	//must bigger than 6
    width: height + text.length * 9 + 3 // 3-->left margin
    enabled: !disabled

    property string text
    property int fontsize: 12
    property bool fontbold: false
    //property Font font
    property bool checked: false
    property bool disabled

    signal clicked(var state)

    Rectangle {
        height: parent.height - 6
        width: height
        radius: 2
        border.width: 1
        border.color: 'gray'
        anchors.left: parent.left
        anchors.leftMargin: 2
        anchors.verticalCenter: parent.verticalCenter
        opacity: disabled ? 0.5 : 1

        MouseArea{
            anchors.fill: parent
            onClicked: {
                rootCheckbox.checked = (checkImg.visible == false ? true : false)

                emit: rootCheckbox.clicked(checkImg.visible)
            }
        }
    }
    Image {
        id: checkImg
        height: parent.height - 3
        width: parent.height
        source: "res/tick.png"
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 3
        z: 1
        visible: rootCheckbox.checked
        opacity: disabled ? 0.5 : 1
    }

    Text {
        width: text.length * 9
        height: parent.height
        text: rootCheckbox.text
        font.pointSize: rootCheckbox.fontsize
        font.bold: rootCheckbox.fontbold
        anchors.left: checkImg.right
        anchors.leftMargin: 3
        verticalAlignment: Text.AlignVCenter
        opacity: disabled ? 0.5 : 1
    }
}
