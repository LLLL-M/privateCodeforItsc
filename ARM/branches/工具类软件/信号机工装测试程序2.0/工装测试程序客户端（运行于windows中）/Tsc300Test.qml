import QtQuick 2.0
import QtQuick.Controls 2.0


Item {
    id: rootTscTest
//    width: 640
//    height: 480

    property int noticeIndex: 0
    //property var buttonColorFlag:[0,0,0,0,0,0,0,0,0]
    property var objMapping:[tUsb,t485Serial,tWifi3G,tAuto,tLampLight,tFontBoardLight, tGps, tWirelessKey,tPedKey, tKeyBoard,lampCtrlPage.curVoltButton, lampCtrlPage.lightButton]
    readonly property var colorSrc:["#f5f5f5", "#65ca63", "#eb6b6a"] //0 - default  1-green(pass) 2-red(failed)
    property var itemNotice: ["鼠标移动到测试项按钮上可以查看该项的测试提示信息",//default
    "1.将USB设备正确插在信号机中如下图所示位置处(USB设备中至少有一个文件或文件夹).\n2.点击USB测试按钮，观察输出结果.",//usb
    "1.将对接后的两个485串口正确插入信号机中如下图所示的位置.\n2.点击RS485串口测试按钮，观察输出结果.",//rs485
    //"1.将外接强电插口插入信号机中如下图所示位置处。\n2.点击电流电压测试按钮，观察输出结果是否正确.",//cur and volt
    "1.点击Wifi/3G测试按钮，观察输出结果是否正确.",//wifi/3g
    "点击后可自动按顺序测试USB、RS485串口和Wifi/3G，对于测试未通过的选项，可以再次点击该项进行单独测试.",//auto
    "1.点击灯控板测试按钮.\n2.在右侧窗口中选择要测试的灯控板，选择后可以点击电流电压测试或者依次点灯测试(需观察选中灯控板上的指示灯亮灭情况)进行测试.\n3.测试完成后根据实际测试情况选择点击通过或者失败按钮，完成退出该项测试.",//lamp
    "1.点击前面板指示灯测试按钮.\n2.观察信号机上对应下图中所标位置处的指示灯亮灭情况是否正确(路口图中的指示灯依次点亮每个方向的左转，直行，右转和行人的红黄绿,程序运行和控制按键指示灯常亮，测试结束后熄灭)",//fontboard
    "1.点击GPS测试按钮.\n2.观察前面板上的GPS指示灯是否闪烁（指示灯闪烁表面GPS正常）.\n3.测试完成点击结束测试按钮退出该项测试.",//gps
    "1.点击无线遥控器测试按钮.\n2.依次按下无线遥控器上的任意按键\n3.观察界面输出结果是否正确.",//wireless
    "1.将外接测试按键正确接入信号机中如下图所标位置处.\n2.按下外接测试按键上的任意按键（也可不按任何按键，查看状态).\n3.点击行人按钮测试按钮.\n4.观察界面输出结果是否正确.",//ped key
    "1.点击手动控制面板测试按钮.\n2.依次按下手动控制面板上的按键，观察对应按键上的指示灯是否被点亮（除自动按键外，其他按键被按下时，手动按键和被按下按键指示灯都会被点亮）.\n3.测试完成点击结束测试按钮退出该项测试."]//keyboard
    property var imgNotice: ["","images/tsc300/usb.png","images/tsc300/rs485.png",  /*"images/tsc300/curvolt.png",*/ "", "", "images/tsc300/lights.png",
        "images/tsc300/fontboard.png", "images/tsc300/gps.png", "images/wireless.png", "images/tsc300/pedkeys.png", "images/tsc300/keyboard.png",]
    //signal setLogFilePath(string path)
    //signal logFinished()

    function setButtonColor(buttonindex, colorvalue)
    {
        //rootTscTest.enabled = true
        if(buttonindex < 10)
            disableAll(false)
        else
            lampCtrlPage.disableAll(false)
        //console.log("recv setbutton color. button"+buttonindex + " = "+colorvalue)
        if((buttonindex >=0 && buttonindex < 12)	// 0-11 buuton
                && (colorvalue >= 0 && colorvalue < 3)) //0 - default  1-green(pass) 2-red(failed)
        {
            //rootTscTest.buttonColorFlag[buttonindex] = colorvalue
            objMapping[buttonindex].color = colorSrc[colorvalue]
        }
        if(tAuto.checkstate)
        {
            if(buttonindex != 2)
                disableAll(true)
            else
                tAuto.checkstate = false
        }
    }

    Rectangle {
        width: 500
        height: 326 //425
        anchors.top: parent.top

        Column {
            anchors.top: parent.top
            anchors.topMargin: 45
            anchors.left: parent.left
            anchors.leftMargin: 3
            spacing: 20

            //auto test Area
            Rectangle {
                id: autoTestArea
                width: 500 - 6
                height: 110//150
                border.color: 'lightgray'
                border.width: 1

                Row {
                    anchors.top: parent.top
                    anchors.topMargin: 10
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 30

                    MButton {
                        id: tUsb
                        height: 35
                        width: 90
                        text: qsTr("USB测试")
                        idfy: 1
                        onEntered: noticeIndex = 1
                        onExited: noticeIndex = 0
                        objectName: qsTr("usbButton")
                        //color: colorSrc[buttonColorFlag[0]]

                        onClicked: {
                            disableAll(true)
                            //rootTscTest.enabled = false
                            //console.log("color:" + color + " colorfalg:" + colorSrc[buttonColorFlag[0]])
                        }

                    }
                    MButton {
                        id: t485Serial
                        height: 35
                        width: 130
                        text: qsTr("RS485串口测试")
                        idfy: 2
                        onEntered: noticeIndex = 2
                        onExited: noticeIndex = 0
                        objectName: qsTr("rs485Button")
                        //color: "#eb6b6a"
                        onClicked: {
                            //rootTscTest.enabled = false
                            disableAll(true)
                        }
                    }
                    /*
                    MButton {
                        id: tCurVolt
                        height: 35
                        width: 120
                        text: qsTr("电流电压测试")
                        idfy: 3
                        onEntered: noticeIndex = 3
                        onExited: noticeIndex = 0
                        objectName: qsTr("curVoltButton")
                        onClicked: {
                            //rootTscTest.enabled = false
                            disableAll(true)
                        }
                    }
                    */
                    MButton {
                        id: tWifi3G
                        height: 35
                        width: 120
                        text: qsTr("Wifi/3G测试")
                        idfy: 4
                        onEntered: noticeIndex = 3
                        onExited: noticeIndex = 0
                        objectName: qsTr("wifi3GButton")
                        onClicked: {
                            //rootTscTest.enabled = false
                            disableAll(true)
                        }
                    }
                }
                MButton {
                    id: tAuto
                    height: 35
                    width: 150
                    text: qsTr("自动测试以上三项")
                    checkable: true
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 10
                    onEntered: noticeIndex = 4
                    onExited: noticeIndex = 0
                    objectName: qsTr("autoTestButton")
                    onClicked: {
                        disableAll(true)
                    }
                }
            }//end of auto

            //led light test area
            Rectangle {
                width: autoTestArea.width
                height: 65
                border.color: 'lightgray'
                border.width: 1

                Row {
                    anchors.top: parent.top
                    anchors.topMargin: 15
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 25


                    MButton {
                        id: tLampLight
                        height: 35
                        width: 120
                        //text: qsTr("灯控板测试")
                        checkable: true
                        text: checkstate ? qsTr("结束测试") : qsTr("灯控板测试")
                        onEntered: noticeIndex = 5
                        onExited: noticeIndex = 0
                        objectName: qsTr("lampCtrlButton")
                        onClicked: {
                            //infoText.append("注意观察灯控板上的指示灯变化是否正确!\n")
                            //disableAll(true)
                            checkButtonSwitch(tLampLight, lampCtrlPage, checkstate, "")
                        }
                    }

                    MButton {
                        id: tFontBoardLight
                        height: 35
                        width: 150
                        text: qsTr("前面板指示灯测试")
                        onEntered: noticeIndex = 6
                        onExited: noticeIndex = 0
                        objectName: qsTr("frontBoardButton")
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: {
                            //infoText.text = "注意观察前面板上路口图中的指示灯、GPS和运行指示灯以及按键指示灯变化是否正确！"
                            infoText.append("注意观察前面板上路口图中的指示灯、GPS和运行指示灯以及按键指示灯变化是否正确！\n")
                            disableAll(true)
                        }
                    }

                    MButton {
                        id: tGps
                        height: 35
                        width: 110
                        checkable: true
                        text: checkstate ? qsTr("结束测试") : qsTr("GPS测试")
                        onEntered: noticeIndex = 7
                        onExited: noticeIndex = 0
                        objectName: qsTr("gpsButton")
                        onClicked: {
                            //if(checkstate)
                            //    infoText.append("注意观察GPS指示灯是否闪烁!\n")
                            //disableAll(checkstate)
                            //tGps.disabled = false
                            checkButtonSwitch(tGps, gpsResultPage, checkstate, "注意观察GPS指示灯是否闪烁!\n")
                        }
                    }

                }
            }//end of light

            //key test area
            Rectangle {
                width: autoTestArea.width
                height: 65
                border.color: 'lightgray'
                border.width: 1

                Row {
                    anchors.top: parent.top
                    anchors.topMargin: 15
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 30

                    MButton {
                        id: tWirelessKey
                        height: 35
                        width: 140
                        checkable: true
                        text: checkstate ? qsTr("结束测试") : qsTr("无线遥控器测试")
                        onEntered: noticeIndex = 8
                        onExited: noticeIndex = 0
                        objectName: qsTr("wirelessButton")
                        onClicked: {
                            //disableAll(true)
                            //tWirelessKey.disabled = false
                            checkButtonSwitch(tWirelessKey, wirelessState, checkstate, "按下遥控器上的按键观察屏幕输出是否正确！\n")
                        }
                    }
                    MButton {
                        id: tPedKey
                        height: 35
                        width: 120
                        text: checkstate ? qsTr("结束测试") : qsTr("行人按钮测试")
                        onEntered: noticeIndex = 9
                        onExited: noticeIndex = 0
                        objectName: qsTr("pedKeysButton")
                        checkable: true
                        onClicked: {
                            checkButtonSwitch(tPedKey, pedButtonPage, checkstate, "改变接在信号机行人按键接口上的按键状态观察显示输出状态是否正确！\n")
                        }
                    }

                    MButton {
                        id: tKeyBoard
                        height: 35
                        width: 140
                        checkable: true
                        text: checkstate ? qsTr("结束测试") : qsTr("手动控制面板测试")
                        onEntered: noticeIndex = 10
                        onExited: noticeIndex = 0
                        objectName: qsTr("keyBoardButton")
                        onClicked: {
                            //if(checkstate)
                             //   infoText.append("按下按键后注意观察控制面板上对应的指示灯是否被点亮!\n")
                            //disableAll(checkstate)
                            //tKeyBoard.disabled = false
                            checkButtonSwitch(tKeyBoard, keyboardResultPage, checkstate, "按下按键后注意观察控制面板上对应的指示灯是否被点亮!\n");
                        }
                    }

                }
            }//end of key
        }
    }
    //right
    Rectangle {
        width: 180
        height: parent.height - 10
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 5
        anchors.topMargin: 5
        border.width: 1
        border.color: 'lightgray'


        Rectangle{
            id: noticeTitle
            width: 100
            height: 20
            anchors.top: parent.top
            anchors.topMargin: 15
            anchors.left: parent.left
            anchors.leftMargin: 3

            Image{
                height: parent.height
                width: parent.height
                source: "images/tips.png"
            }
            Label {
                height: parent.height
                width: parent.width - parent.height
                text: qsTr("测试提示:")
                font.pointSize: 12
                color: 'red'
                anchors.right: parent.right
                verticalAlignment: Label.AlignVCenter
            }
        }

        TextArea {
            id: noticeText
            width: parent.width - 5
            height: 300
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.top: parent.top
            anchors.topMargin: 45
            text: itemNotice[noticeIndex]
            wrapMode: TextArea.Wrap
            clip: true
            font.pointSize: 10
            color: "green"
        }

        Image {
            id: noticeImg
            width: parent.width - 6
            height: 170
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            anchors.left: parent.left
            anchors.leftMargin: 3
            source: imgNotice[noticeIndex]
        }

        LampCtrl {
            id: lampCtrlPage
            anchors.centerIn: parent
            visible: false
            objectName: "lampCtrlPage"
            onButtonClicked: {
                if(val == 3 || val == 4)
                {
                    tLampLight.color = colorSrc[val-2]
                    tLampLight.checkstate = false

                    checkButtonSwitch(tLampLight, lampCtrlPage, false, "")
                }
            }
        }

        MWireless{
            id: wirelessState
            ratio: 0.72
            anchors.centerIn: parent
            visible: false
            objectName: "wirelessState"
            onButtonClicked: {
                tWirelessKey.color =  colorSrc[val]
                tWirelessKey.checkstate = false
                emit: tWirelessKey.clicked()
                checkButtonSwitch(tWirelessKey, wirelessState, false, "")
            }
        }

        PedKeys {
            id: pedButtonPage
            anchors.centerIn: parent
            visible: false
            objectName: "pedButtons"
            onButtonClicked: {
                tPedKey.color =  colorSrc[val]
                tPedKey.checkstate = false
                emit: tPedKey.clicked()
                checkButtonSwitch(tPedKey, pedButtonPage, false, "")
            }
        }
        TestResult {
            id: gpsResultPage
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 200
            anchors.horizontalCenter: parent.horizontalCenter
            visible: false
            onButtonClicked: {
                tGps.color = colorSrc[val]
                tGps.checkstate = false
                emit: tGps.clicked()
                checkButtonSwitch(tGps, gpsResultPage, false, "")
            }
        }

       TestResult {
            id: keyboardResultPage
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 200
            anchors.horizontalCenter: parent.horizontalCenter
            visible: false
            onButtonClicked: {
                tKeyBoard.color = colorSrc[val]
                tKeyBoard.checkstate = false
                emit: tKeyBoard.clicked()
                checkButtonSwitch(tKeyBoard, keyboardResultPage, false, "")
            }
        }

    }
    //bottom
    Rectangle{
    //TextArea{
        width: 500
        height: 243-6
        border.width: 2
        color: Qt.darker("#f0f0f0", 1.05)
        border.color: '#f0f0f0'
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 3

        Flickable {
            width: parent.width - 2
            height: parent.height -2
            flickableDirection: Flickable.VerticalFlick
            anchors.centerIn: parent
            TextArea.flickable: infoText
            ScrollBar.vertical: ScrollBar{}
            //draggingHorizontally: false
        }

        TextArea{
            id: infoText
            wrapMode: TextEdit.Wrap
            clip: true
            readOnly: true
            selectByMouse: true
            objectName: "outputText"
        }
    }

    function init()
    {
        noticeIndex = 0
        for(var i=0; i<10; i++)
           setButtonColor(i, 0); //default color
    }

    function disableAll(flag)
    {
       for(var i=0; i<10; i++)
          objMapping[i].disabled = flag
    }

    function checkButtonSwitch(obj, objpage, checkstate, info)
    {
        if(checkstate == true)
        {
            noticeTitle.visible = false
            noticeText.visible = false
            noticeImg.visible = false
            //ioOutputPage.visible = true
            objpage.toDefault()
            objpage.visible = true
            disableAll(true)
            //tIoOutput.disabled = false
            obj.disabled = false
            infoText.append(info)
        }
        else
        {
            noticeTitle.visible = true
            noticeText.visible = true
            noticeImg.visible = true
            //ioOutputPage.visible = false
            objpage.visible = false
            disableAll(false)
            //color = colorSrc[1]
        }
    }
}
