import QtQuick 2.0
import QtQuick.Controls 2.0

Item {
    id: rootLampCtrl
    width: 130
    height: 400
    signal buttonClicked(var val, var states)
    property var states:[0,0,0,0,0,0]
    property var color:"#f5f5f5"
    property var curVoltButton: btCurVolt
    property var lightButton: btLight
    property var objMapping: [btCurVolt, btLight, btTestPass, btTestFail, selectAll]
    MCheckBox {
        id: selectAll
        height: 25
        fontbold: true
        fontsize: 11
        text: qsTr("全选")
        onClicked: {
            //console.log("checked: "+state)
            checkAll(state)
        }
    }
    Column {
        id: selectionArea
        anchors.top: selectAll.bottom
        anchors.topMargin: 5
        spacing: 3
        Repeater {
            id: lampRepeater
            model: 6
            delegate: MCheckBox {
                height: 30
                text: "灯控板"+(index+1)
                property int id: index
                onClicked: {
                    //console.log("id:"+id+",state:"+state)
                    rootLampCtrl.states[id] = (state == true ? 1 : 0)
                }
            }
        }
    }
    Column {
        spacing: 10
        anchors.top: selectionArea.bottom
        anchors.topMargin: 20
        MButton {
            id: btCurVolt
            width: 120
            text: qsTr("电流电压测试")
            //color: rootLampCtrl.color
            onClicked: {
                emit: rootLampCtrl.buttonClicked(1, rootLampCtrl.states)
                console.log("==:"+rootLampCtrl.states)
                disableAll(true)
            }
        }
        MButton {
            id: btLight
            width: 120
            text: qsTr("依次点灯测试")
            onClicked: {
                emit: buttonClicked(2, rootLampCtrl.states)
                disableAll(true)
            }
        }
    }
    Row{
        spacing: 15
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom

        MButton {
            id: btTestPass
            width: 60
            height: 28
            color: '#65ca63'
            text: qsTr("通过")
            onClicked: {
                emit: buttonClicked(3, states)
            }
        }
        MButton {
            id: btTestFail
            width: 60
            height: 28
            color: '#eb6b6a'
            text: qsTr("失败")
            onClicked: {
                emit: buttonClicked(4, states)
            }
        }
    }

    function toDefault()
    {
        selectAll.checked = false
        checkAll(false)
    }
    function checkAll(flag)
    {
       for(var i = 0; i<6; i++)
       {
           lampRepeater.itemAt(i).checked = flag
           states[i] = (flag == true ? 1: 0)
       }
    }
    function disableAll(flag)
    {
       for(var i=0; i<5; i++)
          objMapping[i].disabled = flag
       for(var i = 0; i<6; i++)//checkboxes
           lampRepeater.itemAt(i).disabled = flag
    }
}
