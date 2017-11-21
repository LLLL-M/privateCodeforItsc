import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Dialogs 1.2

Item {
    //signal tscConnect(var ip, int type)
    signal tscConnect(string ip, int type)
    property alias orderNum: orderNumber.text
    //signal testSig()
    Image {
        height: 12
        width: height * 7
        source: "images/hikvision.png"
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 5
    }

    Column {
        anchors.centerIn: parent
        spacing: 30

        Rectangle {
            width: 450
            height: 125
            Image {
                height: 80
                width: 80
                source: "images/ftest.png"
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Label {
                height: 35
                text: qsTr("海康信号机工装测试程序")
                font.pointSize: 15
                //anchors.centerIn: parent
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        Column{
            spacing: 20
            Row {
                spacing: 20
                //IP
                Rectangle  {
                    height: 35
                    width: 230
                    //color: "lightgray"
                    Label {
                        id: ltscIP
                        width: 80
                        height: parent.height
                        text: qsTr("信号机IP：")
                        font.pointSize: 12
                        verticalAlignment: Label.AlignVCenter
                    }
                    TextField {
                        id: tscIP
                        height: parent.height
                        width: 140
                        anchors.right: parent.right
                        clip: true
                        placeholderText: qsTr("192.168.1.101")
                        font.pointSize: 12
                        text: qsTr("192.168.1.101")
                        selectByMouse: true
                        focus: true
                        //acceptableInput: true
                        //onAccepted: {connectButton.clicked()}
                    }
                }
                //type
                Rectangle  {
                    height: 35
                    width: 220
                    Label {
                        id: ltscType
                        width: 80
                        height: parent.height
                        text: qsTr("信号机型号：")
                        font.pointSize: 12
                        verticalAlignment: Label.AlignVCenter
                    }
                    MComboBox {
                        id: tscType
                        height: parent.height
                        width: 120
                        anchors.right: parent.right
                        model: ["DS-TSC300", "DS-TSC500"]
                        currentIndex: 0
                        displayText: currentText
                        font.pointSize: 12
                    }
                }
            }

            //订单号输入
            Rectangle  {
                height: 35
                width: 270
                //color: "lightgray"
                Label {
                    id: lOrderNumber
                    width: 80
                    height: parent.height
                    text: qsTr("订单号：")
                    font.pointSize: 12
                    verticalAlignment: Label.AlignVCenter
                }
                TextField {
                    id: orderNumber
                    height: parent.height
                    width: 180
                    anchors.right: parent.right
                    clip: true
                    placeholderText: qsTr("123456789")
                    font.pointSize: 14
                    //text: qsTr("192.168.1.101")
                    selectByMouse: true
                    //acceptableInput: true
                    //onAccepted: {connectButton.clicked()}
                }
            }
        }











        Rectangle {
            width: 450
            height: 35

            MButton {
                id: connectButton
                width: 100
                height: 35
                checkable: true
                text: !checkstate ? qsTr("连接") : qsTr("取消")
                color: !checkstate ? "#6a9eea" : "#eb6b6a"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    if(checkstate == false)
                    {
                        connecting.visible = false
                        disableUi(false)
                        return
                    }
                    var ip = tscIP.text;
                    var order = orderNumber.text
                    if(isValidIP(ip) && (order.length > 0))
                    {
                        connecting.visible = true
                        disableUi(true)
                        emit: tscConnect(ip, tscType.currentIndex)
                        //emit: testSig()
                        //console.log("01 - send tscConnect signal...")
                    }
                    else
                    {
                        ipMsgBox.text = !isValidIP(ip) ? qsTr("输入IP错误，请重新输入!") : qsTr("请输入正确的订单号！")
                        ipMsgBox.open()
                        checkstate = false
                        //console.log("no")
                    }
                }
            }
        }
    }
    MessageDialog{
        id: ipMsgBox
        modality: Qt.ApplicationModal
        icon: StandardIcon.Critical
        title: qsTr("错误")
        //text: qsTr("输入IP错误，请重新输入!")
    }

    ProgressBar {
        id: connecting
        indeterminate: true
        value: 50
        visible: false
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 60
        anchors.horizontalCenter: parent.horizontalCenter
        width: 300
    }

    function isValidIP(ip)
    {
        var reg = /^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/
        return reg.test(ip)
    }
    function disableUi(flag)
    {
        if(flag == true)
        {
            ltscIP.enabled = false
            ltscType.enabled = false
            tscIP.enabled = false
            tscType.enabled = false
            lOrderNumber.enabled = false
            orderNumber.enabled = false
        }
        else
        {
            ltscIP.enabled = true
            ltscType.enabled = true
            tscIP.enabled = true
            tscType.enabled = true
            lOrderNumber.enabled = true
            orderNumber.enabled = true
        }
    }
    function tscTypeMisMatch()
    {
        ipMsgBox.text = qsTr("信号机类型与所选类型不匹配！");
        ipMsgBox.open()
        connecting.visible = false
        disableUi(false)
        connectButton.checkstate = false
    }
}
