import QtQuick 2.0

Item {
    width: 248 * ratio
    height: 420 * ratio
    property real ratio: 1

    property var objMapping: [pressLight, autolight, manuallight, yellowblink, allred, steplight]
    property int lastButton: 0
    property int colorIndex: 0
    property string imgPath: "images/wireless/"
    property var imgsrc: [["part00.png", "part01.png"],		//top led
        ["part50.png", "part51.png", "part52.png", "part53.png", "part54.png"],	//auto
        ["part30.png", "part31.png", "part32.png", "part33.png", "part34.png"],	//manuallight
        ["part10.png", "part11.png", "part12.png", "part13.png", "part14.png"],	//yellowblink
        ["part40.png", "part41.png", "part42.png", "part43.png", "part44.png"],	//allred
        ["part20.png", "part21.png", "part22.png", "part23.png", "part24.png"]]	//steplight
    signal buttonClicked(var val)
    Image{
        id: pressLight
        width: 248 * ratio
        height: 32 * ratio
        source: imgPath + imgsrc[0][0]
    }
    Image{
        id: yellowblink
        width: 248 * ratio
        height: 42 * ratio
        source: imgPath + imgsrc[3][0]
        anchors.top: pressLight.bottom
    }

    Row{
        id: middleButtons
        anchors.top: yellowblink.bottom
        width: 248 * ratio
        height: 108 * ratio
        Image{
            id: steplight
            width: 86 * ratio
            height: parent.height
            source: imgPath + imgsrc[5][0]
        }
        Image{
            id: manuallight
            width: 75 * ratio
            height: parent.height
            source: imgPath + imgsrc[2][0]
        }
        Image{
            id: allred
            width: 87 * ratio
            height: parent.height
            source: imgPath + imgsrc[4][0]
        }
    }
    Image {
        id: autolight
        width: 248 * ratio
        height: 159 * ratio
        source: imgPath + imgsrc[1][0]
        anchors.top: middleButtons.bottom
    }
    Timer {
        id: timer
        interval: 500
        repeat: false
        running: true
        onTriggered: {
           ledShow(false)
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


    function ledShow(flag)
    {
        if(flag)
        {
            pressLight.source = imgPath + imgsrc[0][1]
            timer.start()
        }
        else
            pressLight.source = imgPath + imgsrc[0][0]
    }
    function setButtonColor(buttonindex, colorindex)
    {
        ledShow(true)
        if((buttonindex >=1 && buttonindex < 6) && (colorindex >= 0 && colorindex < 5))
        {
            objMapping[buttonindex].source = imgPath + imgsrc[buttonindex][colorindex]
            //console.log("set button"+buttonindex + " colorsurce:" + imgsrc[buttonindex][colorindex])
        }
        if(lastButton != buttonindex)
        {
            objMapping[lastButton].source = imgPath + imgsrc[lastButton][0]
            lastButton = buttonindex
        }
    }
    function showButtonClicked(buttonindex)
    {
        if(buttonindex == lastButton)
        {
            colorIndex++
            if(colorIndex > 4)
                colorIndex = 1
        }
        else
            colorIndex = 1
        setButtonColor(buttonindex, colorIndex)
    }
    function toDefault()
    {
        //if(lastButton != 0)
        //    objMapping[lastButton].source = imgPath + imgsrc[lastButton][0]
        lastButton = 0
        colorIndex = 0
        for(var i=1; i<6; i++)
            setButtonColor(i, 0)
    }
}
