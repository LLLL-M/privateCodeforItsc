import QtQuick 2.0
import QtQuick.Controls 2.0

Item {
    id: rootIOInput
    width: 170
    height: 450
    signal buttonClicked(var val)

    Column {
        spacing: 15
        anchors.centerIn: parent

        Flickable {
            width: rootIOInput.width - 10
            height: rootIOInput.height - 10 -35
            //anchors.centerIn: parent
            ScrollBar.vertical: ScrollBar{}
            flickableDirection: Flickable.VerticalFlick
            clip: true
            contentHeight: 35 * 24
            contentWidth: parent.width - 10

            Column {
                spacing: 5
                anchors.centerIn: parent

                Repeater {
                    id: ioInputRep0	//IO1-8
                    model: 8
                    delegate: Row {
                        spacing: 10
                        property alias checkstate: fswitch.checked
                        Label {
                            height: 30
                            verticalAlignment: Label.AlignVCenter
                            text: "IO输入口"+(index + 1)
                        }
                        RSwitch {
                            id: fswitch
                            readonly: true
                        }
                    }
                }

                Repeater {
                    id: ioInputRep1	//IO9-13
                    model: ["全红", "黄闪", "步进", "自动", "手动"]
                    property var colors: ["#f5f5f5","red", "yellow", "green", "blue", "purple"]
                    property int colorIndex: 0
                    delegate: Row {
                        spacing: 10
                        property alias color: wirelessKeys.color
                        Label {
                            height: 30
                            verticalAlignment: Label.AlignVCenter
                            text: "IO输入口"+(index + 9)
                        }
                        MButton {
                            id: wirelessKeys
                            text: modelData
                        }
                    }
                }

                Repeater {
                    id: ioInputRep2	//IO14-32
                    model: 11
                    delegate: Row {
                        spacing: 10
                        property alias checkstate: bswitch.checked
                        Label {
                            height: 30
                            verticalAlignment: Label.AlignVCenter
                            text: "IO输入口"+(index + 14)
                        }
                        RSwitch {
                            id: bswitch
                            readonly: true
                        }
                    }
                }

            }
        }
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
    }
    function updateIOStatus(status)
    {
        console.log("IOstatus:")
        for(var j=0; j<24; j++)
            console.log(status[j])
        var i = 0;
        for(; i<8; i++)
            ioInputRep0.itemAt(i).checkstate = (status[i] == 0 ? true : false)
        for(i=0; i<5; i++)
        {
            ioInputRep1.itemAt(i).color = (status[i+8] == 0 ? ioInputRep1.colors[ioInputRep1.colorIndex] : ioInputRep1.colors[0])
        }
        ioInputRep1.colorIndex++
        if(ioInputRep1.colorIndex > 5)
            ioInputRep1.colorIndex  = 1
        for(i=0; i<11; i++)
            ioInputRep2.itemAt(i).checkstate = (status[i+13] == 0 ? true : false)
    }
    function toDefault()
    {
        var status = new Array(1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1)
        updateIOStatus(status)
    }
}
