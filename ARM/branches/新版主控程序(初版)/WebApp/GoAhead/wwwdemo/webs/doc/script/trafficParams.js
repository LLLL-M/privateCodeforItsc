
var _pageOpt = {
	'unitParams': {
	'title': '单元参数',
	'error': null,
	'success': getUnitParamsInfo,
	'set': setUnitParamsInfo
	},
	'phaseTable': {
	'title': '相位表',
	'error': null,
	'success': getPhaseTableInfo,
	'set': setPhaseTableInfo
	},
	'ringAndPhase': {
	'title': '环并发相位',
	'error': null,
	'success': getRingAndPhaseInfo,
	'set': setRingAndPhaseInfo
	},
	'channelTable': {
	'title': '通道表',
	'error': null,
	'success': getChannelTableInfo,
	'set': setChannelTableInfo
	},	
	'splitTable': {
	'title': '绿信比',
	'error': null,
	'success': getGreenRatioInfo,
	'set': setGreenRatioInfo
	},
	'faultDetectionSet': {
	'title': '故障检测设置',
	'error': null,
	'success': getFaultDetectionInfo,
	'set': setFaultDetectionInfo
	},
	'sequenceTable': {
	'title': '相序表',
	'error': null,
	'success': getSequenceTableInfo,
	'set': setSequenceTableInfo
	},
	'programTable': {
	'title': '方案表',
	'error': null,
	'success': getProgramTableInfo,
	'set': setProgramTableInfo
	},
	'timeBasedActionTable': {
	'title': '时基动作表',
	'error': null,
	'success': getTimeBasedActionTableInfo,
	'set': setTimeBasedActionTableInfo
	},
	'timeInterval': {
	'title': '时段',
	'error': null,
	'success': getTimeIntervalInfo,
	'set': setTimeIntervalInfo
	},
	'scheduling': {
	'title': '调度计划',
	'error': null,
	'success': getSchedulingInfo,
	'set': setSchedulingInfo
	},
	'overlappingTable': {
	'title': '重叠表',
	'error': null,
	'success': getOverlappingTableInfo,
	'set': setOverlappingTableInfo
	},
	'coordinate': {
	'title': '协调',
	'error': null,
	'success': getCoordinateInfo,
	'set': setCoordinateInfo
	},
	'vehicleDetector': {
	'title': '车辆检测器',
	'error': null,
	'success': getVehicleDetectorInfo,
	'set': setVehicleDetectorInfo
	},
	'pedestrianDetector': {
	'title': '行人检测器',
	'error': null,
	'success': getPedestrianDetectorInfo,
	'set': setPedestrianDetectorInfo
	},
	'faultConfig': {
	'title': '故障配置',
	'error': null,
	'success': getFaultConfigInfo,
	'set': setFaultConfigInfo
	}
}

var _ErrorCode = {
	'1': '操作成功',
	'2': '设备忙',
	'3': '设备出错',
	'4': '错误的操作',
	'5': 'xml格式不正确',
	'6': 'xml内容不正确',
	'7': '需要重启设备',
	'8': '操作失败',
	'9': '视频分析失败',
	'10': '参数校验失败',
	'20': '密码错误',
	'21': '不能进行此操作',
	'22': '服务异常',
	'23': '权限低'
};
// var m_lHttpPort = "8080";
$.ajaxSetup({
	
	cache: false,
	beforeSend: function(XHR){
		XHR.setRequestHeader("If-Modified-Since","0");
		XHR.setRequestHeader("Authorization",  "Basic " + $.cookie('userInfo'+m_lHttpPort));
		XHR.setRequestHeader("Content-Type", "text/xml");
		//XHR.setRequestHeader("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>");
	},
	error: function(XMLHttpRequest, textStatus, errorThrown) {
		if(XMLHttpRequest.status >= 300
			&& XMLHttpRequest.status < 600){
			var xml = XMLHttpRequest.responseText;
			var chs = $.xml2json(xml);
			
			var str = "操作失败！";
			if(typeof chs['statusCode'] != 'undefined'){
				if(_ErrorCode[chs['statusCode']]){
					str += _ErrorCode[chs['statusCode']];
				}else if(chs['statusString']){
					str += chs['statusString'];
				}
			}	
			alert(str+xml);
		}
		//alert(XMLHttpRequest.status);
	}
});


function doTrafficParams (szMark) {
	try{
		$('#iColorPickerBg').click();
	}catch(e){}
	
	var menuName = szMark.substring(0, szMark.indexOf(":"));
	var o = _pageOpt[menuName];
	if(!o){
		log(menuName+' opt is empty!! return;');
		return false;
	}
	if(szMark.indexOf(':set') > 0){
		var sf = o['set'];
		if(sf){
			doSetParam(menuName, sf);
		}

	}else if(szMark.indexOf(":get") > 0){
		var menuName = szMark.replace(':get', '');
		doGetParam(menuName, o);
	}
	return true;
}

function doGetParam(menuName, option){
	if(!option){
		log('option is empty');
		return;
	}
	var err = option['error'];
	if(!err){
		err = function(){};
	}
	var suc = option['success'];
    var conTitle = option['title'];
    if(!conTitle){
    	conTitle = '配置';
    }
    
    //resetOnScene(false);
    paramOptBefore(menuName, option);

	$.ajax({
		url: "params/traffic/"+menuName+".asp",
		type:"post",        
		data: "iType=0",
		dataType:"html",
		error: err,
		success: function(msg){
			$("#center-tab").html(msg);
			$("#SpanAreaTitle").html(conTitle);
			initDictSelect();
			initClass();
			if(suc){
				suc();
			}
			var $v = $("form[traffic]").validate({
				meta:'validate',
				errorClass: 'error',
				validClass: 'valid'
			});
			option['validator'] = $v;
		}
	});
}

function doSetParam(menuName, optFunc){
	if(!$('form[traffic]').valid()){
		log('form is invalid !!')
		var o = _pageOpt[menuName];
		if(!o){
			return;
		}
		var $v = o['validator'];
		try{
			optFunc(false, $v);	
		}catch(e){}
	}else{
		try{
			optFunc(true);
		}catch(e){}
	}
	if(menuName == 'scene'){
		resetOnScene(true);
	}else{
		resetOnScene(false);
	}
}

function initClass(){
	var $f = $('form[traffic]');
	var $t = $f.find('table[traffic]');
	$t.addClass('edittableNo');
	$t.find('td').addClass('tdpaddingleft');
	$t.find('tr td:first-child').width(150);
	$t.find('tr td:nth-child(2)').find('*').width(330);


}

function paramOptBefore(paramName, option){
	$(document).unbind('click');
	if(paramName == 'scene'){
		resetOnScene(true);
	}else{
		resetOnScene(false);
	}

	var hideSaveBtn = option['hideSaveBtn'];
	if(hideSaveBtn == true){
		$("#SaveConfigBtn").hide();
	}else{
		$("#SaveConfigBtn").show();
	}
	var init = option['init'];
	if(init){
		init();
	}
}

function resetOnScene(isScene){
	$('#anotherBtn').unbind('click');
	if(isScene){
		$('#anotherBtn-div').show();
		$('#anotherBtn').val("抓图");
		$('#SpanAreaTitle').parents('tr:first').hide();
		$('#anotherBtn').click(snap);
	}else{
		$('#anotherBtn-div').hide();
		$('#SpanAreaTitle').parents('tr:first').show();
	}
}
/**
* 初始化form中与字典相关的select
*/
function initDictSelect(){
	$('select[dictKey]').each(function(){
		var dictKey = $(this).attr('dictKey');

		var dict = dictionary[dictKey];
		if(!dict){
			return;
		}
		$(this).remove('option');

		for(var d in dict){
			$(this).append('<option value="'+d+'">'+dict[d]+'</option>');
		}
	});
}


/**
* 通用方法
*/
/**
* 获取通道信息
**/
/*
function getTrafficChannelInfo(callback, callback2, options){
	log(' getTrafficChannelInfo ...')
	$.ajax({
		type: 'GET',
		dataType: 'xml',
		url:  m_lHttp+m_szHostName+":"+m_lHttpPort + '/PSIA/Custom/SelfExt/ContentMgmt/Traffic/channels',
		success: function(data, textStatus){
			if(callback){
				callback(data);
			}else{
				defaultCallback(data, callback2, options);
			}
		}
	});

	function defaultCallback(data, callback2, options){
		log(' defaultCallback ...')
		var chs = $.xml2json(data)['Channel'];		
		if(!$.isArray(chs)){
			chs = [chs];
		}	
		var chanSelId;
		if(options && options['chanSel']){
			chanSelId = options['chanSel'];
		}else{
			chanSelId = "channelSelect";
		} 
		var $camSel = $('#'+chanSelId);
		$camSel[0].options.length = 0;

		$camSel.data('chs', chs);

		for (var i = 0; i < chs.length; i++) {
			var ch = chs[i];
			if(!ch || !ch['id']){
				continue;
			}
			if(typeof(ch['deviceType']) == 'undefined' || typeof(ch['online']) == 'undefined'){
				continue;
			}
			if(options){
				var devT = options['deviceType'];
				if(devT){
					if(!$.isArray(devT)){
						devT = [devT];
					}
					var dt = parseInt(ch['deviceType']);
					if($.inArray(dt, devT) < 0){
						continue;
					}
				}
				var online = options['online'];
				if(typeof(online) != 'undefined' && online != null){
					if(online){
						if(ch['online'].toLowerCase() != 'true' && ch['online'] != '1'){
							continue;
						}	
					}
				}
				$camSel.append('<option value="'+ch['id']+'" >'+'IP通道'+' '+ch['id']+' ('+ch['ipAddr']+')'+'</option>');	
			}else{
				if(ch['deviceType'] == '2' && (ch['online'].toLowerCase() == 'true' || ch['online'] == '1')){
					$camSel.append('<option value="'+ch['id']+'" >'+'IP通道'+' '+ch['id']+' ('+ch['ipAddr']+')'+'</option>');	
				}	
			}			
		};

		var va = $('#'+chanSelId).val();
		if ( va != null && typeof(va) != 'undefined') {
			if(callback2){
				callback2();
			}
		};
	}
}
*/

//根据数组项得到其在数组中的序号
function GetArrayIndex(array,val)
{
	for(var i in array)
	{
		if(val == array[i])
		{
			return i;
		}
	}

	return 0;
}


function setFormDefault(data, exclude){
	for(var d in data){
		//alertMsg("content : " + d + " ==>  "+ data[d]);
		
		if($.g.contains(exclude, d)){
			continue;
		}
		$.g.setField(d, data[d]);
		//alertMsg("content : " + d + " ==>  "+ data[d]);
	}
	//alertMsg("asdf");
}

function optResultDefault(data){
	var d = $.xml2json(data);
	if(d['statusCode'] == 1){
		//alertMsg('操作成功！');//以后就自动保存表单，如果成功则不再显示
	}
	else if(_ErrorCode[d['statusCode']]){
		alertMsg(_ErrorCode[d['statusCode']]);
	}else if(d['statusString']){
		alertMsg("操作失败!"+d['statusString']);
	}else{
		alertMsg('操作失败！');
	}
}
/********************** 下面为具体页面的操作 *********************************/

//获取web库的版本号
function getLibsVersionInfo(){
		getlibsInfo(fillForm);
		
	function getlibsInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/libsInfo',
			
			success: function(data, textStatus){
				if(callback){
					callback(data);
				}
			}
		});
	}

	function fillForm(data){
		var d = $.xml2json(data);
		//alertMsg("1111");
		//if(d['StartFlashingYellowTime'] != 0)
		//{
			setFormDefault(d);	
		//}
			
	}

}

/**
* 获取单元参数基本信息
*/
function getUnitParamsInfo(){
		getUnitPInfo(fillForm);
		
	function getUnitPInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/unitParams',
			
			success: function(data, textStatus){
				if(callback){
					callback(data);
				}
			}
		});
	}

	function fillForm(data){
		var d = $.xml2json(data);
		//alertMsg(d['StartFlashingYellowTime']);
		//if(d['StartFlashingYellowTime'] != 0)
		{
			setFormDefault(d);	
		}
			
	}

}

/**
*设置单元参数基本信息
*/
function setUnitParamsInfo(formValid, $validator){
	if(formValid){
		var o = {};
		$.each(['StartFlashingYellowTime', 'StartAllRedTime', 'DegradationTime',
		 	    'SpeedFactor','MinimumRedLightTime', 'CommunicationTimeout', 'FlashingFrequency',
		 	    'TwiceCrossingTimeInterval','TwiceCrossingReverseTimeInterval',
		 	    'SmoothTransitionPeriod', 'FlowCollectionPeriod','CollectUnit'], function(index, ele){
			if($('#'+ele).length > 0){
				o[ele] = $('#'+ele).val();
			}
		});
		$.each(['AutoPedestrianEmpty', 'OverpressureDetection'], function(index, ele){
			if($('#'+ele).length != 0){
				if(!$("#"+ele).is(":checked"))
				{
					o[ele] = 0;
				}
				else
				{
					o[ele] = 1;
				}	
			}
		});				
		var xmlStr = $.json2xml(o,{root:'UNITPARAMS'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/unitParams',
			success: function(data, textStatus){
				optResultDefault(data);
			}
		});
	}
}

/*************************************

/**
* 获取相位
*/
function getPhaseTableInfo(){
		var o = {};
		o['PhaseAccount'] = $('#PhaseAccount').val();
		var xmlStr = $.json2xml(o,{root:'PhaseNo'});
		getPhaseTInfo(fillForm);
		
	function getPhaseTInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort + '/goform/PhaseTable',
			
			success: function(data, textStatus){
				if(callback){
					callback(data);
				}
			}
		});
	}

	function fillForm(data){
		var d = $.xml2json(data);
		
		//if(d["YellowLightTime"] != 0)
		{
			setFormDefault(d);	
		}
			
	}

}

/**
*设置相位
*/
function setPhaseTableInfo(formValid, $validator){
	if(formValid){
		var o = {};
		$.each(['PhaseAccount','CircleAccount','MinimumGreen','MaximumGreenOne','MaximumGreenTwo','ExtensionGreen','MaximumRestrict','DynamicStep','YellowLightTime','AllRedTime','GreenLightTime','RedLightProtect',
			'PedestrianRelease','PedestrianCleaned','Initialize','NonInduction','VehicleAutomaticRequest'], function(index, ele){
			if($('#'+ele).length > 0){
				o[ele] = $('#'+ele).val();
			}
		});
		//alertMsg( $('#PhaseAccount').val());
		$.each(['KeepPedestrianRelease','IncreaseInitialValueCalculation','NoLockDetentionRequest','DoubleEntrancePhase','GuaranteeFluxDensityExtensionGreen','ConditionalServiceValid','MeanwhileEmptyLoseEfficacy','Enable','AutomaticFlashInto','PedestrianAutomaticRequest',
			'AutomaticFlashExit','AutoPedestrianPass'], function(index, ele){
			if($('#'+ele).length != 0){
				if(!$("#"+ele).is(":checked"))
				{
					o[ele] = 0;
				}
				else
				{
					o[ele] = 1;
				}	
			}
		});	
		var xmlStr = $.json2xml(o,{root:'PhaseNo'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url:  m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/PhaseTable',
			success: function(data, textStatus){
				optResultDefault(data);
			}
		});
	}
}
/*************************************

/**
* 获取环/并发相位信息
*/
function getRingAndPhaseInfo(){
		getRingAPInfo(fillForm);
		//alertMsg("uy");
	function getRingAPInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/ringAndPhase',
			success: function(data, textStatus){
				if(callback){
					callback(data);
				}
			}
		});
	}

	function fillForm(data){
		var d = $.xml2json(data);
		
		//setFormDefault(d);	
		
		for(var key in d){
			//alertMsg($("#" + key).val());
			$("#" + key).omCombo({value: d[key]});//修改，这样就可以根据json的数据给它赋值了. 
		}	
	}

}

/**
*设置环/并发相位信息
*/
function setRingAndPhaseInfo(formValid, $validator){
	if(formValid){
		var o = {};

		for(Hphase=1;Hphase<=addPhase;Hphase++){
			o["RingForPhase"+phaseArray[Hphase-1]] = $('#RingForPhase'+phaseArray[Hphase-1]).val();
			o["SamePhase"+phaseArray[Hphase-1]] = $('#SamePhase'+phaseArray[Hphase-1]).val();
			//if(Hphase == 1)
			 //alertMsg(Hphase + "    "+o["RingForPhase"+Hphase] +"  <====>  " + o["SamePhase"+Hphase]);
		}
		o["TotalPhaseCount"] = $("#TotalPhaseCount").text();//并发相位总数
		o["PhaseArray"] = phaseArray.toString();

		var xmlStr = $.json2xml(o,{root:'RingAndPhase'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/ringAndPhase',
			success: function(data, textStatus){
						optResultDefault(data);
					}
			  });
	    }
	}
/*************************************

/**
* 获取通道表信息
*/
function getChannelTableInfo(){
		var o = {};
		o['channelNum'] = $('#channelNum').val();
		var xmlStr = $.json2xml(o,{root:'ChannelTable'});
		getChannelInfo(fillForm);
	function getChannelInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			data: xmlStr,
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/channelTable',
			success: function(data, textStatus){
				if(callback){
					callback(data);
				}
			}
		});
	}

	function fillForm(data){
		var d = $.xml2json(data);
		
		setFormDefault(d);		
	}

}

/**
*设置通道表信息
*/
function setChannelTableInfo(formValid, $validator){
	if(formValid){
		var o = {};
		$.each(['channelNum', 'controlSource','controlType','flashMode',
			'brightMode'], function(index, ele){
			if($('#'+ele).length > 0){
				o[ele] = $('#'+ele).val();
			}
		});
		
		var xmlStr = $.json2xml(o,{root:'ChannelTable'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/channelTable',
			success: function(data, textStatus){
						optResultDefault(data);
					}
			  });
	    }
	}
/*************************************

/**
* 获取绿信比表信息
*/
function getGreenRatioInfo(){
		var o = {};
		o['splitNo'] = $('#splitNo').val();
		var xmlStr = $.json2xml(o,{root:'GreenRatio'});
		getGreenRaInfo(fillForm);
	function getGreenRaInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			data: xmlStr,
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/greenRatio',
			success: function(data, textStatus){
				if(callback){
					callback(data);
				}
			}
		});
	}
	function fillForm(data){
		var d = $.xml2json(data);
		setFormDefault(d);	
		
	}
}

/**
*设置绿信比信息
*/
function setGreenRatioInfo(formValid, $validator){
	if(formValid){
		var o = {};

		o["splitNo"] = $('#splitNo').val();
		for(Hphase=1;Hphase<=addPhase;Hphase++)
		{
			o["splitForPhase"+phaseArray[Hphase-1]] = $('#splitForPhase'+phaseArray[Hphase-1]).val();
			o["splitMode"+phaseArray[Hphase-1]] = $('#splitMode'+phaseArray[Hphase-1]).val();
			
			if(!$("#"+"coordinate"+phaseArray[Hphase-1]).is(":checked"))
			{
				o["coordinate"+phaseArray[Hphase-1]] = 0;
			}
			else
			{
				o["coordinate"+phaseArray[Hphase-1]] = 1;
			}	
			
			//alertMsg("===>   "+o["coordinate"+phaseArray[Hphase-1]]+"   Hphase  "+$("#coordinate1").is(":checked"));
		}
		o["TotalPhaseCount"] = $("#TotalPhaseCount").text();//相位总数
		o["PhaseArray"] = phaseArray.toString();//相位数组的具体顺序
		var xmlStr = $.json2xml(o,{root:'GreenRatio'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/greenRatio',
			success: function(data, textStatus){
						optResultDefault(data);
					}
			  });
	}
}
/*************************************

/**
* 获取故障检测设置信息
*/
function getFaultDetectionInfo(){
		getFaultDetectInfo(fillForm);
	function getFaultDetectInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/faultDetectionSet',
			success: function(data, textStatus){
				if(callback){
					callback(data);
					isChanged();
				}
			}
		});
	}
	function fillForm(data){
		var d = $.xml2json(data);
		setFormDefault(d);		
	}
}

/**
*设置故障诊断设置信息
*/
function setFaultDetectionInfo(formValid, $validator){
	if(formValid){
		var o = {};
		var ChNo0;
		var ChNo1;
		var ChNo2;
		var CNum = 1;
		for(CNum=1;CNum<=32;CNum++)
		{
			ChNo0 = "CNum" + CNum + "_0";
			ChNo1 = "CNum" + CNum + "_1";
			ChNo2 = "CNum" + CNum + "_2";
			o[ChNo0] = $('#'+ChNo0).val();
			o[ChNo1] = $('#'+ChNo1).val();
			o[ChNo2] = $('#'+ChNo2).val();
		}
		$.each(['VoltageDetectionTimes','RedLightDetectionTimes','ConflictDetectionAttempts',
			'ManualPanelKeyNumber','RemoteControlKeyNumber'], function(index, ele){
			if($('#'+ele).length > 0){
				o[ele] = $('#'+ele).val();
			}
		});
		$.each(['SenseSwitch','DynamicStep','CurrentFaultDetection','AlarmAndFaultCurrent',
			'AlarmAndFaultVoltage','EnableWatchdog', 'EnableGPS'], function(index, ele){
			if($('#'+ele).length != 0){
				if(!$("#"+ele).is(":checked"))
				{
					o[ele] = 0;
				}
				else
				{
					o[ele] = 1;
				}	
			}
		});
		var xmlStr = $.json2xml(o,{root:'FaultDetectionSet'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/faultDetectionSet',
			success: function(data, textStatus){
						optResultDefault(data);
					}
			  });
	}
}
/*************************************

/**
* 获取相序表信息
*/
function getSequenceTableInfo(){
		var o = {};
		//alertMsg("xxx");
		o['nPhaseTurnId'] = $('#PhaseTurnNum').val();
		var xmlStr = $.json2xml(o,{root:'SequenceTable'});
		
		//alertMsg("==>" + o['nPhaseTurnId']);
		getSequenceInfo(fillForm);
	function getSequenceInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			data: xmlStr,
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/sequenceTable',
			success: function(data, textStatus){
	//			alertMsg("asdf");
				if(callback){
					
					callback(data);
				}
			}

		});
	}
	function fillForm(data){
	//	alertMsg("asdf");
		var d = $.xml2json(data);
		setFormDefault(d);		
	}
}

/**
*设置相序表信息
*/
function setSequenceTableInfo(formValid, $validator){
	if(formValid){
		var o = {};

		o['nPhaseTurnId'] = $('#PhaseTurnNum').val();

		var H2phase;
		var Hphase;
		var Circle;
		
		for(Circle = 1 ; Circle <= 4 ; Circle++)//依次添加4个环 
		{
			for(Hphase = 1 ; Hphase <= addPhase ; Hphase++)
			{
				var thisID = "Circle_"+Circle+"_"+Hphase;
				o[thisID] = $('#'+thisID).val();
				
				//if((Circle == 1 ) && (Hphase == 1))
				//{
					//alertMsg("====>   "+$('#'+thisID).val() );
				//}
			}
		
		}
		
		var xmlStr = $.json2xml(o,{root:'SequenceTable'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/sequenceTable',
			success: function(data, textStatus){
						optResultDefault(data);
					}
			  });
	}
}
/*************************************

/**
* 获取方案表信息
*/
function getProgramTableInfo(){
	//alertMsg("111");
		var o = {};
		o['ProgramTableNo'] = $('#ProgramTableNo').val();
		var xmlStr = $.json2xml(o,{root:'ProgramTable'});
		getProgramInfo(fillForm);
	function getProgramInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			data: xmlStr,
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/programTable',
			success: function(data, textStatus){
				if(callback){
					callback(data);
				}
			}
		});
	}
	function fillForm(data){
		//alertMsg("asdf");
		var d = $.xml2json(data);
		setFormDefault(d);	
		
		var x = document.getElementById("nGreenSignalRatioID");
		var index = 0;
		index = GetArrayIndex(splitArray,d['nGreenSignalRatioID']);
		//alertMsg(d['nGreenSignalRatioID']);
		x.options[index].selected = true;//设置

		x = document.getElementById("nPhaseTurnID");
		index = GetArrayIndex(phaseTurnArray,d['nPhaseTurnID']);
		x.options[index].selected = true;//设置
	}
}

/**
*设置方案表信息
*/
function setProgramTableInfo(formValid, $validator){
	if(formValid){
		var o = {};
		
		o['ProgramTableNo'] = $('#ProgramTableNo').val();
		o['nCycleTime'] = $('#nCycleTime').val();
		o['nOffset'] = $('#nOffset').val();
		o['nGreenSignalRatioID'] = $('#nGreenSignalRatioID').val();
		o['nPhaseTurnID'] = $('#nPhaseTurnID').val();
		
		var xmlStr = $.json2xml(o,{root:'ProgramTable'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/programTable',
			success: function(data, textStatus){
						optResultDefault(data);
					}
			  });
	}
}
/*************************************

/**
* 获取时基动作表信息
*/
function getTimeBasedActionTableInfo(){
		var o = {};
		o['ActionTable'] = $('#ActionTable').val();
		var xmlStr = $.json2xml(o,{root:'TimeBasedActionTable'});		
		
		getTimeBasedActInfo(fillForm);
		
	function getTimeBasedActInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			data: xmlStr,
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/timeBasedActionTable',
			success: function(data, textStatus){
				if(callback){
					callback(data);
				}
			}
		});
	}
	function fillForm(data){
		var d = $.xml2json(data);
		setFormDefault(d);		
		//alertMsg("===  "+d['ProgramNo']);
		var x = document.getElementById("ProgramNo");
		var index = 0;
		if(d['ProgramNo'] != 0)
		{
			index = GetArrayIndex(programArray,d['ProgramNo']);
			index++;		
		}
		x.options[index].selected = true;//设置
	}
}

/**
*设置时基动作表信息
*/
function setTimeBasedActionTableInfo(formValid, $validator){
	if(formValid){
		var o = {};
		$.each(['ActionTable','ProgramNo'], function(index, ele){
			if($('#'+ele).length > 0){
				o[ele] = $('#'+ele).val();
			}
		});
		//alertMsg(o["ProgramNo"]);
	
		$.each(['AssistFunction1','AssistFunction2','AssistFunction3','AssistFunction4',
			'SpecialFunction1','SpecialFunction2','SpecialFunction3','SpecialFunction4',
			'SpecialFunction5','SpecialFunction6','SpecialFunction7','SpecialFunction8'], function(index, ele){
			if($('#'+ele).length != 0){
				if(!$("#"+ele).is(":checked"))
				{
					o[ele] = 0;
				}
				else
				{
					o[ele] = 1;
				}	
			}
		});
		var xmlStr = $.json2xml(o,{root:'TimeBasedActionTable'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/timeBasedActionTable',
			success: function(data, textStatus){
						optResultDefault(data);
					}
			  });
	}
}
/*************************************

/**
* 获取时段表信息
*/
function getTimeIntervalInfo(){
		var o = {};
		o['TimeIntervalID'] = $('#TimeIntervalNo').val();
		var xmlStr = $.json2xml(o,{root:'TimeInterval'});	

		getTimeInterInfo(fillForm);
	function getTimeInterInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			data: xmlStr,
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/timeInterval',
			success: function(data, textStatus){
				if(callback){
					callback(data);
				}
			}
		});
	}
	function fillForm(data){
		var d = $.xml2json(data);
		//alertMsg(d['Hour_1']);
		//addNewLine(1,1);
		var flag = 0;
		var hour;
		var minute;
		var actionId;

		for(var i in d)
		{
			if(i.indexOf("Hour_") != -1)
			{
				hour = d[i];
				flag++;
				//alertMsg("===>  "+i+"  : "+d[i]);
			}

			if(i.indexOf("Minute_") != -1)
			{
				minute = d[i];
				flag++;
				//alertMsg("===>  "+i+"  : "+d[i]);
			}		
			if(i.indexOf("ActionId_") != -1)
			{
				actionId = d[i];
				flag++;
				//alertMsg("===>  "+i+"  : "+d[i]);
			}		

			if(flag == 3)//每行有3个元素
			{
				flag = 0;
				addNewLine(hour,minute,actionId,0);
			}
		
		
		}
		
		//setFormDefault(d);		
	}
}

/**
*设置时段表信息
*/
function setTimeIntervalInfo(formValid, $validator){
	if(formValid){
		var o = {};
		o['TimeIntervalID'] = $('#TimeIntervalNo').val();
	
		var totalNum = $('#TotalTimeCount').text();
		
		var hour;
		var minute;
		var action;
		var i;
		for(i = 0 ; i < totalNum ; i++)
		{
			hour = "Hour_" + (i+1);
			minute = "Minute_" + (i+1);
			action = "ActionId_" + (i+1);	
			
			o[hour] = $('#'+hour).val();
			o[minute] = $('#'+minute).val();
			
			//alertMsg("--->"  + $('#'+action).val());
			if($('#'+action).val() == "关灯控制")
			{
				o[action] = "115";
			}
			else if($('#'+action).val()  == "全红控制")
			{
				o[action]  = "116";
			}
			else if($('#'+action).val()  == "感应控制")
			{
				o[action]  = "118";
			}
			else if($('#'+action).val()  == "黄闪控制")
			{
				o[action]  = "119";
			}
			else
			{
				o[action] = $('#'+action).val();
			}
			
			//alertMsg("  "+o[action]);
		}
		
		o['TotalTimeCount'] = totalNum;
		var xmlStr = $.json2xml(o,{root:'TimeInterval'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/timeInterval',
			success: function(data, textStatus){
						optResultDefault(data);
					}
			  });
	}
}
/*************************************

/**
* 获取调度计划信息
*/
function getSchedulingInfo(){
		var o = {};
		o['SchedulingNo'] = $('#SchedulingNo').val();
		var xmlStr = $.json2xml(o,{root:'Scheduling'});	

		getScheduleInfo(fillForm);
	function getScheduleInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			data: xmlStr,
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/scheduling',
			success: function(data, textStatus){
				if(callback){
					callback(data);
				}
			}
		});
	}
	function fillForm(data){
		var d = $.xml2json(data);
		setFormDefault(d);
		var x = document.getElementById("TimeIntervalNum");
		var index = 0;
		index = GetArrayIndex(TimeIntervalArray,d['TimeIntervalNum']);

		x.options[index].selected = true;//设置
		
	}
}

/**
*设置调度计划信息
*/
function setSchedulingInfo(formValid, $validator){
	if(formValid){
		var o = {};
		$.each(['SchedulingNo','TimeIntervalNum'], function(index, ele){
			if($('#'+ele).length > 0){
				o[ele] = $('#'+ele).val();
			}
		});
		var MDW;
		var Num = 1;


		for(Num=1;Num<=12;Num++)
		{
			MDW = "Month" + Num;
			if(!$("#"+MDW).is(":checked"))
			{
				o[MDW] = 0;
			}
			else
			{
				o[MDW] = 1;
			}
		}
		

		
		for(Num=1;Num<=31;Num++)
		{
			MDW = "Day" + Num;
			if(!$("#"+MDW).is(":checked"))
			{
				o[MDW] = 0;
			}
			else
			{
				o[MDW] = 1;
			}
		}


		for(Num=1;Num<=8;Num++)
		{
			MDW = "WeekDay" + Num;
			if(!$("#"+MDW).is(":checked"))
			{
				o[MDW] = 0;
			}
			else
			{
				o[MDW] = 1;
			}
		}
		

		var xmlStr = $.json2xml(o,{root:'Scheduling'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/scheduling',
			success: function(data, textStatus){
						optResultDefault(data);
					}
			  });
	}
}
/*************************************

/**
* 获取跟随相位 
*/
function getOverlappingTableInfo(){
		var o = {};
		o['FollowPhase'] = $('#FollowPhase').val();
		var xmlStr = $.json2xml(o,{root:'Overlapping'});	

		getOverlappingInfo(fillForm);
	function getOverlappingInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			data: xmlStr,
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/overlapping',
			success: function(data, textStatus){
				if(callback){
					callback(data);
				}
			}
		});
	}
	function fillForm(data){
		var d = $.xml2json(data);
		setFormDefault(d);		
	}
}

/**
*设置跟随相位 信息
*/
function setOverlappingTableInfo(formValid, $validator){
	if(formValid){
		var o = {};
		var Num;
		var MPhase;
		$.each(['FollowPhase','GreenLight','RedLight','YellowLight',
			'GreenFlash','ModifiedPhase'], function(index, ele){
			if($('#'+ele).length > 0){
				o[ele] = $('#'+ele).val();
			}
		});
		for(Num=1;Num<=32;Num++)
		{
			MPhase = "ParentPhase" + Num;
			if(!$("#"+MPhase).is(":checked"))
			{
				o[MPhase] = 0;
			}
			else
			{
				o[MPhase] = 1;
			}
		}
		var xmlStr = $.json2xml(o,{root:'Overlapping'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/overlapping',
			success: function(data, textStatus){
						optResultDefault(data);
					}
			  });
	}
}
/*************************************

/**
* 获取协调信息
*/
function getCoordinateInfo(){
		getCoordInfo(fillForm);
	function getCoordInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/coordinate',
			success: function(data, textStatus){
				if(callback){
					callback(data);
				}
			}
		});
	}
	function fillForm(data){
		var d = $.xml2json(data);
		setFormDefault(d);		
	}
}

/**
*设置协调信息
*/
function setCoordinateInfo(formValid, $validator){
	if(formValid){
		var o = {};
		$.each(['ControlModel','ManualMethod','CoordinationMode',
			'CoordinateMaxMode','CoordinateForceMode'], function(index, ele){
			if($('#'+ele).length > 0){
				o[ele] = $('#'+ele).val();
			}
		});
		var xmlStr = $.json2xml(o,{root:'Coordinate'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/coordinate',
			success: function(data, textStatus){
						optResultDefault(data);
					}
			  });
	}
}
/*************************************

/**
* 获取车辆检测器信息
*/
function getVehicleDetectorInfo(){
		var o = {};
		o['DetectorNo'] = $('#DetectorNo').val();
		var xmlStr = $.json2xml(o,{root:'VehicleDetector'});
		//alertMsg("Send===>  "  + o['DetectorNo']);
		getVehicleInfo(fillForm);
	function getVehicleInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			data: xmlStr,
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/vehicleDetector',
			success: function(data, textStatus){
				if(callback){
					callback(data);
				}
			}
		});
	}
	function fillForm(data){
		var d = $.xml2json(data);
		//alertMsg("Get  ===>  "+d["RequestPhase"]);
		//d["RequestPhase"] = d["RequestPhase"] - 1;
		setFormDefault(d);
		
		var index = 0;
		var x ;
		
		x = document.getElementById("RequestPhase");		
		if(d['RequestPhase'] != 0)
		{
			index = GetArrayIndex(phaseArray,d['RequestPhase']);
			index++;		
		}
		//alertMsg("===>  Request "+ d["RequestPhase"]+"   Index "+index);
		x.options[index-1].selected = true;//设置		
		
		x = document.getElementById("SwitchPhase");		
		if(d['SwitchPhase'] != 0)
		{
			index = GetArrayIndex(phaseArray,d['SwitchPhase']);
			index++;		
		}
		x.options[index-1].selected = true;//设置		
		
	}
}

/**
*设置车辆检测器信息
*/
function setVehicleDetectorInfo(formValid, $validator){
	if(formValid){
		var o = {};
		$.each(['DetectorNo','RequestPhase','SwitchPhase','Delay','FailureTime',
			'QueueLimit','NoResponseTime','MaxDuration','Extend','MaxVehicle'], function(index, ele){
			if($('#'+ele).length > 0){
				o[ele] = $('#'+ele).val();
			}
		});
		$.each(['Flow','Occupancy','ProlongGreen','AccumulateInitial','Queue',
			'Request','RedInterval','YellowInterval'], function(index, ele){
			if($('#'+ele).length != 0){
				if(!$("#"+ele).is(":checked"))
				{
					o[ele] = 0;
				}
				else
				{
					o[ele] = 1;
				}	
			}
		});
		var xmlStr = $.json2xml(o,{root:'VehicleDetector'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/vehicleDetector',
			success: function(data, textStatus){
						optResultDefault(data);
					}
			  });
	}
}
/*************************************

/**
* 获取行人检测器信息
*/
function getPedestrianDetectorInfo(){

		var o = {};
		o['DetectorNo'] = $('#DetectorNo').val();
		var xmlStr = $.json2xml(o,{root:'PedestrianDetector'});
		getPedestrianInfo(fillForm);
	function getPedestrianInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			data: xmlStr,
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/pedestrianDetector',
			success: function(data, textStatus){
				if(callback){
					callback(data);
				}
			}
		});
	}
	function fillForm(data){
		var d = $.xml2json(data);
		setFormDefault(d);	

		var index = 0;
		var x ;
		
		x = document.getElementById("RequestPhase");		
		if(d['RequestPhase'] != 0)
		{
			index = GetArrayIndex(phaseArray,d['RequestPhase']);
			index++;		
		}
		//alertMsg("===>  Request "+ d["RequestPhase"]+"   Index "+index);
		x.options[index-1].selected = true;//设置	
		
	}
}

/**
*设置行人检测器信息
*/
function setPedestrianDetectorInfo(formValid, $validator){
	if(formValid){
		var o = {};
		$.each(['DetectorNo','RequestPhase','NoResponseTime',
			'MaxDuration','InductionNumber'], function(index, ele){
			if($('#'+ele).length > 0){
				o[ele] = $('#'+ele).val();
			}
		});
		var xmlStr = $.json2xml(o,{root:'PedestrianDetector'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/pedestrianDetector',
			success: function(data, textStatus){
						optResultDefault(data);
					}
			  });
	}
}
/*************************************

/**
* 获取故障配置信息
*/

function setButtonAttr()
{
	if ($("#ControlRecord").val() == 1) {
		$("#ShowFaultRecord").attr("style", "color:red; visibility:visible");
	} else {
		$("#ShowFaultRecord").attr("style", "visibility:hidden");
		if (tableName.localeCompare("T_Fault") == 0) {
			$("#TableArea").attr("style", "visibility:hidden");
		}
	}
	
	if ($("#LogRecord").val() == 1) {
		$("#ShowLogRecord").attr("style", "color:blue; visibility:visible");
	} else {
		$("#ShowLogRecord").attr("style", "visibility:hidden");
		if (tableName.localeCompare("T_Record") == 0) {
			$("#TableArea").attr("style", "visibility:hidden");
		}
	}
}

function getFaultConfigInfo(){
		getFaultConfInfo(fillForm);
	function getFaultConfInfo(callback){
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/faultConfig',
			success: function(data, textStatus){
				if(callback){
					callback(data);
				}
			}
		});
	}
	function fillForm(data){
		var d = $.xml2json(data);
		setFormDefault(d);
		setButtonAttr();
	}
}

/**
*设置故障配置信息
*/
function setFaultConfigInfo(formValid, $validator){
	if(formValid){
		var o = {};
		//$.each(['ControlRecord','CommunicatRecord','DetectorRecord'], function(index, ele){
		$.each(['ControlRecord','LogRecord'], function(index, ele){
			if($('#'+ele).length > 0){
				o[ele] = $('#'+ele).val();
			}
		});
		var xmlStr = $.json2xml(o,{root:'FaultConfig'});
		$.ajax({
			type: 'POST',
			dataType: 'xml',
			data: xmlStr,
			url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/faultConfig',
			success: function(data, textStatus){
						optResultDefault(data);
						setButtonAttr();
					}
			  });
	}
}



