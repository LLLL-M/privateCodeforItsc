import QtQuick 2.0
import QtQuick.Controls 2.0

Item {
    id: rootCarDet
    width: 170
    height: 450
    property var imgSrc: ["images/tsc500/road.png", "images/tsc500/car.png"]
    signal buttonClicked(var val)
    Column {
        spacing: 15
        anchors.centerIn: parent

        Flickable{
            width: rootCarDet.width - 10
            height: rootCarDet.height - 10 -35
            ScrollBar.vertical: ScrollBar{}
            flickableDirection: Flickable.VerticalFlick
            clip: true
            contentHeight: 30*48
            contentWidth: parent.width - 10
            Column {
                spacing: 5
                anchors.centerIn: parent

                Repeater {
                    id: cardetRepeater
                    model: 48
                    delegate:Row {
                        height: 25
                        spacing: 10
                        property alias source: stateImg.source
                        Label {
                            height: 25
                            verticalAlignment: Label.AlignVCenter
                            text: "通道" + (index+1) + "过车状态:"
                        }
                        Image {
                            id: stateImg
                            width: 25
                            height: 25
                            source: rootCarDet.imgSrc[0]
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
    function updateCarDetStatus(status)
    {
        for(var i=0; i<48; i++)
        {
            ///console.log("1-" + i + "==>"+ cardetRepeater.itemAt(i).source)
            cardetRepeater.itemAt(i).source = rootCarDet.imgSrc[status[i]]
            //rootCarDet.cardetState[i] = rootCarDet.imgSrc[status[i]]
        }
    }
    function toDefault()
    {
        var status = new Array(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
                               0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0)
        updateCarDetStatus(status)
    }
}
