import QtQuick 2.0

Item {
    id: rootIOOupt
    width: 150
    height: 400
    signal buttonClicked(int index)
    property var imgSrc: ["images/tsc500/lampOff.png", "images/tsc500/lampOn.png"]
    Column{
        spacing: 20
        Image {
            id: lightState
            width: 100
            height: 100
            source: rootIOOupt.imgSrc[0]
            anchors.horizontalCenter: parent.horizontalCenter
        }

        MButton {
            id: buttonHight
            width: 150
            height: 38
            text: qsTr("20个输出高电平")
            checkable: true
            onClicked: {
                lightState.source = imgSrc[0]
                emit: buttonClicked(51)
                buttonLow.checkstate = false
            }
        }

        MButton {
            id: buttonLow
            width: 150
            height: 38
            text: qsTr("20个输出低电平")
            checkable: true
            onClicked: {
                lightState.source = imgSrc[1]
                emit: buttonClicked(52)
                buttonHight.checkstate = false
            }
        }
        Row {
            spacing: 10

            MButton {
                width: 60
                height: 35
                color: '#65ca63'
                text: qsTr("通过")
                onClicked: {
                    emit: buttonClicked(53)
                }
            }
            MButton {
                width: 60
                height: 35
                color: '#eb6b6a'
                text: qsTr("失败")
                onClicked: {
                    emit: buttonClicked(54)
                }
            }
        }
    }
    function toDefault()
    {
        lightState.source = imgSrc[0]
        buttonHight.checkstate = false
        buttonLow.checkstate = false
    }
}
