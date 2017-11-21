import QtQuick 2.0
import QtQuick.Controls 2.0


Item {
    id: rootTsc500cTest
//    width: 640
//    height: 480

    property int noticeIndex: 0
    property var objMapping:[tUsb,tRs232,tRs422,tRs485,tWifi3G,tAuto,tLampLight,tIoOutput,tGps,tIoInput,tCarDetector,tKeyBoard,lampCtrlPage.objMapping[0], lampCtrlPage.objMapping[1]]
    readonly property var colorSrc:["#f5f5f5", "#65ca63", "#eb6b6a"] //0 - default  1-green(pass) 2-red(failed)
    property var itemNotice: ["鼠标移动到测试项按钮上可以查看该项的测试提示信息",//default
    "1.将USB设备正确插在信号机中如下图所示位置处(USB设备中至少有一个文件或文件夹).\n2.点击USB测试按钮，观察输出结果.",//usb
    "1.将232接口测试设备正确接入信号机中如下图所示位置处。\n2.点击RS232测试按钮，观察输出结果。",//232
    "1.将422接口测试设备正确接入信号机中如下图所示位置处。\n2.点击RS422测试按钮，观察输出结果。",//422
    "1.将对接后的两个485串口正确插入信号机中如下图所示的位置.\n2.点击RS485串口测试按钮，观察输出结果.",//rs485
    //"1.将测试设备的外接强电插口插入信号机中如下图所示位置处。\n2.点击电流电压测试按钮，观察输出结果是否正确.",//cur and volt
    "1.确认WIFI/3G模块已正确安装到信号机上。\n2.点击Wifi/3G测试按钮，观察输出结果是否正确.",//wifi/3G
    "点击后可自动按顺序测试USB接口、RS232接口、RS422接口、RS485串口、灯控板电流电压和Wifi/3G，对于测试未通过的选项，可以再次点击该项按钮进行单独测试.",//auto
    "1.点击灯控板指示灯测试按钮.\n2.勾选需要测试的灯控板号.\n3.点击电流电压测试按钮观察输出结果是否正确\n4.点击依次点灯测试按钮观察信号机上灯控板上的指示灯亮灭情况是否正确(依次点亮灯控板上的指示灯)",//lamp
    "1.将外接的测试设备连接到如图所示的20个IO输出口上(转接板上标注的为NO1~NO20).\n2.点击界面上的IO输出口测试按钮.\n3.点击右边窗口中的全部输出高电平按钮，观察连接测试设备的指示灯亮灭情况.(图示为低电平点亮指示灯)\n4.点击右边窗口中的全部输出低电平按钮，观察连接测试设备的指示灯亮灭情况.",//io output
    "1.点击GPS测试按钮.\n2.观察如下图所示主控板上的GPS指示灯是否闪烁（指示灯闪烁表明GPS正常）.\n3.测试完成点击结束测试按钮退出该项测试.",//gps
    "1.将外接的测试设备连接到如图所示的32个IO输入口上(转接板上标注的为F1+ F1- ~ F32+ F32-),其中第9 ~ 13个IO输入口应该连接的为无线遥控器接收模块.\n2.点击IO输入口测试按钮开始测试.\n3.依次按下无线遥控器上的任意按键，观察界面第9到13输入口状态显示\n4.改变其他输入口外接测试设备的状态，观察界面上对应位置的状态显示是否正确.",//ioinput
    "1.将车检器测试设备正确接入信号机中如下图所标位置处.\n2.改变车检器测试设备上的各车检器状态.\n3.观察界面显示状态是否正确.",//cardetector
    "1.点击手动控制面板测试按钮.\n2.依次按下手动控制面板上的按键，观察对应按键上的指示灯是否被点亮（除自动按键外，其他按键被按下时，手动按键和被按下按键指示灯都会被点亮）.\n3.测试完成点击结束测试按钮退出该项测试."]//keboard
    property var imgNotice: ["","images/tsc500/usb.png","images/tsc500/232.png","images/tsc500/422.png","images/tsc500/485.png", "", "", "images/tsc500/lamp.png",
        "images/tsc500/iooutput.png", "images/tsc500/gps500.png", "images/tsc500/ioinput.png", "images/tsc500/cardet01.png", "images/tsc500/keyboard500.png"]
    signal clicked(int buttonIndex)

    function setButtonColor(buttonindex, colorvalue)
    {
        if(buttonindex < 12)
            disableAll(false)
        else
            lampCtrlPage.disableAll(false)

        //console.log("recv setbutton color. button"+buttonindex + " = "+colorvalue)
        if((buttonindex >=0 && buttonindex < 14)	// 0-13 buuton
                && (colorvalue >= 0 && colorvalue < 3)) //0 - default  1-green(pass) 2-red(failed)
        {
            objMapping[buttonindex].color = colorSrc[colorvalue]
        }
        if(tAuto.checkstate)
        {
            if(buttonindex != 4)
                disableAll(true)
            else
                tAuto.checkstate = false
        }
    }

    Rectangle {
        width: 500
        height: 326 //425
        anchors.top: parent.top
        //border.width: 1

        Column {
            anchors.top: parent.top
            anchors.topMargin: 35
            anchors.left: parent.left
            anchors.leftMargin: 3
            spacing: 17

            //auto test Area
            Rectangle {
                id: autoTestArea
                width: 500 - 6
                height: 150
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
                        onClicked: {
                            disableAll(true)
                            emit: rootTsc500cTest.clicked(1)
                        }

                    }
                    MButton {
                        id: tRs232
                        height: 35
                        width: 110
                        text: qsTr("RS232测试")
                        idfy: 2
                        onEntered: noticeIndex = 2
                        onExited: noticeIndex = 0
                        objectName: qsTr("rs232Button")
                        onClicked: {
                            disableAll(true)
                            emit: rootTsc500cTest.clicked(2)
                        }
                    }
                    MButton {
                        id: tRs422
                        height: 35
                        width: 110
                        text: qsTr("RS422测试")
                        idfy: 3
                        onEntered: noticeIndex = 3
                        onExited: noticeIndex = 0
                        objectName: qsTr("rs422Button")
                        onClicked: {
                            disableAll(true)
                            emit: rootTsc500cTest.clicked(3)
                        }
                    }
                }

                Row {
                    anchors.top: parent.top
                    anchors.topMargin: 55
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 40

                    MButton {
                        id: tRs485
                        height: 35
                        width: 100
                        text: qsTr("RS485测试")
                        idfy: 4
                        onEntered: noticeIndex = 4
                        onExited: noticeIndex = 0
                        objectName: qsTr("rs485Button")
                        onClicked: {
                            disableAll(true)
                            emit: rootTsc500cTest.clicked(4)
                        }
                    }

                    MButton {
                        id: tWifi3G
                        height: 35
                        width: 120
                        text: qsTr("Wifi/3G测试")
                        idfy: 6
                        onEntered: noticeIndex = 5
                        onExited: noticeIndex = 0
                        objectName: qsTr("wifi3GButton")
                        onClicked: {
                            disableAll(true)
                            emit: rootTsc500cTest.clicked(5)
                        }
                    }
                }

                MButton {
                    id: tAuto
                    height: 35
                    width: 150
                    text: qsTr("自动测试以上五项")
                    checkable: true
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 10
                    onEntered: noticeIndex = 6
                    onExited: noticeIndex = 0
                    objectName: qsTr("autoTestButton")
                    onClicked: {
                        disableAll(true)
                        emit: rootTsc500cTest.clicked(6)
                    }
                }
            } //end of auto

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
                    spacing: 30

                    MButton {
                        id: tLampLight
                        height: 35
                        width: 150
                       // text: qsTr("灯控板指示灯测试")
                        checkable: true
                        text: checkstate ? qsTr("结束测试") : qsTr("灯控板测试")
                        onEntered: noticeIndex = 7
                        onExited: noticeIndex = 0
                        objectName: qsTr("lampCtrlButton")
                        onClicked: {
                            //infoText.append("注意观察灯控板上的指示灯变化是否正确!\n")
                            //disableAll(true)
                            //emit: rootTsc500cTest.clicked(8)
                            checkButtonSwitch(tLampLight, lampCtrlPage, checkstate, "")
                        }
                    }
                    MButton {
                        id: tIoOutput
                        height: 35
                        width: 120
                        checkable: true
                        text: checkstate ? qsTr("结束测试") : qsTr("IO输出口测试")
                        onEntered: noticeIndex = 8
                        onExited: noticeIndex = 0
                        objectName: qsTr("ioOutputButton")
                        onClicked:{
                            checkButtonSwitch(tIoOutput, ioOutputPage, checkstate, "点击输出高/低电平按钮观察连接的LED亮灭情况是否正确！\n");
                            //ioOutputSwitch(checkstate)
                        }
                    }
                    MButton {
                        id: tGps
                        height: 35
                        width: 90
                        checkable: true
                        text: checkstate ? qsTr("结束测试") : qsTr("GPS测试")
                        idfy: 1
                        onEntered: noticeIndex = 9
                        onExited: noticeIndex = 0
                        objectName: qsTr("gpsButton")
                        onClicked: {
                            checkButtonSwitch(tGps, gpsResultPage, checkstate, "注意观察GPS指示灯是否闪烁!\n")
                            emit: rootTsc500cTest.clicked(10)
                        }

                    }
                }
            } //end of light

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
                    spacing: 20

                    MButton {
                        id: tIoInput
                        height: 35
                        width: 130
                        checkable: true
                        text: checkstate ? qsTr("结束测试") : qsTr("IO输入口测试")
                        onEntered: noticeIndex = 10
                        onExited: noticeIndex = 0
                        objectName: qsTr("ioInputButton")
                        onClicked: {
                            //checkButtonSwitch(tIoInput, ioInputPage, checkstate, "改变接在信号机输入口上的设备(无线遥控器)按键状态观察显示输出状态是否正确！\n")
                            emit: rootTsc500cTest.clicked(11)
                        }
                        function changeState(flag)
                        {
                            //console.log("flag: " + flag + ", chekstate: " + tIoInput.checkstate)
                            if(flag == false && tIoInput.checkstate == true)
                                tIoInput.checkstate = false
                            checkButtonSwitch(tIoInput, ioInputPage, tIoInput.checkstate, "改变接在信号机输入口上的设备(无线遥控器)按键状态观察显示输出状态是否正确！\n")
                        }
                    }
                    MButton {
                        id: tCarDetector
                        height: 35
                        width: 120
                        text: checkstate ? qsTr("结束测试") : qsTr("车检器测试")
                        onEntered: noticeIndex = 11
                        onExited: noticeIndex = 0
                        objectName: qsTr("carDetectorButton")
                        checkable: true
                        onClicked: {
                            checkButtonSwitch(tCarDetector,carDetectorPage,checkstate, "改变各车检器状态观察界面上实时状态显示输出是否正确！\n")
                            emit: rootTsc500cTest.clicked(12)
                            //infoText.append("改变接在信号机行人按键接口上的按键状态观察显示输出状态是否正确！\n")
                        }
                    }
                    MButton {
                        id: tKeyBoard
                        height: 35
                        width: 150
                        checkable: true
                        text: checkstate ? qsTr("结束测试") : qsTr("手动控制面板测试")
                        onEntered: noticeIndex = 12
                        onExited: noticeIndex = 0
                        objectName: qsTr("keyBoardButton")
                        onClicked: {
                            emit: rootTsc500cTest.clicked(13)
                             checkButtonSwitch(tKeyBoard, keyboardResultPage, checkstate, "按下按键后注意观察控制面板上对应的指示灯是否被点亮!\n")
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

        IOOutput {
            id: ioOutputPage
            anchors.centerIn: parent
            visible: false
            objectName: "IOOutputPage"
            onButtonClicked: {
                if(index >52 && index < 55)
                {
                    tIoOutput.color = colorSrc[index - 52]
                    tIoOutput.checkstate = false

                    checkButtonSwitch(tIoOutput, ioOutputPage, false,"")
                    //ioOutputSwitch(false)
                }
            }
        }

        IOInput {
            id: ioInputPage
            anchors.centerIn: parent
            visible: false
            objectName: "IOInputPage"
            onButtonClicked: {
                tIoInput.color =  colorSrc[val]
                tIoInput.checkstate = false
                emit: rootTsc500cTest.clicked(11)
                checkButtonSwitch(tIoInput,ioInputPage, false, "")
            }

        }

        CarDetPage {
            id: carDetectorPage
            anchors.centerIn: parent
            visible: false
            objectName: "CarDetPage"
            onButtonClicked: {
                tCarDetector.color =  colorSrc[val]
                tCarDetector.checkstate = false
                emit: rootTsc500cTest.clicked(12)
                checkButtonSwitch(tCarDetector,carDetectorPage, false, "")
            }
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
                else
                    emit: rootTsc500cTest.clicked(val+6)	//7 & 8
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
        }

        TextArea{
            id: infoText
            wrapMode: TextEdit.Wrap
            clip: true
            readOnly: true
            selectByMouse: true
            objectName: "outputText500"
        }
    }

    function init()
    {
        noticeIndex = 0
        for(var i=0; i<12; i++)
           setButtonColor(i, 0); //default color
    }

    function disableAll(flag)
    {
       for(var i=0; i<12; i++)
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

