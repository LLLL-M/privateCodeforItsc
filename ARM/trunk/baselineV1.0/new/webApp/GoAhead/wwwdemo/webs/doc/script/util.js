
top._device_type_ = null;

if(!top._device_type_obj_){
	top._device_type_obj_ = {};
}

function getProductType(){
	if(typeof(window.isValidPwdFromPlatform) != 'undefined' && window.isValidPwdFromPlatform==false){
		log("XXX");
		return "XXX";	
	}
	if(top._device_type_){
		var t = top._device_type_;
		if(t.indexOf("VAR") >= 0){
			return "VAR";
		}else if(t.indexOf("TS") >= 0){
			return "TS";
		}	
	}
	var len = top.$('#main_type').length;
	if(len > 0){
		var t = top.$('#main_type').text();
		if(t.indexOf("VAR") >= 0){
			return "VAR";
		}else if(t.indexOf("TS") >= 0){
			return "TS";
		}	
	}
	
	//log(" 1111 ");
	$.ajax({
		url: m_lHttp+m_szHostName+":"+m_lHttpPort+"/PSIA/System/deviceInfo",
		type:"get",   
		async: false, 
		success: function(msg){
			var obj = $.xml2json(msg);
			top._device_type_ = obj.model;
			log(" 2222 devType="+top._device_type_)
		}
	});
	
	
	var t = top._device_type_;
	
	log(" 3333 t="+t);
	if(t){
		if(t.indexOf("VAR") >= 0){
			return "VAR";
		}else if(t.indexOf("TS") >= 0){
			return "TS";
		}		
	}
	return "VAR";
}

function log(content){
	try{
  		console.log(content);	
   	}catch(e){}
}
(function($){
	function createxmlDoc(){
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

	function xml2Str(Xml){
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

	var defaultParams = {
		root: 'Root',
		listNameIfArray: 'list'
	};
	$.json2xml = function(obj, params){
		var defPara = $.extend({}, defaultParams);
		if(params){
			defPara = $.extend({}, defPara, params);
		}
		var xmlDoc = createxmlDoc();
		
		var eles;
		if($.isArray(obj)){
			var o = $.extend(true, {}, obj);
			var o1 = {};
			o1[listNameIfArray] = o;
			eles = buildXmlEles(o1, xmlDoc, defPara.root);

		}else if($.isFunction(obj)){
			throw new Error("fuction is not supported yet!");
		}else{
			eles = buildXmlEles(obj, xmlDoc, defPara.root);	
		}
		
		xmlDoc.appendChild(eles[0]);
		
		return xml2Str(xmlDoc);
	}

	function buildXmlEles(obj, xmlDoc, eleName){
		var result = [];
		if(obj == null || typeof(obj) == "undefined"){
			return result;
		}
		if($.isArray(obj)){
			for (var i = 0; i < obj.length; i++) {
				var es = buildXmlEles(obj[i], xmlDoc, eleName);
				$.merge(result, es);
			};
		}else if($.isPlainObject(obj)){
			var e = xmlDoc.createElement(eleName);
			for(var p in obj){
				var es = buildXmlEles(obj[p], xmlDoc, p);
				for (var i = 0; i < es.length; i++) {
					e.appendChild(es[i]);
				};
			}
			result.push(e);
		}else if($.isFunction(obj)){

		}else{
			var e = xmlDoc.createElement(eleName);
			var t = xmlDoc.createTextNode(obj);
			e.appendChild(t);
			result.push(e);
		}
		return result;
	}
})($);


(function($){
	function getParams(search){
		var result = {};
		var s = $.trim(search);
		if(s == '' || s == '?'){
			return result;
		}
		s = s.replace('?', '');
		var arr = s.split('&');
		for(var i=0; i<arr.length; i++){
			var parr = arr[i].split('=');
			if(parr.length < 2){
				continue;
			}
			result[parr[0]] = parr[1];
		}
		return result;
	}

	$.getParams = getParams;
})($);

function log(content){
	try{
		console.log(content);	
	}catch(e){}
	
}


(function($){
	function stringifyJson(obj){
		//if(window.JSON && window.JSON.stringify){
		//	return window.JSON.stringify(obj);
		//}else 
		if(JSON2){
			return JSON2.stringify(obj);
		}else{
			throw "JSON stringify failed !";
		}
	}
	$.stringify = stringifyJson;
})($);

(function($){
	$.setData = function(name, data){
		if(!top._mo){
			top._mo = {};
		}
		top._mo[name] = data;
	};

	$.getData = function(name){
		if(!top._mo){
			return undefined;
		}
		return top._mo[name];
	};

	$.removeData = function(name){
		if(!top._mo){
			return true;
		}
		if(top._mo[name]){
			top._mo[name] = undefined;	
		}
		return true;
	}
})($);


(function(){
	function setField(id, value){
		var $es = $('#'+id);
		if($es.length == 0){
			return;
		}	
		var tagName = $es[0].tagName.toLowerCase();
		if(tagName == 'input'){
			var type = $es.attr('type');
			if ((type && type.toLowerCase() == 'checkbox') || (type && type.toLowerCase() == 'radio')){
				if ( parseInt(value) ) {
					$es.attr('checked', true);
				}else{
					$es.removeAttr('checked');
				}
			}else{
				$es.val(value);
			}
		}else if(tagName == 'td' || tagName=='span'){
			$es.text(value);
		}else if(tagName == 'select'){
			$es.find('option[value="'+value+'"]').attr('selected', 'selected');
		}else{
			log(' -- '+id+' is not found while setting Field !')
		}

	}

	function contains(arr, e){
		if(!$.isArray(arr)){
			arr = [arr];
		}
		for (var i = 0; i < arr.length; i++) {
			if(arr[i] == e){
				return true;
			}	
		};
		return false;
	}

	$.g = $.goldway || {};
	$.g.setField = setField;
	$.g.contains = contains;
})($);

function alertMsg(content, level){
	alert(content);
}

Date.prototype.format=function(f){
    if (!f){
        f="yyyyMMddHHmmss";
    }
    var o={
        "M+":this.getMonth()+1,
        "d+":this.getDate(),
        "H+":this.getHours(),
        "m+":this.getMinutes(),
        "s+":this.getSeconds()
    }
    if (/(y+)/.test(f)){
        f = f.replace(RegExp.$1, (this.getFullYear()+"").substr(4-RegExp.$1.length));
    }
    for(var k in o){
        if(new RegExp("("+k+")").test(f)){
            f = f.replace(RegExp.$1,
                 RegExp.$1.length == 1 ? o[k]:("00"+o[k]).substr((""+o[k]).length));
        }
    }
    return f;
}


function isHikUser(){
	var upe = Base64.encode("_hik_:12345");
	var cok = $.cookie('userInfo'+m_lHttpPort);
	if(cok == upe){
		return true;
	}
	return false;
}
