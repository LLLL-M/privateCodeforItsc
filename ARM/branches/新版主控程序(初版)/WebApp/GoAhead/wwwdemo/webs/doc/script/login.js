var m_szHostName="";
var	m_lHttpPort = "8080";
var m_szUserPwdValue = "";
var m_lHttp="";
var m_szLanguage = "cn";
var m_bConnectTimeOut = false;
var m_hConnectClockTime = null;         //连接超时定时器句柄
/**********************************
功能: 初始化界面
***********************************/
function InitLogin()
{	
	if(!(document.cookie || navigator.cookieEnabled))
    {
		alert(getNodeValue('CookieTips'));
		return;
	}
	m_szLanguage = $.cookie('language');
	if(m_szLanguage == null)				//如果直接到登录界面，也获取一下语言
	{
		if (navigator.appName == "Netscape" || navigator.appName == "Opera")
		{
			var sysLanguage= navigator.language.toLowerCase();
		}
		else
		{
			var sysLanguage= navigator.browserLanguage.toLowerCase();
		}
		if(sysLanguage == "zh-cn" || sysLanguage == "zh-tw") 
		{
			m_szLanguage = "cn";
			$.cookie('language', 'cn');	
		}
		else
		{ 
			m_szLanguage = "en";
			$.cookie('language', 'en');
		}
	}
	
	ChangeLanguage('../xml/login.xml',m_szLanguage);

	document.getElementById('UserName').focus();
}
/**********************************
功能: 按回车键登录
***********************************/
document.onkeydown=function (event) 
{
	event = event?event:(window.event?window.event:null);	 
	 if(event.keyCode==13)
	{
		submitForm(); 
	}
}
/**********************************
功能: 计算字符串的长度
参数: szString: 输入的字符串
***********************************/   
function JudgeTextLength(szString)
{
	var  iLength = 0;   
   	for(var i=0; i<szString.length; i++)   
   	{   
		if(szString.charCodeAt(i)>255)   
	  	{   
			iLength+=2;   
	  	}   
	  	else   
	  	{   
	    	iLength+=1;   
	  	}        
   	}   
   	return  iLength;    
}
/**********************************
功能: 登陆
***********************************/
function DoLogin()                            
{ 
	$("#LoginBtn").focus();
	m_szAltTips1 = getNodeValue('LoginTips1');  
	m_szAltTips2 = getNodeValue('LoginTips2');  
	m_szAltTips3 = getNodeValue('LoginTips3');

    //用户名为空时提示  
	if(document.getElementById('UserName').value.length==0)     
	{ 
		document.getElementById('UserName').focus();    
		alert(m_szAltTips1); 
		return false
	} 
	if(JudgeTextLength(document.getElementById('UserName').value) > 16)
  	{
	  	document.getElementById('UserName').focus();
		document.getElementById('UserName').value = "";  
		alert(m_szAltTips2);
      	return false;
  	} 	
  	if(JudgeTextLength(document.getElementById('Password1').value) > 16)
  	{
	  	document.getElementById('Password1').focus(); 
		document.getElementById('Password1').value = "";  
		alert(m_szAltTips3);
      	return false;
  	} 
	
	m_szUserPwdValue = Base64.encode(document.getElementById('UserName').value + ":" + document.getElementById('Password1').value);
	szXmlhttp = getXMLHttpRequest();   
	szXmlhttp.onreadystatechange = LoginCallback;  
	var szURL=m_lHttp+m_szHostName+":"+m_lHttpPort+"/PSIA/Custom/HIK/userCheck";
	szXmlhttp.open("GET", szURL, true); 
	szXmlhttp.setRequestHeader("If-Modified-Since","0");   
	szXmlhttp.setRequestHeader("Authorization",  "Basic " + m_szUserPwdValue);
	szXmlhttp.send(null); 
	m_hConnectClockTime = setTimeout("TimeOut()", 15000);
}

function  LoginCallback()
{   
	if(szXmlhttp.readyState == 4)
	{   
		clearTimeout(m_hConnectClockTime);
		if(szXmlhttp.status == 200)
		{
			var xmlDoc = parseXmlFromStr(szXmlhttp.responseText);
		   
			if("200" == xmlDoc.documentElement.getElementsByTagName('statusValue')[0].childNodes[0].nodeValue)
			{
				$.cookie('page',null);
				$.cookie('userInfo'+m_lHttpPort,m_szUserPwdValue);
				window.location.href = "main.asp"; 
			}
			else
			{   
				document.getElementsByName('UserName')[0].focus();
				document.getElementsByName('UserName')[0].value = "";
				document.getElementsByName('Password1')[0].value = "";
				alert(getNodeValue('LoginTips4'));
			}	
		}   
		else
		{ 
			if(!m_bConnectTimeOut)
			{
				alert(getNodeValue('NetworkErrorTips'));
			}
		} 
		m_bConnectTimeOut = false;
	}    	
} 

/*************************************************
Function:		timeOut
Description:	连接超时
Input:			无
Output:			无
return:			无				
*************************************************/
function TimeOut()
{
	m_bConnectTimeOut = true;
	alert(getNodeValue('ConnectTimeoutTips'));
}

/*************************************************
Function:		ChangeLanguage
Description:	改变页面语言
Input:			xmlFile：xml文件
				lan：语言
Output:			无
return:			无				
*************************************************/
function ChangeLanguage(xmlFile,lan)
{
	m_szLanguage = lan;
	$.cookie('language', lan);	
	getXMLHandler(xmlFile,lan);	//加载语言xml
	
	TranslatePage();	//翻译
	
	window.parent.document.title = getNodeValue('title');
}
/*************************************************
Function:		CheckKeyDown
Description:	输入时按下空格时，不允许输入
Input:			iSetId: 需要验证表单Id	
				iSetValue: 需要验证的值	
Output:			无
return:			无				
*************************************************/
function CheckKeyDown(event)
{
	event = event?event:(window.event?window.event:null);
	if(event.keyCode == 32)   
    {
    	if(navigator.appName == "Netscape" || navigator.appName == "Opera")
		{
			event.preventDefault();
		}
		else
		{
		    event.returnValue = false;    //非ie浏览器event无returnValue属性
		}      
		return;
     }
}

/***************************************************
Function : 检查用户名，密码
***************************************************/


function buildXmlHttp(){
	var xmlHttp=null;
	if (window.XMLHttpRequest)
	{// code for IE7, Firefox, Opera, etc.
		xmlHttp=new XMLHttpRequest();
	}
	else if (window.ActiveXObject)
	{// code for IE6, IE5
		xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
	}
		return xmlHttp;
}
		
function log(content){
	if(window.console && window.console.log){
	window.console.log(content);
	}
}
		
function submitForm(){
	var xmlHttp = buildXmlHttp();
	if (xmlHttp!=null)
	{
		var UserName=document.getElementById("UserName").value;  			   
		var PassWord=document.getElementById("Password1").value;
			 
		log('UserName='+UserName+', Password='+PassWord);

		txt="<?xml version='1.0' encoding='utf-8'?>"
		txt=txt+"<Login>";
		txt=txt+"<username>"+UserName+"</username>";
		txt=txt+"<password>"+PassWord+"</password>";
		txt=txt+"</Login>";
			
		xmlHttp.open("POST", m_lHttp+m_szHostName+":"+m_lHttpPort+"/goform/loginTest", false);
		xmlHttp.send(txt);
		if (window.DOMParser)
		{
			parser=new DOMParser();
			xmlDoc=parser.parseFromString(xmlHttp.responseText,"text/xml");
		}
		else // Internet Explorer
		{
			xmlDoc=new ActiveXObject("Microsoft.XMLDOM");
			xmlDoc.async="false";
			xmlDoc.loadXML(xmlHttp.responseText);
		}
			log(xmlHttp.responseText);
			if(xmlDoc.getElementsByTagName("loginresult")[0].childNodes[0].nodeValue == 1)
			{	
				document.write("<br>");
				// alert("登陆成功");
				window.location.href="paramconfig.asp";
				document.write("<br>");
			}
			else if(xmlDoc.getElementsByTagName("loginresult")[0].childNodes[0].nodeValue == 0)
			{
				document.write("<br>");
				alert("登陆异常");
				window.location.href="login.asp";
				document.write("<br>");
			}
			else if(xmlDoc.getElementsByTagName("loginresult")[0].childNodes[0].nodeValue == -1)
			{
				document.write("<br>");
				alert("用户名错误");
				window.location.href="login.asp";
				document.write("<br>");
			}
			else if(xmlDoc.getElementsByTagName("loginresult")[0].childNodes[0].nodeValue == -2)
			{
				document.write("<br>");
				alert("密码错误");
				window.location.href="login.asp";
				document.write("<br>");
			}
		}
	else
	{
		alert("Your browser does not support XMLHTTP.");
	}
}