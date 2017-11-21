<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>index</title>
<script type="text/javascript" src="doc/script/jquery.min.js"></script>
<script type="text/javascript" src="doc/script/jquery.cookie.js"></script>
<script language="JavaScript"> 
if (navigator.appName == 'Netscape' || navigator.appName == "Opera")
{
    var sysLanguage= navigator.language.toLowerCase();
}
else
{
    var sysLanguage= navigator.browserLanguage.toLowerCase();
}
self.moveTo(0,0);   //使其IE窗口最大化
self.resizeTo(screen.availWidth,screen.availHeight); 
if(sysLanguage == "zh-cn" || sysLanguage == "zh-tw") 
{
	$.cookie('language', 'cn');	
}
else
{ 
	$.cookie('language', 'en');
}
$.cookie('updateTips', 'true');

window.location.href = "doc/page/login.asp"; //如果是简体中文或繁体中文
</script> 
</head>
<body>
<table cellspacing="0" cellpadding="0">
  <tr>
    <td></td>
  </tr>
</table>
</body>
</html>