<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8" />
<meta http-equiv="X-UA-Compatible" content="IE=8" >  
<link rel="stylesheet" href="../css/main.css"  type="text/css">
<script type="text/javascript" src="../script/jquerymin.js"></script>
<script type="text/javascript" src="../script/jquery.cookie.js"></script>
<script type="text/javascript" src="../script/jquery.xml2json.js"></script>
<script type="text/javascript" src="../script/common.js"></script>
<script type="text/javascript" src="My97DatePicker/WdatePicker.js"></script>
<script type="text/javascript" src='../script/artDialog/jquery.artDialog.js?skin=simple'></script>
<script type="text/javascript" src='../script/artDialog/iframeTools.js'></script>
<script type="text/javascript">
/*************************************************
Function:		LastPage
Description:	主页面加载时，获取cookie，跳转到刷新前的界面
Input:			无
Output:			无
return:			无				
*************************************************/
function LatestPage()
{
	m_szUserPwd = $.cookie('userInfo'+m_lHttpPort);
	if(m_szUserPwd == null)
	{
		window.location.href="login.asp";
		return;
	}
	var curpage = $.cookie('page');
	if(null == curpage)
	{
		ChangeFrame("preview.asp",1);
	}else
	{
		ChangeFrame(curpage.split("%")[0],curpage.split("%")[1]);
	}
	//GetDeviceInfo(); // 重复交互
}

/*************************************************
Function:		ChangeFrame
Description:	主页面加载时，获取cookie，跳转到刷新前的界面
Input:			src:页面路径
				index:ID序号
Output:			无
return:			无				
*************************************************/
function ChangeFrame(src,index)
{
	$("#volumeDiv").remove();
	$("#WndArrangePart").remove();
	GetDeviceInfo(); //切换界面时实时获取设备名称
	$("#ContentFrame").attr("src",src);
}

/*************************************************
Function:		ChangeMenu
Description:	改变主页菜单栏
Input:			index:ID序号
Output:			无
return:			无				
*************************************************/
function ChangeMenu(index)
{
	for(var i = 1;i <= $("*[id^='iMenu']").length;i++)
	{
		if($("#iMenu"+i).hasClass("menuBackground"))
		{
			$("#iMenu"+i).removeClass("menuBackground");
		}
		
		if("" != $("#iMenu"+i).attr(""))
		{
			
		}
	}
	$("#iMenu"+index).addClass("menuBackground");
}

/*************************************************
Function:		ChangeFrameLanguage
Description:	改变页面语言
Input:			lan:语言
Output:			无
return:			无				
*************************************************/
function ChangeFrameLanguage(lan)
{
	ContentFrame.window.ChangeLanguage('',lan);
}

/*************************************************
Function:		iframeAutoFit
Description:	处理iframe高度自适应
Input:			无
Output:			无
return:			无				
*************************************************/
function iframeAutoFit()
{
	try
	{
		var a = document.getElementById("ContentFrame");		
		a.style.height = document.body.clientHeight - 138 + 'px';
		a.style.width = document.body.clientWidth - 28 + 'px';
	}
	catch(ex)
	{
	}
}

/*************************************************
Function:		GetDeviceInfo
Description:	获取设备名称
Input:			无
Output:			无
return:			无
*************************************************/
function GetDeviceInfo()
{
	szXmlhttp=new getXMLHttpRequest();
	var szURL=m_lHttp+m_szHostName+":"+m_lHttpPort+"/PSIA/System/deviceInfo";
	szXmlhttp.open("GET",szURL,false);
	szXmlhttp.setRequestHeader("If-Modified-Since","0");
	szXmlhttp.setRequestHeader("Authorization","Basic "+m_szUserPwd);
	szXmlhttp.send(null);
	GetDeviceInfoCallback();
}
function GetDeviceInfoCallback()
{
	if(szXmlhttp.readyState==4)
	{
		if(szXmlhttp.status==200)
		{
			var xmlDoc=new createxmlDoc();
			xmlDoc = szXmlhttp.responseXML;
	
			var obj = $.xml2json(xmlDoc);
			
			$('#main_type').html(obj.model);
			
			if(obj['deviceName']){
				m_szDeviceName = obj['deviceName'];
			}else{
				m_szDeviceName = "Embedded Net DVR";
			}
			
		    try{
		    	if(!top._device_type_obj_){
		    		top._device_type_obj_ = {};
		    	}
		    	
		    	var M = obj['realModel'] ? obj['realModel'] : obj['model'];
		    	
		    	
		    	if(M.indexOf("TS") >= 0){
		    		top._device_type_ = "TS";
		    	}else if(M.indexOf('VAR') >= 0){
		    		top._device_type_ = "VAR";
		    	}
		    	top._device_type_obj_['proType'] = top._device_type_;	
		    	
		    	var cus = obj['customConfig'];
		    	if(cus && cus.toUpperCase() == 'TRUE'){
		    		top._device_type_obj_['customConfig'] = true;
		    	}else{
		    		top._device_type_obj_['customConfig'] = false;
		    	}
		    }catch(e){}
		}
		else
		{
			return;
		}
	}
}

//事件捕获，主要是为了处理chrome下iframe的高度异常问题
$(this).bind('resize', iframeAutoFit);

function showHelp()
{
	if(document.getElementById("SoftwareEdition").style.display != "block")
	{
	   $("#SoftwareEdition").show();
	}
}

function hideHelp()
{
	if(document.getElementById("SoftwareEdition").style.display == "block")
	{
	   $("#SoftwareEdition").hide();
	}
}

var m_iTime = 0;
var m_iTimerID = 0;
/*************************************************
Function:		ShowTipsDiv
Description:	显示滑动提示框
Input:			szTitle 标题
				szTips  提示语
Output:			无
return:			无
*************************************************/
function ShowTipsDiv(szTitle, szTips)
{
	if(m_iTime == 0)
	{
		$('#TipsDiv').show();
		if(navigator.appName == 'Microsoft Internet Explorer'){
			$('#TipsIframe').show();
		}			
	}
	
	if(0 != arguments.length)
	{
		if(m_iTimerID)
		{
			clearTimeout(m_iTimerID);
			m_iTimerID = 0;
		}
		$(document.getElementById("TipsDiv")).css("bottom", "-106px");
		if(navigator.appName == 'Microsoft Internet Explorer')
		{
			$(document.getElementById("TipsIframe")).css("bottom", "-106px");
		}
		m_iTime = 0;
		document.getElementById("TipsTitle").innerHTML = szTitle;
		document.getElementById("CurTips").innerHTML = szTips;
	}
	
	if(m_iTime >= 20)
	{
		m_iTimerID= setTimeout("HideTipsDiv()",1000);
		return;
	}
	var iBottom = parseInt(document.getElementById("TipsDiv").style.bottom.replace("px", ""));
	$(document.getElementById("TipsDiv")).css("bottom", (iBottom+5)+"px");
	if(navigator.appName == 'Microsoft Internet Explorer')
	{
	    var iframeBottom = parseInt(document.getElementById("TipsIframe").style.bottom.replace("px", ""));
	    $(document.getElementById("TipsIframe")).css("bottom", (iframeBottom+5)+"px");
	}
	m_iTime++;
	
	m_iTimerID = setTimeout("ShowTipsDiv()",20);
}

/*************************************************
Function:		HideTipsDiv
Description:	隐藏滑动提示框
Input:			无
Output:			无
return:			无
*************************************************/
function HideTipsDiv()
{
	if(m_iTime <= 0)
	{
		m_iTime = 0;
		m_iTimerID = 0;
		return;
	}
	var iBottom = parseInt(document.getElementById("TipsDiv").style.bottom.replace("px", ""));
	$(document.getElementById("TipsDiv")).css("bottom", (iBottom-5)+"px");
	if(navigator.appName == 'Microsoft Internet Explorer')
	{
	    var iframeBottom = parseInt(document.getElementById("TipsIframe").style.bottom.replace("px", ""));
	    $(document.getElementById("TipsIframe")).css("bottom", (iframeBottom-5)+"px");
	}
	m_iTime--;
	
	m_iTimerID = setTimeout("HideTipsDiv()",20);
	if(m_iTime == 0)
	{
		$('#TipsDiv').hide();
		if(navigator.appName == 'Microsoft Internet Explorer'){
			$('#TipsIframe').hide();
		}	
	}
}
</script>
</head>
<body onLoad="LatestPage()">
<div style="width:100%;height:100%;">
  <div id="header">
    <div style="width:100%;height:68px">
	  <table cellpadding="0" style="width:100%;height:68px;border-collapse:collapse">
		<tr style="width:100%;height:68px">
		<td style="width:781px;background:url(../images/public/banner/mid.png) no-repeat;"><span id='main_type' style="padding-left:400px; font-size:32px; color:#adacac;"></span>
		  </td>
		  <td id="headBgLeft" bgcolor="#1e0a07">&nbsp;</td>
		  <td id="headBgRight" style="background:url(../images/public/banner/other.png); width:212px"></td>
		</tr>
	  </table>
	  <!--<div id="LogoEdition"></div>-->
	  <div id="LanguageAndEdition">
	    <!--<span onclick="DeviceRestart()" style="color:#b26062;cursor:pointer;">设备重启&nbsp;&nbsp;</span>-->
		<span onClick="showHelp()" style="color:#b26062;cursor:pointer; font-size:24"><label id='laHelp'>帮助</label>&nbsp;&nbsp;</span>
	    <span>
          <select id="LanguageSelect" name="LanguageSelect" onChange="ChangeFrameLanguage(this.value)" style="width:100px; height:20px; display:none;">
          </select>
		</span>
      </div>
	  <div onMouseOver="showHelp()" onMouseOut="hideHelp()" id="SoftwareEdition" style="width:134px;height:50px;background:url(../images/main/EditionPopBackground.png)">
        <div style="height:25px; line-height:25px; padding-left:10px; color:#757575;">
          <span><label>Web:</label>&nbsp;&nbsp;&nbsp;&nbsp;3.0.1.121207</span>
        </div>	
        <div style="height:25px; line-height:25px; padding-left:10px; color:#757575;">
          <span><label>Plugin:</label>&nbsp;3.0.3.6</span>
        </div>	
      </div>
    </div> 
    <div id="main_menu">
      <div class="main_table">
        <span style="float:left;display:block;width:50px;">&nbsp;</span>
        <span style="float:left;display:block;text-align:center;width:111px;" id="iMenu1" class="menuBackground">
          <label id="laPreview" onMouseOver="this.className='linklabelSel'" onMouseOut="this.className='linklabel'" class="linklabel" onClick="ChangeFrame('preview.asp',1)"></label>
        </span>
        <span style="float:left;display:block;text-align:center;width:111px;" id="iMenu2">
          <label id="laPlayback" onMouseOver="this.className='linklabelSel'" onMouseOut="this.className='linklabel'" class="linklabel" onClick="ChangeFrame('playback.asp',2)"></label>
        </span>
		<span style="float:left;display:block;text-align:center;width:111px;" id="iMenu5">
          <label id="laTraffic" onMouseOver="this.className='linklabelSel'" onMouseOut="this.className='linklabel'" class="linklabel" onClick="ChangeFrame('query/trafficQuery.asp',5)"></label>
        </span>
        <span style="float:left;display:block;text-align:center;width:111px;" id="iMenu3">
          <label id="laLog" onMouseOver="this.className='linklabelSel'" onMouseOut="this.className='linklabel'" class="linklabel" onClick="ChangeFrame('log.asp',3)"></label>
        </span>
        <span style="float:left;display:block;text-align:center;width:111px;" id="iMenu4">
          <label id="laConfig" onMouseOver="this.className='linklabelSel'" onMouseOut="this.className='linklabel'" class="linklabel" onClick="ChangeFrame('paramconfig.asp',4)"></label>
        </span>
		<span style="float:right;display:block;width:20px;">&nbsp;</span>
        <span style="float:right;display:block;text-align:center;">
		  <label style="color:#727272; font-size:12px" id = "curruser"></label>
		  <label>&nbsp;&nbsp;</label>
          <label id="laExit" style="color:#727272; font-size:12px; cursor:pointer;" onClick="GoAway()" ></label>
        </span>
      </div>
    </div>
  </div>	

  <div id="MainFrame">
    <iframe frameborder="0" scrolling="no" id="ContentFrame" name="ContentFrame" onload="iframeAutoFit()" src="">
	</iframe>
  </div>
  
  <div id="footer">
    <!--<label style="font-size:12px;color:#727272" id="laCopyRight">©Hikvision Digital Technology Co., Ltd. All Rights Reserved.
	</label>-->
  </div>
</div>  
<div id='TipsDiv' style='border:1px solid #808080;position:absolute; height:100px; width:150px; z-index:999; right:0px; bottom:-106px; display:none;'> 
  <table cellspacing='0' cellpadding='0' border='0' width='100%' height='100%'>
    <tr height='20'><td align='left' bgcolor='#000' id='TipsTitle' style="color:#fff;"></td></tr>
    <tr><td align='center' id='CurTips' bgcolor='#000' style="color:#fff;"></td></tr>
  </table>
</div>
<iframe id='TipsIframe' style='position:absolute; height:102px; width:152px; z-index:1; right:0px; bottom:-106px;display: none;'>
</iframe>
<iframe id="downFrame" name='downFrame' style="display: none;"></iframe>
</body>
</html>
