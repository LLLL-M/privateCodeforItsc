var m_xmlDoc=null;

var m_oXmlDoc = null;             //模拟通道
var m_oDigXmlDoc = null;          //数字通道
var m_oZeroXmlDoc = null;         //零通道

var m_PreviewOCX=null;
var m_szHostName="";
//var m_lHttpPort="8080";
var m_lHttpPort="80";
var m_lHttp="http://";
var m_lRtspPort="554";
var m_szUserPwdValue="";
var m_iStreamType = 0; //码流类型

var m_iChannelId = new Array(); //通道对应的ID号
for(var i=0;i<64;i++)
{
	m_iChannelId[i] = -1;
}

var m_iAnalogChannelNum=0;
var m_iDigitalChannelNum=0;
var m_iZeroChanNum = 0;

m_lHttp = location.protocol + "//";
m_szHostName = location.hostname;
if(isIPv6Add(m_szHostName))
{
	m_szHostName = "[" + m_szHostName + "]";
}
if(location.port != "")
{
	m_lHttpPort = location.port;
}
else if(m_lHttp == "https://")
{
	m_lHttpPort = "443";
}
var m_iTalkNum = 0;  //语音对讲通道数
var m_bTalk = 0; //是否正在对讲
var m_iTalkingNO = 0; //正在对讲的通道号
var m_szaudioCompressionType = 'G.711ulaw';

/*************************************************
Function:		replaceAll
Description:	替换所有
Input:			szDir:源字符
				szTar:目标字符
Output:			无
return:			无
*************************************************/
String.prototype.replaceAll = function(szDir,szTar)
{
	var szStr = this;
	while(szStr.indexOf(szDir)>=0)
	{
		szStr = szStr.replace(szDir, szTar);
	}
	return szStr;
}
/*************************************************
Function:		alert
Description:	重载页面弹出提示框
Input:			str:提示信息
Output:			无
return:			无				
*************************************************/
//if( !(navigator.userAgent.indexOf("Safari") > 0) )//判断是否为firefox
if(navigator.userAgent.indexOf("Firefox") > 0)
{
    window.alert = function(str)
    {
		var msgw,msgh,bordercolor;
		msgw=300;//提示窗口的宽度
		msgh=120;//提示窗口的高度
		bordercolor="#336699";//提示窗口的边框颜色
		titlecolor="#235cdb";//提示窗口的标题颜色
				
		var sWidth,sHeight;
		sWidth=document.body.offsetWidth;
		sHeight=document.body.offsetHeight;
	
		var bgIframeObj=document.createElement("iframe");
		bgIframeObj.style.position="absolute";
		bgIframeObj.style.top=(document.documentElement.scrollTop + (sHeight-msgh)/2) + "px";
		bgIframeObj.style.left=(sWidth-msgw)/2 + "px";
		bgIframeObj.style.background="#777";
		bgIframeObj.style.width=298 + "px";
		bgIframeObj.style.height=118 + "px";
		document.body.appendChild(bgIframeObj);
				
		var bgObj=document.createElement("div");
		bgObj.setAttribute('id','bgDiv');
		bgObj.style.position="absolute";
		bgObj.style.top="0";
		bgObj.style.left="0";
		bgObj.style.background="#777";
		bgObj.style.filter="progid:DXImageTransform.Microsoft.Alpha(style=3,opacity=25,finishOpacity=75";
		bgObj.style.opacity="0.6";
		bgObj.style.width=sWidth + "px";
		bgObj.style.height=sHeight + "px";
		document.body.appendChild(bgObj);
				
		var msgObj=document.createElement("div")
		msgObj.setAttribute("id","msgDiv");
		msgObj.setAttribute("align","center");
		msgObj.style.position="absolute";
		msgObj.style.background="#ece9d8";
		msgObj.style.font="12px/1.6em Verdana, Geneva, Arial, Helvetica, sans-serif";
		msgObj.style.border="1px solid " + bordercolor;
		msgObj.style.width=msgw + "px";
		msgObj.style.height=msgh + "px";
		msgObj.style.top=(document.documentElement.scrollTop + (sHeight-msgh)/2) + "px";
		msgObj.style.left=(sWidth-msgw)/2 + "px";
		
		var title=document.createElement("h4");
		title.setAttribute("id","msgTitle");
		title.setAttribute("align","right");
		title.style.margin="0";
		title.style.padding="3px";
		title.style.background=bordercolor;
		title.style.filter="progid:DXImageTransform.Microsoft.Alpha(startX=20, startY=20, finishX=100, finishY=100,style=1,opacity=75,finishOpacity=100);";
		title.style.opacity="0.75";
		title.style.border="1px solid " + bordercolor;
		title.style.height="18px";
		title.style.font="12px Verdana, Geneva, Arial, Helvetica, sans-serif";
		title.style.color="white";
		title.style.cursor="pointer";
		title.innerHTML="X";
		title.onclick=function()
					  {
						  document.body.removeChild(bgObj);
						  document.body.removeChild(bgIframeObj);
						  document.getElementById("msgDiv").removeChild(title);
						  document.body.removeChild(msgObj);
					  }
					  
		 document.body.appendChild(msgObj);
		 document.getElementById("msgDiv").appendChild(title);
		 var txt=document.createElement("p");
		 txt.style.margin="1em 0"
		 txt.setAttribute("id","msgTxt");
		 txt.innerHTML=str;
		 document.getElementById("msgDiv").appendChild(txt);
		 var input = document.createElement("input");
		 input.setAttribute("type","button");
		 if(m_szLanguage == 'cn')
		 {
		     input.setAttribute("value","确定");
		 }
		 else
		 {
			input.setAttribute("value","OK"); 
		 }
		 input.style.width = "100px";
		 input.style.position="absolute";
		 input.style.top= 90+ "px";
		 input.style.left=100 + "px";
		 input.onclick=function()
					  {
						  document.body.removeChild(bgObj);
						  document.body.removeChild(bgIframeObj);
						  document.getElementById("msgDiv").removeChild(title);
						  document.body.removeChild(msgObj);
					  }
		 document.getElementById("msgDiv").appendChild(input); 
    }
}
/*************************************************
Function:		UnloadPage
Description:	子页面销毁时，修改cookie为当前页
Input:			src:页面路径
				index:ID序号
Output:			无
return:			无				
*************************************************/
function UnloadPage(src,index)
{
	$.cookie('page',src+"%"+index);
}

/*************************************************
Function:		parseXml
Description:	从xml文件中解析xml
Input:			无
Output:			无
return:			无				
*************************************************/
function parseXml(fileRoute)
{
	xmlDoc=null;
	if(window.ActiveXObject)
	{
		var xmlDom=new ActiveXObject("Microsoft.XMLDOM");
		xmlDom.async=false;
		xmlDom.load(fileRoute);
		xmlDoc=xmlDom;
	}
	else if(document.implementation&&document.implementation.createDocument)
	{
		var xmlhttp=new window.XMLHttpRequest();
		xmlhttp.open("GET",fileRoute,false);
		xmlhttp.send(null);
		xmlDoc=xmlhttp.responseXML;
	}
	else
	{
		xmlDoc=null;
	}
	return xmlDoc;
}

/*************************************************
Function:		parseXmlFromStr
Description:	从xml字符串中解析xml
Input:			szXml xml字符串
Output:			无
return:			xml文档				
*************************************************/
function parseXmlFromStr(szXml)
{
	if(null == szXml || '' == szXml)
	{
		return null;
	}
	var xmlDoc=new createxmlDoc();
	if(navigator.appName == "Netscape" || navigator.appName == "Opera")
	{
		var oParser = new DOMParser();
		xmlDoc = oParser.parseFromString(szXml,"text/xml");
	}
	else
	{
		xmlDoc.loadXML(szXml);
	}
	return xmlDoc;
}

/*************************************************
Function:		xmlToStr
Description:	xml转换字符串
Input:			Xml xml文档
Output:			无
return:			字符串				
*************************************************/
function xmlToStr(Xml)
{
	var XmlDocInfo = '';
	if(navigator.appName == "Netscape" || navigator.appName == "Opera")
	{
		var oSerializer = new XMLSerializer();
		XmlDocInfo = oSerializer.serializeToString(Xml);
	}
	else
	{
		XmlDocInfo = Xml.xml;
	}
	if(XmlDocInfo.indexOf('<?xml') == -1)
	{
		XmlDocInfo = "<?xml version='1.0' encoding='utf-8'?>" + XmlDocInfo;
	}
	return XmlDocInfo;
}

/*************************************************
Function:		Base64
Description:	Base64加密解密
Input:			无		
Output:			无
return:			无				
*************************************************/
 var Base64 = {
 
	// private property
	_keyStr : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",
 
	// public method for encoding
	encode : function (input) {
		var output = "";
		var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
		var i = 0;
 
		input = Base64._utf8_encode(input);
 
		while (i < input.length) {
 
			chr1 = input.charCodeAt(i++);
			chr2 = input.charCodeAt(i++);
			chr3 = input.charCodeAt(i++);
 
			enc1 = chr1 >> 2;
			enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
			enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
			enc4 = chr3 & 63;
 
			if (isNaN(chr2)) {
				enc3 = enc4 = 64;
			} else if (isNaN(chr3)) {
				enc4 = 64;
			}
 
			output = output +
			this._keyStr.charAt(enc1) + this._keyStr.charAt(enc2) +
			this._keyStr.charAt(enc3) + this._keyStr.charAt(enc4);
 
		}
 
		return output;
	},
 
	// public method for decoding
	decode : function (input) {
		if (!input) {
			return '';
		}
		var output = "";
		var chr1, chr2, chr3;
		var enc1, enc2, enc3, enc4;
		var i = 0;
 
		input = input.replace(/[^A-Za-z0-9\+\/\=]/g, "");
 
		while (i < input.length) {
 
			enc1 = this._keyStr.indexOf(input.charAt(i++));
			enc2 = this._keyStr.indexOf(input.charAt(i++));
			enc3 = this._keyStr.indexOf(input.charAt(i++));
			enc4 = this._keyStr.indexOf(input.charAt(i++));
 
			chr1 = (enc1 << 2) | (enc2 >> 4);
			chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
			chr3 = ((enc3 & 3) << 6) | enc4;
 
			output = output + String.fromCharCode(chr1);
 
			if (enc3 != 64) {
				output = output + String.fromCharCode(chr2);
			}
			if (enc4 != 64) {
				output = output + String.fromCharCode(chr3);
			}
 
		}
 
		output = Base64._utf8_decode(output);
 
		return output;
 
	},
 
	// private method for UTF-8 encoding
	_utf8_encode : function (string) {
		string = string.replace(/\r\n/g,"\n");
		var utftext = "";
 
		for (var n = 0; n < string.length; n++) {
 
			var c = string.charCodeAt(n);
 
			if (c < 128) {
				utftext += String.fromCharCode(c);
			}
			else if((c > 127) && (c < 2048)) {
				utftext += String.fromCharCode((c >> 6) | 192);
				utftext += String.fromCharCode((c & 63) | 128);
			}
			else {
				utftext += String.fromCharCode((c >> 12) | 224);
				utftext += String.fromCharCode(((c >> 6) & 63) | 128);
				utftext += String.fromCharCode((c & 63) | 128);
			}
 
		}
 
		return utftext;
	},
 
	// private method for UTF-8 decoding
	_utf8_decode : function (utftext) {
		var string = "";
		var i = 0;
		var c = c1 = c2 = 0;
 
		while ( i < utftext.length ) {
 
			c = utftext.charCodeAt(i);
 
			if (c < 128) {
				string += String.fromCharCode(c);
				i++;
			}
			else if((c > 191) && (c < 224)) {
				c2 = utftext.charCodeAt(i+1);
				string += String.fromCharCode(((c & 31) << 6) | (c2 & 63));
				i += 2;
			}
			else {
				c2 = utftext.charCodeAt(i+1);
				c3 = utftext.charCodeAt(i+2);
				string += String.fromCharCode(((c & 15) << 12) | ((c2 & 63) << 6) | (c3 & 63));
				i += 3;
			}
 
		}
 
		return string;
	} 
}
/*************************************************
Function:		checkPlugin
Description:	检测是否安装插件及向页面插入插件元素
Input:			iType:表示插入插件的位置，0表示本地配置或远程升级等，1表示字符叠加等，2表示预览节界面
				szInfo:提示信息
				iWndType:窗口分割模式
Output:			true:插件已安装；false:插件未安装
return:			无				
*************************************************/
function checkPlugin(iType, szInfo, iWndType, szPlayMode, bQueryPage)
{
	var ppath = "../../codebase/WebComponents.exe";
	if(bQueryPage){
		ppath = "../../../codebase/WebComponents.exe";
	}
	if(navigator.appName == "Netscape" || navigator.appName == "Opera")
	{
		var bInstalled = false;
		var len = navigator.mimeTypes.length;
		for(var i=0;i<len;i++)
		{
			if(navigator.mimeTypes[i].type.toLowerCase() == "application/hwp-webvideo-plugin")
			{
				bInstalled = true;
				if(iType == '0')
				{
				    $("#main_plugin").html("<embed type='application/hwp-webvideo-plugin' id='PreviewActiveX' width='1' height='1' name='PreviewActiveX' align='center' wndtype='"+iWndType+"' playmode='"+szPlayMode+"'>");
				}
				else if(iType == '1')
				{
					$("#main_plugin").html("<embed type='application/hwp-webvideo-plugin' id='PreviewActiveX' width='352' height='288' name='PreviewActiveX' align='center' wndtype='"+iWndType+"' playmode='"+szPlayMode+"'>");
				}
				else
				{
					$("#main_plugin").html("<embed type='application/hwp-webvideo-plugin' id='PreviewActiveX' width='100%' height='100%' name='PreviewActiveX' align='center' wndtype='"+iWndType+"' playmode='"+szPlayMode+"'>");
				}
				$("#PreviewActiveX").css('width','99.99%'); 
				break;
			}
		}
		if(!bInstalled)
		{
			if(navigator.platform == "Win32")
			{
				$("#main_plugin").html("<label id='laPlugin' onclick='window.open(\""+ppath+"\",\"_self\")' class='pluginLink' onMouseOver='this.className =\"pluginLinkSel\"' onMouseOut='this.className =\"pluginLink\"'>"+szInfo+"<label>");
			}
			else if(navigator.platform == "Mac68K" || navigator.platform == "MacPPC" || navigator.platform == "Macintosh")
			{
				$("#main_plugin").html("<label id='laPlugin' onclick='' class='pluginLink' onMouseOver='' onMouseOut=''>"+szInfo+"<label>");
			}
			else
			{
				$("#main_plugin").html("<label id='laPlugin' onclick='' class='pluginLink' onMouseOver='' onMouseOut=''>"+szInfo+"<label>");
			}
		  	return false;
		}
	}
	else
	{
		$("#main_plugin").html("<object classid='clsid:E7EF736D-B4E6-4A5A-BA94-732D71107808' codebase='' standby='Waiting...' id='PreviewActiveX' width='100%' height='100%' name='ocx' align='center' ><param name='wndtype' value='"+iWndType+"'><param name='playmode' value='"+szPlayMode+"'></object>");
		var previewOCX=document.getElementById("PreviewActiveX");
		if(previewOCX == null || previewOCX.object == null)
		{
			if((navigator.platform == "Win32") || (navigator.platform == "Windows"))
			{
				$("#main_plugin").html("<label id='laPlugin' onclick='window.open(\""+ppath+"\",\"_self\")' class='pluginLink' onMouseOver='this.className =\"pluginLinkSel\"' onMouseOut='this.className =\"pluginLink\"'>"+szInfo+"<label>");
			}
			else if(navigator.platform == "Mac68K" || navigator.platform == "MacPPC" || navigator.platform == "Macintosh")
			{
				$("#main_plugin").html("<label id='laPlugin' onclick='' class='pluginLink' onMouseOver='' onMouseOut=''>"+szInfo+"<label>");
			}
			else
			{
				$("#main_plugin").html("<label id='laPlugin' onclick='' class='pluginLink' onMouseOver='' onMouseOut=''>"+szInfo+"<label>");
			}
			
		  return false;
		}
	}
	return true;
}
/*************************************************
Function:		CompareFileVersion
Description:	比较文件版本
Input:			无
Output:			无
return:			false:需要更新 true：不需要更新
*************************************************/
function CompareFileVersion()
{
	var previewOCX=document.getElementById("PreviewActiveX");
	if(previewOCX == null)
	{
		return false;
	}
	var xmlDoc=parseXml("../xml/version.xml");
	
	var szXml = xmlToStr(xmlDoc);
	var bRes = false;
	try
	{
		bRes = !previewOCX.HWP_CheckPluginUpdate(szXml);
		return bRes;
	}
	catch(e)
	{
		if(m_szBrowser != 'Netscape')
		{
			if(1 == CompareVersion("WebVideoActiveX.ocx"))
			{
				return false;		//插件需要更新
			}
		}
		else
		{
			if(1 == CompareVersion("npWebVideoPlugin.dll"))
			{
				return false;		//插件需要更新
			}
		}
		
		if(1 == CompareVersion("PlayCtrl.dll"))
		{
			return false;		//插件需要更新
		}
		
		if(1 == CompareVersion("StreamTransClient.dll"))
		{
			return false;		//插件需要更新
		}
		
		if(1 == CompareVersion("NetStream.dll"))
		{
			return false;		//插件需要更新
		}
		
		if(1 == CompareVersion("SystemTransform.dll"))
		{
			return false;		//插件需要更新
		}
		
		return true;
	}
}

/*************************************************
Function:		CompareVersion
Description:	比较文件版本
Input:			文件名
Output:			无
return:			-1 系统中的版本高 0 版本相同 1 设备中的版本高
*************************************************/
function CompareVersion(szFileName)
{
	var xmlDoc=parseXml("../xml/version.xml");
	
	var fvOld = m_PreviewOCX.GetFileVersion(szFileName,"FileVersion");
	var fvNew = xmlDoc.documentElement.getElementsByTagName(szFileName)[0].childNodes[0].nodeValue;
	
	if(szFileName == "hpr.dll")
	{
		var sp = ".";
	}
	else
	{
		var sp = ",";
	}
	var fvSigleOld = fvOld.split(sp);
	var fvSigleNew = fvNew.split(sp);
	
	for(var i = 0;i < 4;i++)
	{
		if(parseInt(fvSigleOld[i]) > parseInt(fvSigleNew[i]))
		{
			return -1;
		}
		
		if(parseInt(fvSigleOld[i]) < parseInt(fvSigleNew[i]))
		{
			return 1;
		}
	}
	return 0;
}

/*************************************************
Function:		getXMLHttpRequest
Description:	创建xmlhttprequest对象
Input:			无			
Output:			无
return:			无				
*************************************************/
function getXMLHttpRequest()    
{
        var xmlHttpRequest = null; 
        if (window.XMLHttpRequest) 
        {
            xmlHttpRequest = new XMLHttpRequest();
        }
        else if (window.ActiveXObject)
        {
        	xmlHttpRequest = new ActiveXObject("Microsoft.XMLHTTP");
        } 
     	return xmlHttpRequest;
} 
/*************************************************
Function:		createxmlDoc
Description:	创建xml DOM对象
Input:			无			
Output:			无
return:			无				
*************************************************/
function createxmlDoc()
{
	var xmlDoc;
	var aVersions = [ "MSXML2.DOMDocument","MSXML2.DOMDocument.5.0",
	"MSXML2.DOMDocument.4.0","MSXML2.DOMDocument.3.0",
	"Microsoft.XmlDom"];
	
	for (var i = 0; i < aVersions.length; i++) 
	{
		try 
		{
			xmlDoc = new ActiveXObject(aVersions[i]);
			break;
		}
		catch (oError)
		{
			xmlDoc = document.implementation.createDocument("", "", null);
			break;
		}
	}
	xmlDoc.async="false";
	return xmlDoc;
}

/*************************************************
Function:		GoAway
Description:	注销用户
Input:			无
Output:			无
return:			无
*************************************************/
function GoAway()
{
	Warning = confirm(ContentFrame.window.m_szExit);
	if(Warning)
	{
		$.cookie('userInfo'+m_lHttpPort,null);
		$.cookie('page',null);
		window.location.href="login.asp";
	}
}
/*************************************************
Function:		get_nextsibling
Description:	获取节点的下一个子节点
Input:			无
Output:			无
return:			无
*************************************************/
function get_nextsibling(n)
{
	var x=n.nextSibling;
	while (x.nodeType!=1)
	{
	    x=x.nextSibling;
	}
	return x;
}
/*************************************************
Function:		get_previoussibling
Description:	获取节点的上一个子节点
Input:			无
Output:			无
return:			无
*************************************************/
function get_previoussibling(n)
{
    var x = n.previousSibling;
    while (x.nodeType!=1)
    {
        x = x.previousSibling;
    }
    return x;
}

/*************************************************
Function:		browseFilePath
Description:	浏览系统文件夹路径
Input:			szId:文本框ID, iSelectMode 打开模式		
Output:			无
return:			无				
*************************************************/
function browseFilePath(szId, iSelectMode)
{
	if(m_PreviewOCX != null)
	{
		var szPost= m_PreviewOCX.HWP_OpenFileBrowser(iSelectMode,"");
		if(szPost == "" || szPost == null)
		{
			return;
		}
		else
		{
			if(iSelectMode == 1)
			{
				if(szPost.length > 100)
				{
					alert(getNodeValue('tipsTooLong'));
					return;
				}
			}
			else
			{
				if(szPost.length > 130)
				{
					alert(getNodeValue('tipsTooLong'));
					return;
				}
			}
		
			document.getElementById(szId).value= szPost;
		}
	}
}

/*************************************************
Function:		getObjLeft
Description:	获取对象相对网页的左上角坐标
Input:			obj 对象
Output:			无
return:			坐标
*************************************************/
function getObjLeft(obj)   
{   
	var x = obj.offsetLeft;   
	while(obj=obj.offsetParent) 
	{ 
		x += obj.offsetLeft;
	} 
	return x;   
}

/*************************************************
Function:		getObjTop
Description:	获取对象相对网页的左上角坐标
Input:			obj 对象
Output:			无
return:			坐标
*************************************************/
function getObjTop(obj)   
{   
	var y = obj.offsetTop;   
	while(obj=obj.offsetParent) 
	{ 
		y += obj.offsetTop;
	} 
	return y;   
}

/*************************************************
Function:		CreateCalendar
Description:	创建日历
Input:			iType: 0 日志界面日历 1 时间配置界面日历 2 假日配置界面日历
Output:			无
return:			无
*************************************************/
function CreateCalendar(iType)
{
	var szLanguage;
	if(m_szLanguage == 'cn')
	{
		szLanguage = 'zh-cn';
	}
	else
	{
		szLanguage = 'en';
	}
	if(iType == 0)
	{
	    WdatePicker({startDate:'%y-%M-%d %h:%m:%s',dateFmt:'yyyy-MM-dd HH:mm:ss',alwaysUseStartDate:false,minDate:'1970-01-01 00:00:00',maxDate:'2037-12-31 23:59:59',readOnly:true,lang:szLanguage});
	}
	else if(iType == 1)
	{
		WdatePicker({startDate:'%y-%M-%d %h:%m:%s',dateFmt:'yyyy-MM-ddTHH:mm:ss',alwaysUseStartDate:false,minDate:'1970-01-01 00:00:00',maxDate:'2037-12-31 23:59:59',readOnly:true,lang:szLanguage});
	}
	else
	{
		WdatePicker({startDate:'%y-%M-%d',dateFmt:'yyyy-MM-dd',alwaysUseStartDate:true,minDate:'1970-01-01 00:00:00',maxDate:'2037-12-31 23:59:59',readOnly:true,lang:szLanguage});
	}
}
/*************************************************
Function:		getXMLHandler
Description:	读取xml文件，返回xmlDoc对象
Input:			xmlFile：xml文件名(路径)
				language:语言
Output:			无
return:			xmlDoc对象
*************************************************/
function getXMLHandler(xmlFile,language)
{
	try
	{
		m_xmlDoc = parseXml(xmlFile);
		var szOptionInfo="";
		var oSelectObj = window.parent.document.getElementById('LanguageSelect');
		var iLength = oSelectObj.options.length;
		if(iLength <= 0)
		{
			for(var i=0;i<m_xmlDoc.getElementsByTagName("Resources").length;i++)
			{
				szOptionInfo+="<option value ='"+m_xmlDoc.getElementsByTagName("Resources")[i].getAttribute("lan")+"'>"+m_xmlDoc.getElementsByTagName("Resources")[i].getAttribute("name") + "</option>";
			}
			$(szOptionInfo).appendTo("#LanguageSelect");
			setTimeout(function(){$("#LanguageSelect").attr("value",language);},1);
		}
		else
		{
			for(i = 0; i < iLength; i++)
			{
				oSelectObj.options[i].innerHTML = m_xmlDoc.getElementsByTagName("Resources")[i].getAttribute("name");
			}
			//$("#LanguageSelect").attr("value",language);
		}
		
		for(var i=0;i<m_xmlDoc.getElementsByTagName("Resources").length;i++)
		{
			if(language==m_xmlDoc.getElementsByTagName("Resources")[i].getAttribute("lan"))
			{
				break;
			}
		}
		m_xmlDoc=m_xmlDoc.getElementsByTagName("Resources")[i];
	}
	catch(e)
	{
		//alert(e.message)
	}
	return m_xmlDoc;
}
/*************************************************
Function:		getNodeValue
Description:	得到节点值
Input:			tagName:元素名
Output:			无
return:			无
*************************************************/
function getNodeValue(tagName)
{
	if(m_xmlDoc == null)
	{
		return;
	}
	if(m_xmlDoc.getElementsByTagName(tagName)!=null)
	try
	{
		return m_xmlDoc.getElementsByTagName(tagName)[0].childNodes[0].nodeValue;
	}
	catch(e)
	{
		return "";
	}
}
/*************************************************
Function:		TranslateElements
Description:	转变元素语言
Input:			targetDocument:文档
				tag:标签名
				propertyToSet:方式
Output:			无
return:			无
*************************************************/
function TranslateElements(targetDocument,tag,propertyToSet)
{
	var e=targetDocument.getElementsByTagName(tag);
	for(var i=0;i<e.length;i++)
	{
		var sKey=e[i].getAttribute('id');
		if(sKey)
		{
			var s=getNodeValue(sKey);
			if(s && tag == 'LABEL')
			{
			   s = s.replace(/ /g,"&nbsp;");
			}
			if(s)
			eval('e[i].'+propertyToSet+' = s');
		}
	}
}
/*************************************************
Function:		TranslatePage
Description:	转换界面中的语言
Input:
Output:			无
return:			无
*************************************************/
function TranslatePage()
{
	TranslateElements( document, 'INPUT', 'value' ) ;
	//this.TranslateElements( targetDocument, 'SPAN', 'innerHTML' ) ;
	TranslateElements(document,'LABEL','innerHTML');
	TranslateElements(document,'img','title');
	TranslateElements(document, 'OPTION', 'text' );
}
/*************************************************
Function:		getConfigXMLHandler
Description:	读取xml文件，返回xmlDoc对象
Input:			xmlFile：xml文件名(路径)
				language:语言
Output:			无
return:			xmlDoc对象
*************************************************/
function getConfigXMLHandler(xmlFile,language)
{
	try
	{
		m_ConfigxmlDoc = parseXml(xmlFile);
		var szOptionInfo="";
	
		var oSelectObj = window.parent.document.getElementById('LanguageSelect');
		var iLength = oSelectObj.options.length;
		if(iLength <= 0)
		{
			for(var i=0;i<m_ConfigxmlDoc.getElementsByTagName("Resources").length;i++)
			{
				szOptionInfo+="<option value ='"+m_ConfigxmlDoc.getElementsByTagName("Resources")[i].getAttribute("lan")+"'>"+m_ConfigxmlDoc.getElementsByTagName("Resources")[i].getAttribute("name") + "</option>";
			}
			$(szOptionInfo).appendTo("#LanguageSelect");
			setTimeout(function(){$("#LanguageSelect").attr("value",language);},1);
		}
		else
		{
			for(i = 0; i < iLength; i++)
			{
				oSelectObj.options[i].innerHTML = m_ConfigxmlDoc.getElementsByTagName("Resources")[i].getAttribute("name");
			}
			//$("#LanguageSelect").attr("value",language);
		}
		
		for(var i=0;i<m_ConfigxmlDoc.getElementsByTagName("Resources").length;i++)
		{
			if(language==m_ConfigxmlDoc.getElementsByTagName("Resources")[i].getAttribute("lan"))
			{
				break;
			}
		}
		m_ConfigxmlDoc=m_ConfigxmlDoc.getElementsByTagName("Resources")[i];
	}
	catch(e)
	{
		//alert(e.message)
	}
	return m_ConfigxmlDoc;
}
/*************************************************
Function:		getConfigNodeValue
Description:	得到节点值
Input:			tagName:元素名
Output:			无
return:			无
*************************************************/
function getConfigNodeValue(tagName)
{
	if(m_ConfigxmlDoc == null)
	{
		return;
	}
	if(m_ConfigxmlDoc.getElementsByTagName(tagName)!=null)
	try
	{
		return m_ConfigxmlDoc.getElementsByTagName(tagName)[0].childNodes[0].nodeValue;
	}
	catch(e)
	{
		return "";
	}
}
/*************************************************
Function:		TranslateConfigElements
Description:	转变元素语言
Input:			targetDocument:文档
				tag:标签名
				propertyToSet:方式
Output:			无
return:			无
*************************************************/
function TranslateConfigElements(targetDocument,tag,propertyToSet)
{
	var e = targetDocument.getElementsByTagName(tag);
	for(var i = 0; i < e.length; i++)
	{
		var sKey = e[i].getAttribute('id');
		if(sKey)
		{
			var s = getConfigNodeValue(sKey);
			if(s && tag == 'LABEL')
			{
			   s = s.replace(/ /g,"&nbsp;");
			}
			if(s)
			eval('e[i].'+propertyToSet+' = s');
		}
	}
}
/*************************************************
Function:		TranslateConfigPage
Description:	转换界面中的语言
Input:
Output:			无
return:			无
*************************************************/
function TranslateConfigPage()
{
	TranslateConfigElements( document, 'INPUT', 'value' ) ;
	//this.TranslateElements( targetDocument, 'SPAN', 'innerHTML' ) ;
	TranslateConfigElements(document,'LABEL','innerHTML');
	TranslateConfigElements(document,'img','title');
	TranslateConfigElements(document, 'OPTION', 'text' );
}

/**********************************
Function:		DayAdd
Description:	日期加天数
Input:			szDay: 要加的日期
				iAdd： 加的天数
Output:			无
return:			true 有录像； false 没有录像；		
***********************************/
function DayAdd(szDay,iAdd)
{
	var date =  new Date(Date.parse(szDay.replace(/\-/g,'/')));
	var newdate = new Date(date.getTime()+(iAdd*24 * 60 * 60 * 1000));
	
	return newdate.Format("yyyy-MM-dd hh:mm:ss");   
}

// 对Date的扩展，将 Date 转化为指定格式的String
// 月(M)、日(d)、小时(h)、分(m)、秒(s)、季度(q) 可以用 1-2 个占位符，
// 年(y)可以用 1-4 个占位符，毫秒(S)只能用 1 个占位符(是 1-3 位的数字)
// 例子：
// (new Date()).Format("yyyy-MM-dd hh:mm:ss.S") ==> 2006-07-02 08:09:04.423
// (new Date()).Format("yyyy-M-d h:m:s.S")      ==> 2006-7-2 8:9:4.18
Date.prototype.Format = function (fmt)
{
	var o=
	{
		"M+":this.getMonth()+1,//月份
		"d+":this.getDate(),//日
		"h+":this.getHours(),//小时
		"m+":this.getMinutes(),//分
		"s+":this.getSeconds(),//秒
		"q+":Math.floor((this.getMonth()+3)/3),//季度
		"S":this.getMilliseconds()//毫秒
	};
	if(/(y+)/.test(fmt))
	fmt=fmt.replace(RegExp.$1,(this.getFullYear()+"").substr(4-RegExp.$1.length));
	for(var k in o)
	if(new RegExp("("+k+")").test(fmt))
	fmt=fmt.replace(RegExp.$1,(RegExp.$1.length==1)?(o[k]):(("00"+o[k]).substr((""+o[k]).length)));
	return fmt;
}

/*************************************************
Function:		GetRTSPPort
Description:	获取RTSP端口
Input:			无
Output:			无
return:			无
*************************************************/
function GetRTSPPort()
{
	szXmlhttp=new getXMLHttpRequest();
	var szURL=m_lHttp+m_szHostName+":"+m_lHttpPort+"/PSIA/Streaming/channels";
	szXmlhttp.open("GET",szURL,false);
	szXmlhttp.setRequestHeader("If-Modified-Since","0");
	szXmlhttp.setRequestHeader("Authorization","Basic "+m_szUserPwdValue);
	szXmlhttp.send(null);
	GetRTSPPortCallback();
}

function GetRTSPPortCallback()
{
	if(szXmlhttp.readyState==4)
	{
		if(szXmlhttp.status==200)
		{
			var xmlDoc=new createxmlDoc();
			xmlDoc = szXmlhttp.responseXML;
			
			try
			{
				m_lRtspPort = xmlDoc.documentElement.getElementsByTagName('rtspPortNo')[0].childNodes[0].nodeValue;
			}
			catch(e)
			{
				GetRTSPPortDyn(); 
			}
		}
		else
		{
			return ;
		}
	}
}

/*************************************************
Function:		GetRTSPPortDyn
Description:	获取RTSP端口
Input:			无
Output:			无
return:			无
*************************************************/
function GetRTSPPortDyn()
{
	szXmlhttp=new getXMLHttpRequest();
	var szURL=m_lHttp+m_szHostName+":"+m_lHttpPort+"/PSIA/Custom/SelfExt/ContentMgmt/DynStreaming/channels";
	szXmlhttp.open("GET",szURL,false);
	szXmlhttp.setRequestHeader("If-Modified-Since","0");
	szXmlhttp.setRequestHeader("Authorization","Basic "+m_szUserPwdValue);
	szXmlhttp.send(null);
	GetRTSPPortDynCallback();
}
function GetRTSPPortDynCallback()
{
	if(szXmlhttp.readyState==4)
	{
		if(szXmlhttp.status==200)
		{
			var xmlDoc=new createxmlDoc();
			xmlDoc = szXmlhttp.responseXML;
			
			try
			{
				m_lRtspPort = xmlDoc.documentElement.getElementsByTagName('rtspPortNo')[0].childNodes[0].nodeValue;
			}
			catch(e)
			{
				m_lRtspPort = "554";
			}
		}
		else
		{
			return ;
		}
	}
}

/*************************************************
Function:		GetChannelInfo
Description:	获取通道信息
Input:			无
Output:			无
return:			无
*************************************************/
function GetChannelInfo()
{
	szXmlhttp=new getXMLHttpRequest();
	var szURL=m_lHttp+m_szHostName+":"+m_lHttpPort+"/PSIA/System/Video/inputs/channels";
	szXmlhttp.open("GET",szURL,false);
	szXmlhttp.setRequestHeader("If-Modified-Since","0");
	szXmlhttp.setRequestHeader("Authorization","Basic "+m_szUserPwdValue);
	szXmlhttp.send(null);
	GetChannelInfoCallback();
}
function GetChannelInfoCallback()
{
	if(szXmlhttp.readyState==4)
	{
		if(szXmlhttp.status==200)
		{
			var xmlDoc = new createxmlDoc();
			xmlDoc = szXmlhttp.responseXML;
			m_oXmlDoc = xmlDoc;
			
			try
			{
				m_iAnalogChannelNum = xmlDoc.documentElement.getElementsByTagName('VideoInputChannel').length;
				for(var i = 0; i < m_iAnalogChannelNum; i++)
				{
					m_iChannelId[i] = parseInt(xmlDoc.documentElement.getElementsByTagName('VideoInputChannel')[i].getElementsByTagName('id')[0].childNodes[0].nodeValue);
				}
			}
			catch(e)
			{
				m_iAnalogChannelNum = 0;
			}
		}
	}
}

/*************************************************
Function:		GetDigChannelInfo
Description:	获取数字通道信息
Input:			无
Output:			无
return:			无
*************************************************/
function GetDigChannelInfo()
{
	szXmlhttp=new getXMLHttpRequest();
	var szURL=m_lHttp+m_szHostName+":"+m_lHttpPort+"/PSIA/Custom/SelfExt/ContentMgmt/DynVideo/inputs/channels";
	szXmlhttp.open("GET",szURL,false);
	szXmlhttp.setRequestHeader("If-Modified-Since","0");
	szXmlhttp.setRequestHeader("Authorization","Basic "+m_szUserPwdValue);
	szXmlhttp.send(null);
	GetDigChannelInfoCallback();
}

function GetDigChannelInfoCallback()
{
	if(szXmlhttp.readyState==4)
	{
		if(szXmlhttp.status==200)
		{
			var xmlDoc = new createxmlDoc();
			xmlDoc = szXmlhttp.responseXML;
			m_oDigXmlDoc = xmlDoc;
			
			
			try
			{
				m_iDigitalChannelNum = xmlDoc.documentElement.getElementsByTagName('DynVideoInputChannel').length;
				
				for(var i = 0; i < m_iDigitalChannelNum; i++)
				{
					var iChannelId = parseInt(xmlDoc.documentElement.getElementsByTagName('DynVideoInputChannel')[i].getElementsByTagName('id')[0].childNodes[0].nodeValue)
					m_iChannelId[m_iAnalogChannelNum + i] = iChannelId;
				}
			}
			catch(e)
			{
				m_iDigitalChannelNum = 0;
			}
		}
		else
		{
			
		}
	}
}

/*************************************************
Function:		GetZeroChannelInfo
Description:	获取零通道信息
Input:			无
Output:			无
return:			无
*************************************************/
function GetZeroChannelInfo()
{
	szXmlhttp=new getXMLHttpRequest();
	var szURL=m_lHttp+m_szHostName+":"+m_lHttpPort+"/PSIA/Custom/SelfExt/ContentMgmt/ZeroVideo/channels";
	szXmlhttp.open("GET",szURL,false);
	szXmlhttp.setRequestHeader("If-Modified-Since","0");
	szXmlhttp.setRequestHeader("Authorization","Basic "+m_szUserPwdValue);
	szXmlhttp.send(null);
	GetZeroChannelInfoCallback();
}

function GetZeroChannelInfoCallback()
{
	if(szXmlhttp.readyState==4)
	{
		if(szXmlhttp.status==200)
		{
			var xmlDoc = new createxmlDoc();
			xmlDoc = szXmlhttp.responseXML;
			m_oZeroXmlDoc = xmlDoc;
			
			try
			{
				var iZeroLen = xmlDoc.documentElement.getElementsByTagName('ZeroVideoChannel').length;
				for(var i = 0; i < iZeroLen; i++)
				{
					if(xmlDoc.documentElement.getElementsByTagName('ZeroVideoChannel')[i].getElementsByTagName('enabled')[0].childNodes[0].nodeValue == 'true')
					{
						m_iChannelId[m_iAnalogChannelNum+m_iDigitalChannelNum+m_iZeroChanNum] = parseInt(xmlDoc.documentElement.getElementsByTagName('ZeroVideoChannel')[i].getElementsByTagName('id')[0].childNodes[0].nodeValue);
						m_iZeroChanNum++;
					}
				}
			}
			catch(e)
			{
				m_iZeroChanNum = 0;
			}
		}
	}
}

/*************************************************
Function:		UpdateTips
Description:	更新提示
Input:			无
Output:			无
return:			无
*************************************************/
function UpdateTips()
{
	var bUpdateTips = $.cookie('updateTips');
	var szUpdate = '';
	if(bUpdateTips == 'true')
	{
		if(navigator.platform == "Win32")
		{
			szUpdate = getNodeValue('jsUpdatePlugin');
			Warning =confirm(szUpdate);
			if (Warning)
			{
				window.open("../../codebase/WebComponents.exe","_self");
			}
			else
			{
				$.cookie('updateTips', 'false');
			}
		}
		else
		{
			szUpdate = getNodeValue('jsUpdateNotWin32');
			setTimeout(function() {alert(szUpdate);},20);
			$.cookie('updateTips', 'false');
		}
	}
}
/*************************************************
Function:		isIPv6Add
Description:	校验是否为有效的IPV6地址
Input:			strInfo:IPV6地址
Output:			true:是 false:否
return:			无
*************************************************/
function  isIPv6Add(strInfo)
{
	  return /:/.test(strInfo) && strInfo.match(/:/g).length<8 && /::/.test(strInfo)?(strInfo.match(/::/g).length==1 && /^::$|^(::)?([\da-f]{1,4}(:|::))*[\da-f]{1,4}(:|::)?$/i.test(strInfo)):/^([\da-f]{1,4}:){7}[\da-f]{1,4}$/i.test(strInfo);
}
/*************************************************
Function:		GetTalkNum
Description:	获取语音对讲通道数
Input:			无
Output:			无
return:			无
*************************************************/
function GetTalkNum()
{
	szXmlhttp=new getXMLHttpRequest();
	var szURL=m_lHttp+m_szHostName+":"+m_lHttpPort+"/PSIA/Custom/SelfExt/TwoWayAudio/channels";
	szXmlhttp.open("GET",szURL,false);
	szXmlhttp.setRequestHeader("If-Modified-Since","0");
	szXmlhttp.setRequestHeader("Authorization","Basic "+m_szUserPwdValue);
	szXmlhttp.send(null);
	GetTalkNumCallback();
}
function GetTalkNumCallback()
{
	if(szXmlhttp.readyState==4)
	{
		if(szXmlhttp.status==200)
		{
			var xmlDoc = new createxmlDoc();
			xmlDoc = szXmlhttp.responseXML;
			m_iTalkNum = xmlDoc.documentElement.getElementsByTagName('TwoWayAudioChannel').length;
			if(m_iTalkNum > 0)
			{
			    m_szaudioCompressionType = xmlDoc.documentElement.getElementsByTagName('audioCompressionType')[0].childNodes[0].nodeValue;
			}
		}
		else
		{
			m_iTalkNum = 0;
		}
	}
}
/*************************************************
Function:		Talk
Description:	语言对讲
Input:			无			
Output:			无
return:			无				
*************************************************/
function Talk(obj)
{
	GetTalkNum();	
	if(m_iTalkNum == 0)
	{
		return;
	}
		
   	m_PreviewOCX=document.getElementById("PreviewActiveX");
	
   	if(m_bTalk == 0)
   	{
       	if(m_iTalkNum <= 1)
	   	{
	       	var szOpenURL = m_lHttp + m_szHostName + ":" + m_lHttpPort + "/PSIA/Custom/SelfExt/TwoWayAudio/channels/1/open";
			var szCloseURL = m_lHttp + m_szHostName + ":" + m_lHttpPort + "/PSIA/Custom/SelfExt/TwoWayAudio/channels/1/close";
			var szDataUrl = m_lHttp + m_szHostName + ":" + m_lHttpPort + "/PSIA/Custom/SelfExt/TwoWayAudio/channels/1/audioData";
			var iAudioType = 1;
			if(m_szaudioCompressionType == 'G.711ulaw')
			{
				iAudioType = 1;
			}
			else
			{
				iAudioType = 0;
			}
			var iTalk = m_PreviewOCX.HWP_StartVoiceTalk(szOpenURL, szCloseURL, szDataUrl, m_szUserPwdValue, parseInt(iAudioType));
			if(iTalk == 0)
			{
				document.getElementById("voiceTalk").src="../images/public/ICON/speak_sound_normal.png";
				document.getElementById("voiceTalk").title = getNodeValue('StopvoiceTalk');
				m_bTalk =1;
			}
			else
		    {
				alert(getNodeValue('VoiceTalkFailed'));
				return ;
			}
	   	}
	   	else
	   	{
			$('#EditVoiceTalk').css('right', '2px');
			$('#EditVoiceTalk').css('top', $(obj).offset().top - $('#EditPatrolPreset').height() + 5);
			$('#EditVoiceTalk').modal();
	   	}
	}
	else 
	{
	 	document.getElementById("voiceTalk").src="../images/public/ICON/speak_normal.png";
		document.getElementById("voiceTalk").title = getNodeValue('voiceTalk');
	  	m_PreviewOCX.HWP_StopVoiceTalk();
      	m_bTalk =0;
	 }
}
/*************************************************add: 2009-03-20
Function:		SelectAllFile
Description:	选中语音通道
Input:			num : 通道序号			
Output:			无
return:			无				
*************************************************/
function SelectAllFile(num)
{
	if(document.getElementById("Num"+num).checked == false)
	{
		 m_iTalkingNO = 0;
		 return;
	}
	for(var i=1;i < 3; i ++)
	{  
		if(i == num)
		{
		   document.getElementById("Num"+i).checked = true;
		}
		else
		{
		   document.getElementById("Num"+i).checked = false;
		}
	}
	m_iTalkingNO = num;
}
/*************************************************
Function:		onVoiceTalkDlgOk
Description:	确定选择进行对讲
Input:			无			
Output:			无
return:			无				
*************************************************/ 
function onVoiceTalkDlgOk()
{
    if(m_iTalkingNO == 0)
	{
	    alert(getNodeValue('ChooseTalkChan'));
		return;
	}
	var PlayOCX = document.getElementById("PreviewActiveX");
	var szOpenURL = m_lHttp + m_szHostName + ":" + m_lHttpPort + "/PSIA/Custom/SelfExt/TwoWayAudio/channels/" + m_iTalkingNO + "/open";
	var szCloseURL = m_lHttp + m_szHostName + ":" + m_lHttpPort + "/PSIA/Custom/SelfExt/TwoWayAudio/channels/" + m_iTalkingNO + "/close";
	var szDataUrl = m_lHttp + m_szHostName + ":" + m_lHttpPort + "/PSIA/Custom/SelfExt/TwoWayAudio/channels/" + m_iTalkingNO + "/audioData";
	var iAudioType = 1;
	if(m_szaudioCompressionType == 'G.711ulaw')
	{
		iAudioType = 1;
	}
	else
	{
		iAudioType = 0;
	}
	var iTalk = PlayOCX.HWP_StartVoiceTalk(szOpenURL, szCloseURL, szDataUrl, m_szUserPwdValue, parseInt(iAudioType));
	if(iTalk == 0)
	{
		document.getElementById("voiceTalk").src="../images/public/ICON/speak_sound_normal.png";
		document.getElementById("voiceTalk").alt = getNodeValue('StopvoiceTalk');
		m_bTalk =1;
	}
	else
	{
		alert(getNodeValue('VoiceTalkFailed'));
		return ;
	}
	$.modal.impl.close();
}
//add by tangzz 2011-12-29
/*************************************************
Function:		GetTreeTable
Description:	获取左边树table(预览/回放)
Input:			无			
Output:			无
return:			无				
*************************************************/ 
function GetTreeTable()
{
	var innerHTML = document.getElementById("content_left").innerHTML;
	innerHTML = innerHTML + "<table cellspacing='0' cellpadding='0' style='width:200px; height:100%; border-collapse:collapse; border:1px solid #9b9b9b;'><tr style='width:200px;height:8px;'><td height='9' style='width:8px;height:8px;'></td><td style='width:184px;height:8px; '></td><td style='width:8px;height:8px; '></td></tr><tr style='width:200px;height:30px;'><td style='width:8px;height:30px;'></td><td style='width:184px;height:30px;'><div id='Device' class='ellipsis'>&nbsp; <img src='../images/public/ICON/DVR.png' /> <span id='DeviceName' style='-moz-user-select:none;' onselectstart='return false;'></span></div></td><td style='width:8px;height:30px; '></td></tr><tr style='width:200px;height:auto;'><td style='width:8px;height:auto; '></td><td valign='top' style='width:184px;height:auto;background:#dbdbdb;'><div id='sub_menu'></div></td><td style='width:8px;height:auto;'></td></tr><tr style='width:200px;height:8px;'><td style='width:8px;height:8px; '></td><td style='width:184px;height:8px;'></td><td style='width:8px;height:8px;'></td></tr></table>";
    $("#content_left").html(innerHTML);	
}

