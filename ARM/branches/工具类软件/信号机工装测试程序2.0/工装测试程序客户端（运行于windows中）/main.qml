import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Dialogs 1.2

Window {
    id: rootWin
    visible: true
    width: 700
    height: 600//495
    maximumWidth: 710
    maximumHeight: 610
    minimumWidth: 695
    minimumHeight: 595
    title: qsTr("海康信号机工装测试程序V2.0")
    flags: /*Qt.Dialog*/ Qt.Window
    objectName: "rootWindow"
    /*property var closeObj: 0
    onClosing:
    {
        console.log("close..")
        close.accepted = false
        closeObj = close
        emit: winClose()
    }
    function winExit()
    {
        closeObj.accepted = true
    }
    */
    property int chosenType: 0
    signal setLogFilePath(string path)
    signal logStart(string order)
    signal logFinished()
    signal clearLog()
    //signal winClose()
    signal testOut()

/************* menu bar *************/

    Row{
        id: menuBar
        //spacing: 10
        visible: false
        anchors.fill: parent
        z: 100
        Button {
            id: menubutton
            text: qsTr("控制")
            font.pointSize: 11
            width: 50
            height: 25
            onClicked: menu.open()
            Menu {
              id: menu
              width: 120
              y: menubutton.height
              MenuItem {
                  text: qsTr("   重新开始测试")
                  onTriggered: {
                      if(chosenType == 1)
                          t300TestUI.init()
                      else
                          t500TestUI.init()
                      emit: clearLog()
                  }
              }
              MenuItem {
                  id: logRecordItem
                  text: qsTr("记录输出日志");
                  checkable: true
                  checked: true
                  onTriggered: {
                      if(checked)
                          emit: logStart()
                      else
                          emit: logFinished()
                          //console.log("end of log")
                  }
              }

              MenuItem {
                  text: qsTr("   清除日志");
                  onTriggered: {
                      emit: clearLog()
                  }
              }

              MenuItem {
                  text: qsTr("   完成&退出测试");
                  onTriggered: {
                      emit: testOut()
                  }
              }
            }
        }

        Button {
            id: helpbutton
            text: qsTr("帮助")
            font.pointSize: 11
            width: 50
            height: 25
            onClicked: helpmenu.open()
            Menu {
              id: helpmenu
              width: 50
              y: helpbutton.height
              MenuItem {
                  text: qsTr("关于")
                  onTriggered: {
                      aboutDialog.open()
                  }
              }
            }
        }
    }
/*
    FileDialog {
        id: saveLog
        title: qsTr("请选择要保存的日志文件路径并输入名称")
        nameFilters: ["日志文件 (*.log)", "所有文件 (*.*)"]
        selectExisting: false
        onAccepted: {
            var filepath = new String(fileUrl)
            //console.log("000 file path(slice8): " + filepath.slice(8))
            emit: setLogFilePath(filepath.slice(8))
        }
        onRejected: {
            logRecordItem.checked = false
        }
    }
*/
    Dialog {
        id: aboutDialog
        width: 410
        height: 300
        objectName: "aboutDialog"
        title: qsTr("关于海康信号机工装测试程序V2.0")
        property alias version: versionText.text
        contentItem: Rectangle {
            width: 410
            height: 300

            Rectangle {
                width: 410
                height: 200
                Image {
                    height: 13
                    width: 7* height
                    source: "images/hikvision.png"
                    anchors.top: parent.top
                    anchors.topMargin: 3
                }
                Row{
                    anchors.centerIn: parent
                    spacing: 10
                    Image {
                        width: 50
                        height: 50
                        source: "images/ftest.png"
                    }

                    Column {
                        height: 40
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8
                        Text {
                            text: qsTr("海康信号机工装测试程序V2.0")
                            font.pointSize: 14
                            verticalAlignment: Text.AlignVCenter
                        }
                        Text {
                            id: versionText
                            text: qsTr("版本:  v2.0.0-b20170517")
                            font.pointSize: 10
                        }
                    }
                }
            }
            Rectangle {
                width: 410
                height: 90
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 20
                Text {
                    //anchors.centerIn: parent
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    //width: 100
                    clip: true
                    text: qsTr("本计算机程序是一个测试海康信号机设备的软件，主要用于在出\n厂时测试检验信号机设备功能的完整性。本软件只限公司内部使\n用，严禁传播到公司之外。")
                    font.pointSize: 10
                }
            }
            Text {
                text: qsTr("Any problem or advice, please contact panwenjun@hikvision.com.cn")
                font.pointSize: 7
                anchors.bottom: parent.bottom
                anchors.right: parent.right
            }
        }
    }
/*************** end of menu bar **************/

    ConnectTsc {
        id: loginUI
        anchors.fill: parent
        onTscConnect: {
            chosenType = type + 1
            //if(chosenType == 2)
            //    tscConnected(true)
            if(logRecordItem.checked)
                emit: logStart(loginUI.orderNum)
        }
        objectName: "connectTsc"
    }

    Tsc300Test {
        id: t300TestUI
        anchors.fill: parent
        visible: false
        objectName: "tsc300Test"
    }

    Tsc500Test {
        id: t500TestUI
        anchors.fill: parent
        visible: false
        objectName: "tsc500Test"
    }

    function tscConnected(succflag)
    {
        //console.log("tsc connected. type: " + chosenType +"...")
        if(true == succflag)
        {
            loginUI.visible = false
            if(chosenType == 1)
                t300TestUI.visible = true
            else
                t500TestUI.visible = true
            menuBar.visible = true
        }
        else
            loginUI.tscTypeMisMatch()
    }
}
