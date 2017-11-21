<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title></title>
<script type="text/javascript" src="doc/script/jquery-1.5.min.js"></script>
<script type="text/javascript" src="doc/script/jquery.cookie.js"></script>
<script src="doc/script/common.js"></script>
<script type="text/javascript">
function init()
{
	var szLanguage = $.cookie('language');
	if(szLanguage == null)				//如果直接到登录界面，也获取一下语言
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
			$.cookie('language', 'cn');	
		}
		else
		{
			$.cookie('language', 'en');
		}
	}
	
	var szUrl = decodeURI(document.URL);
	if(szUrl.indexOf("user=")!=-1 && szUrl.indexOf("&pass=")!=-1 && szUrl.indexOf("&page=")!=-1)
	{
		var szUser = szUrl.substring(szUrl.indexOf("user=") + 5, szUrl.indexOf("&pass="));
		var szPass = szUrl.substring(szUrl.indexOf("pass=") + 5, szUrl.indexOf("&page="));
	
		m_szUserPwdValue = Base64.encode(szUser + ":" + szPass);
		
		szXmlhttp = getXMLHttpRequest();
		var szURL=m_lHttp+m_szHostName+":"+m_lHttpPort+"/PSIA/Custom/SelfExt/userCheck";
		szXmlhttp.open("GET", szURL, false); 
		szXmlhttp.setRequestHeader("If-Modified-Since","0");   
		szXmlhttp.setRequestHeader("Authorization",  "Basic " + m_szUserPwdValue);
		szXmlhttp.send("");
		LoginCallback(); 
	}
	else
	{
		window.location.href = "doc/page/login.asp";
	}
}
function  LoginCallback()
{   
	if(szXmlhttp.readyState == 4)
	{
		if(szXmlhttp.status == 200)
		{
			var xmlDoc = parseXmlFromStr(szXmlhttp.responseText);
		   
			if("200" == xmlDoc.documentElement.getElementsByTagName('statusValue')[0].childNodes[0].nodeValue)
			{
				var szUrl = decodeURI(document.URL);
				var szPage = szUrl.substring(szUrl.indexOf("&page=") + 6, szUrl.indexOf("[&"));
				if(szPage.indexOf(".asp") == -1)
				{
					szPage = szPage.concat(".asp");
				}
				var szParam = szUrl.substring(szUrl.indexOf("[&") + 2, szUrl.length - 1);
				$.cookie('page',szPage+"?"+szParam+"%1");
				$.cookie('userInfo'+m_lHttpPort,m_szUserPwdValue);
				window.location.href = "doc/page/main.asp";
			}
			else
			{   
				window.location.href = "doc/page/login.asp"; 
			}	
		}   
		else
		{ 
			
		} 
	}    	
} 
</script>
</head>

<body onLoad="init()">
</body>
</html>
