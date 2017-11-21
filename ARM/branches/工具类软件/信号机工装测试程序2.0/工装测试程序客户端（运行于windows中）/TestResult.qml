import QtQuick 2.0

Item {
    signal buttonClicked(var val)
    Row {
        spacing: 15
        anchors.horizontalCenter: parent.horizontalCenter
        MButton {
            width: 60
            height: 28
            color: '#65ca63'
            text: qsTr("通过")
            onClicked: {
                emit: buttonClicked(1)
            }
        }
        MButton {
            width: 60
            height: 28
            color: '#eb6b6a'
            text: qsTr("失败")
            onClicked: {
                emit: buttonClicked(2)
            }
        }
    }
    function toDefault()
    {

    }
}
