import QtQuick 2.0

Item {
    id: rootItem
    width: 70 + calcTextLen(text)
    height: 30
    opacity: !disabled ? 1 : 0.6
    enabled: !disabled

    property bool checked: false
    property bool readonly: false
    property bool disabled: false
    property string text: ""
    readonly property var imgsrc: ["switchon.png", "switchoff.png"]
    signal clicked()

    Image {
        id: bakImg
        //anchors.fill: parent
        anchors.left: parent.left
        width: 70
        height: parent.height
        source: "res/"+imgsrc[checked == true ? 0 : 1]

        MouseArea {
           anchors.fill: parent
           onClicked: {
               if(!readonly)
               {
                   checked = (checked ? false : true)
                   emit: rootItem.clicked()
               }
           }
       }
    }

    Text {
        width: parent.width - bakImg.width
        height: parent.height
        text: rootItem.text
        anchors.right: parent.right
        font.pointSize: 12
        verticalAlignment: Text.AlignVCenter
        clip: true
    }
    function calcTextLen(text)
    {
        var chlen = 0
        var pos = 0
        while((pos=escape(text).indexOf("%u", pos)) >= 0)
        {
            chlen++
            pos++
        }
        return chlen * 17 + (text.length - chlen) * 8
    }
}
