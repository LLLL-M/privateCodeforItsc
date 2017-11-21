import QtQuick 2.0

Item {
    id: rootItem
    height: 420
//    property var buttonCheckFlags: [0,0,0,0,0,0,0,0]
    property var rsids: [rs1, rs2, rs3, rs4, rs5, rs6, rs7, rs8]
    property var rsState: [false, true]
    signal buttonClicked(var val)

    Column {
        spacing: 5
        anchors.centerIn: parent
        /*
        Repeater {
            model: 8
            delegate: RSwitch {
                readonly: true
                text: qsTr("行人按键") + (index + 1)
                //objectName: "pedKey"+index
                checked: rootItem.buttonCheckFlags[index] == 0 ? false : true
            }
        }
        */
        RSwitch {
            id: rs1
            readonly: true
            text: qsTr("行人按键") + 1
        }
        RSwitch {
            id: rs2
            readonly: true
            text: qsTr("行人按键") + 2
        }
        RSwitch {
            id: rs3
            readonly: true
            text: qsTr("行人按键") + 3
        }
        RSwitch {
            id: rs4
            readonly: true
            text: qsTr("行人按键") + 4
        }
        RSwitch {
            id: rs5
            readonly: true
            text: qsTr("行人按键") + 5
        }
        RSwitch {
            id: rs6
            readonly: true
            text: qsTr("行人按键") + 6
        }
        RSwitch {
            id: rs7
            readonly: true
            text: qsTr("行人按键") + 7
        }
        RSwitch {
            id: rs8
            readonly: true
            text: qsTr("行人按键") + 8
        }
    }
    Row {
        spacing: 15
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom

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
    function updateButtonCheckState(status)
    {
        for(var i=0; i<8; i++)
            rsids[i].checked = rsState[status[i]]

    }
    function toDefault()
    {
        var status = new Array(0,0,0,0,0,0,0,0)
        updateButtonCheckState(status)
        //free(status)
    }
}
