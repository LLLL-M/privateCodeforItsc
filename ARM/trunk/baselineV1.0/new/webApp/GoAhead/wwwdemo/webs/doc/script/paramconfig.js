var _paramObj = {};
var menuArray = ["相位表","通道表","方案表","绿信比表","相序表","时基动作表","时段","调度计划","跟随相位","车辆检测","行人检测"];
var phaseArray = new Array();
var channelArray = new Array();
var programArray = new Array();
var splitArray = new Array();
var phaseTurnArray = new Array();
var ActionArray = new Array();
var TimeIntervalArray = new Array();
var ScheduleArray = new Array();
var FollowPhaseArray = new Array();
var VehicleDetectorArray = new Array();
var PedestrianDetectorArray = new Array();


//var phaseArray = new Array();
var navData = [{id:"n1",text:"单元参数",expanded:false},
               {id:"n2",text:"相位表",expanded:false},
               {id:"n1_1",pid:"n1",text:"参数配置",url:"unitParams"},
               //{id:"n2_1",pid:"n2",text:"相位号1",url:"phaseTable"},
               {id:"n3",text:"环/并发相位",expanded:false},
               {id:"n3_1",pid:"n3",text:"环/并发相位",url:"ringAndPhase"},
               {id:"n4",text:"通道表",expanded:false},
               //{id:"n4_1",pid:"n4",text:"通道号1",url:"channelTable"},
               {id:"n5",text:"绿信比表",expanded:false},
              // {id:"n5_1",pid:"n5",text:"绿信比表1",url:"splitTable"},
               {id:"n6",text:"相序表",expanded:false},
               //{id:"n6_1",pid:"n6",text:"相序表1",url:'sequenceTable'},
               {id:"n7",text:"方案表",expanded:false},
              // {id:"n7_1",pid:"n7",text:"方案表1",url:"programTable"},
               {id:"n8",text:"时基动作表",expanded:false},
              // {id:"n8_1",pid:"n8",text:"动作表1",url:"timeBasedActionTable"},
               {id:"n9",text:"时段",expanded:false},
               //{id:"n9_1",pid:"n9",text:"时段号1",url:"timeInterval"},
               {id:"n10",text:"调度计划",expanded:false},
              // {id:"n10_1",pid:"n10",text:"调度表1",url:"scheduling"},
               {id:"n11",text:"跟随相位",expanded:false},
              // {id:"n11_1",pid:"n11",text:"跟随相位1",url:"overlappingTable"},
               {id:"n12",text:"协调",expanded:false},
               {id:"n12_1",pid:"n12",text:"协调",url:"coordinate"},
               {id:"n13",text:"车辆检测",expanded:false},
               //{id:"n13_1",pid:"n13",text:"检测器编号1",url:"vehicleDetector"},
               {id:"n14",text:"行人检测",expanded:false},
               //{id:"n14_1",pid:"n14",text:"检测器编号1",url:"pedestrianDetector"},
               {id:"n15",text:"故障配置",expanded:false},
               {id:"n15_1",pid:"n15",text:"故障配置",url:"faultConfig"},
               {id:"n16",text:"故障配置设置",expanded:false},
               {id:"n16_1",pid:"n16",text:"故障配置设置",url:"faultDetectionSet"}];
/*************************************************
Function:   InitTreeParam
Description:  加载树的动态参数
Input:          无
Output:     无
return:     无       
*************************************************/
function InitTreeParam(callback){
    $.ajax({
      type: 'GET',
      dataType: 'xml',
      url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/TreeDynamicParameter',
      
      success: function(data, textStatus){
		fillForm(data);
        if(callback){
            callback();
        }
      }
    });
  

  function fillForm(data){
    var d = $.xml2json(data);
	
    addPhase = parseInt(d.addCount);	//相位表总数
    addChannel = parseInt(d.addChannel);//通道表总数
    addProgram = parseInt(d.addProgram);//方案表总数
    addSplit = parseInt(d.addSplit);//绿信比表总数
	nPhaseTurnCount = parseInt(d.nPhaseTurnCount);//相序表总数
	nActionCount = parseInt(d.nActionCount);//动作表总数
	nTimeInterval = parseInt(d.nTimeInterval);//时段表总数	
	nScheduleCount = parseInt(d.nScheduleCount);
	nFollowPhaseCount = parseInt(d.nFollowPhaseCount);
	nVehicleDetectorCount = parseInt(d.nVehicleDetectorCount);
	nPedestrianDetectorCount = parseInt(d.nPedestrianDetectorCount);	
	
	//alert("===>   "+addPhase);
	var arrayTemp = new Array();
	var strTemp = d.PhaseArray;
	
	arrayTemp = strTemp.split(",");//所有的id都是按照存入数组的实际顺序来做的
	for(i = 0 ; i < addPhase ; i++)
	{
		navData.push({id:"n1_"+i,pid:"n2",text:"相位号"+arrayTemp[i],url:"phaseTable"});
		phaseArray.push(arrayTemp[i]);
	}
	
	//通道表
	strTemp = d.ChannelArray;
	arrayTemp = strTemp.split(",");
	for(i = 0 ; i < addChannel ; i++)
	{
		navData.push({id:"n4_"+i,pid:"n4",text:"通道号"+arrayTemp[i],url:"channelTable"});
		channelArray.push(arrayTemp[i]);
	}
	
	//方案表
	strTemp = d.SchemeArray;
	arrayTemp = strTemp.split(",");
	for(i = 0 ; i < addProgram ; i++)
	{
        navData.push({id:"n7_"+i,pid:"n7",text:"方案表"+arrayTemp[i],url:"programTable"});
        programArray.push(arrayTemp[i]);
	}
	
	//绿信比表
	strTemp = d.GreenSignalRationArray;
	arrayTemp = strTemp.split(",");
	for(i = 0 ; i < addSplit ; i++)
	{
        navData.push({id:"n5_"+i,pid:"n5",text:"绿信比表"+arrayTemp[i],url:"splitTable"});
        splitArray.push(arrayTemp[i]);
	}
	
	//相序表
	strTemp = d.PhaseTurnArray;
	arrayTemp = strTemp.split(",");
	for(i = 0 ; i < nPhaseTurnCount ; i++)
	{
        navData.push({id:"n6_"+i,pid:"n6",text:"相序表"+arrayTemp[i],url:"sequenceTable"});
        phaseTurnArray.push(arrayTemp[i]);
	}
	
	//动作表
	strTemp = d.ActionArray;
	arrayTemp = strTemp.split(",");
	for(i = 0 ; i < nActionCount ; i++)
	{
        navData.push({id:"n8_"+i,pid:"n8",text:"动作表"+arrayTemp[i],url:"timeBasedActionTable"});
        ActionArray.push(arrayTemp[i]);
	}

    //时段表
	strTemp = d.TimeIntervalArray;
	arrayTemp = strTemp.split(",");
	for(i = 0 ; i < nTimeInterval ; i++)
	{
        navData.push({id:"n9_"+i,pid:"n9",text:"时段表"+arrayTemp[i],url:"timeInterval"});
        TimeIntervalArray.push(arrayTemp[i]);
	}	
	
	//调度表
	strTemp = d.ScheduleArray;
	arrayTemp = strTemp.split(",");
	for(i = 0 ; i < nScheduleCount ; i++)
	{
        navData.push({id:"n10_"+i,pid:"n10",text:"调度表"+arrayTemp[i],url:"scheduling"});
        ScheduleArray.push(arrayTemp[i]);
	}
	
	//跟随相位表
	strTemp = d.FollowPhaseArray;
	arrayTemp = strTemp.split(",");
	for(i = 0 ; i < nFollowPhaseCount ; i++)
	{
        navData.push({id:"n11_"+i,pid:"n11",text:"跟随相位"+arrayTemp[i],url:"overlappingTable"});
        FollowPhaseArray.push(arrayTemp[i]);
	}
	//alert("===>   "+phaseArray[addPhase - 2]);
	
	
	//车辆检测器
	strTemp = d.VehicleDetectorArray;
	arrayTemp = strTemp.split(",");
	//alert("===>Get  Array   "+strTemp);
	for(i = 0 ; i < nVehicleDetectorCount ; i++)
	{
        navData.push({id:"n13_"+i,pid:"n13",text:"车辆检测器"+arrayTemp[i],url:"vehicleDetector"});
        VehicleDetectorArray.push(arrayTemp[i]);
	}	
	
	//行人检测器
	strTemp = d.PedestrianDetectorArray;
	arrayTemp = strTemp.split(",");
	for(i = 0 ; i < nPedestrianDetectorCount; i++)
	{
        navData.push({id:"n14_"+i,pid:"n14",text:"行人检测器"+arrayTemp[i],url:"pedestrianDetector"});
        PedestrianDetectorArray.push(arrayTemp[i]);
	}
	
  }
}



/*************************************************
Function:   SaveTreeParam
Description:  保存树的动态参数
Input:          无
Output:     无
return:     无       
*************************************************/
function SaveTreeParam(){
    var o = {};
	
	o['addCount'] = addPhase;
	o['addChannel'] = addChannel;
	o['addProgram'] = addProgram;
	o['addSplit'] = addSplit;
	o['nPhaseTurnCount'] = nPhaseTurnCount;
	o['nActionCount'] = nActionCount;
	o['nTimeInterval'] = nTimeInterval;
	o['nScheduleCount'] = nScheduleCount;
	o['nFollowPhaseCount'] = nFollowPhaseCount;
	o['nVehicleDetectorCount'] = nVehicleDetectorCount;
	o['nPedestrianDetectorCount'] = nPedestrianDetectorCount;	
	
	o["PhaseArray"] = phaseArray.toString();
	o["channelArray"] = channelArray.toString();
	o["programArray"] = programArray.toString();
	o["splitArray"] = splitArray.toString();
	o["phaseTurnArray"] = phaseTurnArray.toString();
	o["ActionArray"] = ActionArray.toString();
	o["TimeIntervalArray"] = TimeIntervalArray.toString();
	o["FollowPhaseArray"] = FollowPhaseArray.toString();
	o["ScheduleArray"] = ScheduleArray.toString();
	o["VehicleDetectorArray"] = VehicleDetectorArray.toString();
	o["PedestrianDetectorArray"] = PedestrianDetectorArray.toString();
		
	//alert("===>Send Array   "+o["VehicleDetectorArray"]);
    var xmlStr = $.json2xml(o,{root:'TreeDynamicParameter'});
    $.ajax({
      type: 'POST',
      dataType: 'xml',
      data: xmlStr,
      url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/TreeDynamicParameter',
      success: function(data, textStatus){
            optResultDefault(data);
          }
        });
}
/*************************************************
Function:   saveFile
Description:  下载文件并保存
Input:          无
Output:     无
return:     无       
*************************************************/

function saveFile(content){
  getDownloadInfo();

  function getDownloadInfo(){
    $.ajax({
      type: 'GET',
      dataType: 'xml',
      url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/actionDownload',
        
      success: function(data, textStatus){
        var d = $.xml2json(data);
        //document.getElementById(content).innerHTML = d.download;
        var win=window.open('','','top=10000,left=10000');
        //win.document.write(document.all(content).innerHTML);
        win.document.write(d.download);
        win.document.execCommand('SaveAs','',"login.ini");
        win.close();
      }
    });
  }
}




