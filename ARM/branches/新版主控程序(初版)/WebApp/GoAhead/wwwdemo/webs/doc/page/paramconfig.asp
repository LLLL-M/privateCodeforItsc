<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
  <title>配置</title>
	<script type="text/javascript" src="../script/jquery.min.js"></script>
	<script type="text/javascript" src="../script/operamasks-ui.min.js"></script>
  <script type="text/javascript" src="../script/jquery.cookie.js"></script>
  <script type="text/javascript" src="../script/jquery.xml2json.js"></script>
	<script type="text/javascript" src="../script/paramconfig.js"></script>
  <script type="text/javascript" src="../script/util.js"></script>
  <script type="text/javascript" src="../script/trafficParams.js"></script>
  <script type="text/javascript" src="../script/common.js"></script>
	<link href="../css/om-elegant.css" rel="stylesheet" type="text/css"/>
  <link href="../css/paramconfig.css" rel="stylesheet" type="text/css"/>

  <style>
      #expand-top-bottom{
      width: 100%;
      height: 800px;
      }
    html 
    {
      width:100%;
      height:100%;
    }
    body 
    { 
      width:100%;
      height:100%;
      overflow:hidden; 
      background:#CCC;
      z-index:0;
    }
    label {width: 10em; float: left;}
    label.error {float: none; color: red; padding-left: .5em; vertical-align: top;}
    p {clear: both;}
    .submit {margin-left: 12em;}
    em {font-weight: bold; padding-right: 1em; vertical-align: top;}
    textarea{font-size: 15px;}
  </style>
  <script type="text/javascript">
    var option1;//这个是全局变量，在后面单击条目时会给它赋值，由这个值再来做保存
    function SaveChange(){
        doTrafficParams(option1);
    }

	//新添加一项时，插入前需判断该ID是否已存在。
	function IsIdExist(array,val)
	{
		for(var i in array)
		{
			if(val == array[i])
			{
				alert("ID 已存在，请确认 !");
				return 1;
			}
		}

		return 0;
	}

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


	//用户右键添加新一项时，需要用户输入新的ID号,形参分别为建议值、限定最大值
	function GetInsertId(defaultVal , maxVal)
	{
		if(defaultVal > maxVal)
		{
			defaultVal = maxVal;
		}
		
		var newId = prompt("请输入插入项的ID，范围是 1 - "+maxVal,defaultVal);

		if((newId > maxVal) || (newId < 1) || isNaN(newId))
		{
			newId = 0;
			alert("ID不合法，请重新添加!");
		}

		return newId;
	}

	
	//重置函数，重新加载配置文件，删除当前所有修改
	function ResetPara()
	{
		var r  = confirm("重新加载配置文件，删除当前所有修改 ?"+'\n'+"点击确定后，您所做的所有更改将被永久清除。");

		if(r == true)
		{
		  ResetAllPara();

		  function ResetAllPara(){
			$.ajax({
			  type: 'GET',
			  dataType: 'xml',
			  url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/resetAllPara',
				
			  success: function(data, textStatus){
				var d = $.xml2json(data);
				
				if(d['statusCode'] == 1)
				{
					//alert("修改成功");
					InitTreeParam();
					location.reload(true);//获得新的目录树后，刷新当前页面。
				}
				else
				{
					alert("修改失败");
				}

			  }
			});
		  }	
		}
	
	}
	
	//清空配置文件，只是清空并不保存到板卡
	function ClearPara()
	{
		var r  = confirm("清空所有已配置信息?"+'\n'+"点击确定后,配置表将置零且您所做的所有更改将被永久清除。");

		if(r == true)
		{
		  ClearAllPara();

		  function ClearAllPara(){
			$.ajax({
			  type: 'GET',
			  dataType: 'xml',
			  url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/clearAllPara',
				
			  success: function(data, textStatus){
				var d = $.xml2json(data);
				
				if(d['statusCode'] == 1)
				{
					//alert("修改成功");
					InitTreeParam();
					location.reload(true);//获得新的目录树后，刷新当前页面。
				}
				else
				{
					alert("修改失败");
				}

			  }
			});
		  }	
		}	
	}

	//将当前所有修改发送到板卡
	function SaveAll()
	{
		var r = confirm("立即将全部参数发送到板卡?");
		
		if(r == true)
		{
		  saveAllPara();

		  function saveAllPara(){
			$.ajax({
			  type: 'GET',
			  dataType: 'xml',
			  url:m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/saveAllPara',
				
			  success: function(data, textStatus){
				var d = $.xml2json(data);
				
				if(d['statusCode'] == 0)
				{
					alert("保存成功");
				}
				else
				{
					alert("保存失败,失败码为: "+d['statusCode']+"\n"+d['ErrorContent']);
				}

			  }
			});
		  }		
		}

	}

	function GetLibsVersion()
	{
		getLibsVersionInfo();
		//alert("Get Version !");
	}
	
    $(document).ready(function() {

      InitTreeParam(InitTree);//初始化系统动态参数
	  GetLibsVersion();
      $('body').omBorderLayout({
        panels:[{
          id:"center-panel",
          header:false,
          region:"center"
        },{
          id:"west-panel",
          title:"特征参数",
          region:"west",
          expandToBottom:true,
          expandToTop:false,
          width:150
        },{
            id:"north-panel",
            header:false,
            region:"north",
            height:85
        },{
            id:"east-panel",
            header:false,
            expandToBottom:true,
            expandToTop:false,
            region:"east",
            width:300
        }],
		spacing:3 
      });
      //给按钮设置样式start
      $("#SaveConfigBtn").omButton(); 
        var tabElement = $('#center-tab').omTabs({
            height : "fit"
        });
      $("#BtnSaveAll").omButton(); 
        var tabElement = $('#center-tab').omTabs({
            height : "fit"
        });
      $("#BtnReset").omButton(); 
        var tabElement = $('#center-tab').omTabs({
            height : "fit"
        });
      $("#BtnClear").omButton(); 
        var tabElement = $('#center-tab').omTabs({
            height : "fit"
        });
      $("#download").omButton(); 
      var tabElement = $('#center-tab').omTabs({
          height : "fit"
      });
      $("#upload").omButton(); 
      var tabElement = $('#center-tab').omTabs({
          height : "fit"
      });
      //给按钮设置样式end
   
      function InitTree(){
        $("#navTree").omTree({
          dataSource : navData,
          simpleDataModel: true,
          onClick : function(nodeData,event){
			//alert("==>  "+option1);
            $("#menu").omMenu("hide");
            if(nodeData.url){
              var tabId = tabElement.omTabs('getAlter','tab_'+nodeData.id);
			  //alert("==>   "+nodeData.nid);
			  //var str = nodeData.id;
			  //alert(str.substr(3));
              if(tabId){
                          tabElement.omTabs('activate',tabId);
              }else{      
                      var option = nodeData.url+":get";
                      option1 = nodeData.url+":set";//不能删，是全局变量
					  //alert("==>  "+option1);
                      doTrafficParams(option);
					  //setTimeout("doTrafficParams(option1)",200);//200ms后自动保存
              }
            }
          },
          onRightClick : function(nodeData,event){
            if($.inArray(nodeData.text,menuArray)!=-1){
              $("#tree").omTree("select",nodeData);
              $("#menu").omMenu("enableItem","001");
              $("#menu").omMenu("disableItem","002");
              $('#menu').omMenu('show',event);
            }
			
			var text = nodeData.text;

			//修改为只有最后一行才有删除选项
             if(($.inArray(text.substr(3),phaseArray)!=-1)||($.inArray(text.substr(3),channelArray)!=-1)||($.inArray(text.substr(3),programArray)!=-1)||($.inArray(text.substr(4),splitArray)!=-1) || ($.inArray(text.substr(3),phaseTurnArray)!=-1) || ($.inArray(text.substr(3),ActionArray)!=-1) || 
			 ($.inArray(text.substr(3),TimeIntervalArray)!=-1) || ($.inArray(text.substr(3),ScheduleArray)!=-1) || ($.inArray(text.substr(4),FollowPhaseArray)!=-1)|| ($.inArray(text.substr(5),VehicleDetectorArray)!=-1)|| ($.inArray(text.substr(5),PedestrianDetectorArray)!=-1)){
              $("#tree").omTree("select",nodeData);
              $("#menu").omMenu("disableItem","001");
              $("#menu").omMenu("enableItem","002");
              $('#menu').omMenu('show',event);
            }
            event.preventDefault(); 
          }
        });
      //左键点击页面隐藏菜单
      $("body").bind("click", function(){
        $("#menu").omMenu("hide");
      });
      $('#menu').omMenu({
        contextMenu : true,
        dataSource : [{id:'001',label:'添加'},
                      {id:'002',label:'删除'}
                     ],
        onSelect : function(item){
          var node = $("#navTree").omTree("getSelected");
		  var newId = 0;
          $("#menu").omMenu("hide");
          if(item.id=="001"){
            if(node.text =="相位表"){
				
				newId = GetInsertId(addPhase+1,16);
				if( (newId != 0) &&(IsIdExist(phaseArray,newId) == 0))
				{
					var newnode = {id:"n1_"+phaseArray.length,pid:"n2",text:"相位号"+(newId),url: "phaseTable"};
					addPhase++;
					$("#navTree").omTree("insert",newnode, node);	
					phaseArray.push(newId);
				}

            }else if(node.text =="通道表"){

				newId = GetInsertId(addChannel+1,36);
				if( (newId != 0) &&(IsIdExist(channelArray,newId) == 0))
				{
					addChannel++;
					var newnode = {text:"通道号"+(newId),url: "channelTable"};
					$("#navTree").omTree("insert",newnode, node);	
					channelArray.push(newId);
				}

			}else if(node.text =="方案表"){

				newId = GetInsertId(addProgram+1,108);
				if( (newId != 0) &&(IsIdExist(programArray,newId) == 0))
				{
					addProgram++;
					var newnode = {text:"方案表"+(newId),url: "programTable"};
					$("#navTree").omTree("insert",newnode, node);	
					programArray.push(newId);
				}

            }else if(node.text =="绿信比表"){

				newId = GetInsertId(addSplit+1,36);
				if( (newId != 0) &&(IsIdExist(splitArray,newId) == 0))
				{
					addSplit++;
					var newnode = {text:"绿信比表"+(newId),url: "splitTable"};
					$("#navTree").omTree("insert",newnode, node);	
					splitArray.push(newId);
				}

            }else if(node.text =="相序表"){

				newId = GetInsertId(nPhaseTurnCount+1,16);
				if( (newId != 0) &&(IsIdExist(phaseTurnArray,newId) == 0))
				{
					nPhaseTurnCount++;
					var newnode = {text:"相序表"+(newId),url: "sequenceTable"};
					$("#navTree").omTree("insert",newnode, node);	
					phaseTurnArray.push(newId);
				}
              
            }else if(node.text =="时基动作表"){

				newId = GetInsertId(nActionCount+1,255);
				if( (newId != 0) &&(IsIdExist(ActionArray,newId) == 0))
				{
					nActionCount++;
					var newnode = {text:"动作表"+(newId),url: "timeBasedActionTable"};
					$("#navTree").omTree("insert",newnode, node);	
					ActionArray.push(newId);
				}
              
            }else if(node.text =="时段"){

				newId = GetInsertId(nTimeInterval+1,16);
				if( (newId != 0) &&(IsIdExist(TimeIntervalArray,newId) == 0))
				{
					nTimeInterval++;
					var newnode = {text:"时段表"+(newId),url: "timeInterval"};
					$("#navTree").omTree("insert",newnode, node);	
					TimeIntervalArray.push(newId);
				}
              
            }else if(node.text =="调度计划"){

				newId = GetInsertId(nScheduleCount+1,40);
				if( (newId != 0) &&(IsIdExist(ScheduleArray,newId) == 0))
				{
					nScheduleCount++;
					var newnode = {text:"调度表"+(newId),url: "scheduling"};
					$("#navTree").omTree("insert",newnode, node);	
					ScheduleArray.push(newId);
				}
              
            }else if(node.text =="跟随相位"){

				newId = GetInsertId(nFollowPhaseCount+1,16);
				if( (newId != 0) &&(IsIdExist(FollowPhaseArray,newId) == 0))
				{
					nFollowPhaseCount++;
					var newnode = {text:"跟随相位"+(newId),url: "overlappingTable"};
					$("#navTree").omTree("insert",newnode, node);	
					FollowPhaseArray.push(newId);
				}
            }else if(node.text =="车辆检测"){
				
				newId = GetInsertId(nVehicleDetectorCount+1,72);
				if( (newId != 0) &&(IsIdExist(VehicleDetectorArray,newId) == 0))
				{
					nVehicleDetectorCount++;
					var newnode = {text:"车辆检测器"+(newId),url: "vehicleDetector"};
					$("#navTree").omTree("insert",newnode, node);	
					VehicleDetectorArray.push(newId);
				}
            }else if(node.text =="行人检测"){

				newId = GetInsertId(nPedestrianDetectorCount+1,8);
				if( (newId != 0) &&(IsIdExist(PedestrianDetectorArray,newId) == 0))
				{
					nPedestrianDetectorCount++;
					var newnode = {text:"行人检测器"+(newId),url: "pedestrianDetector"};
					$("#navTree").omTree("insert",newnode, node);	
					PedestrianDetectorArray.push(newId);
				}
            }				
			
			
			SaveTreeParam();
          }      
        //删除处理
          else if(item.id === "002")
          {
			  var index = 0;
			  var text = node.text;
              if(text.indexOf("相位号") != -1){
				
				index = GetArrayIndex(phaseArray,text.substr(3));
				phaseArray.splice(index,1);
				addPhase--;

              }else if(text.indexOf("通道号") != -1){

				index = GetArrayIndex(channelArray,text.substr(3));
				channelArray.splice(index,1);

                addChannel--;
              }else if(text.indexOf("方案表") != -1){

				index = GetArrayIndex(programArray,text.substr(3));
				programArray.splice(index,1);

                addProgram--;
              }else if(text.indexOf("绿信比表") != -1){

				index = GetArrayIndex(splitArray,text.substr(4));
				splitArray.splice(index,1);

                addSplit--;
              }else if(text.indexOf("相序表") != -1){///////

				index = GetArrayIndex(phaseTurnArray,text.substr(3));
				phaseTurnArray.splice(index,1);

                nPhaseTurnCount--;
              }else if(text.indexOf("动作表") != -1){

				index = GetArrayIndex(ActionArray,text.substr(3));
				ActionArray.splice(index,1);

                nActionCount--;
              }else if(text.indexOf("时段表") != -1){

				index = GetArrayIndex(TimeIntervalArray,text.substr(3));
				TimeIntervalArray.splice(index,1);

                nTimeInterval--;
              }else if(text.indexOf("调度表") != -1){

				index = GetArrayIndex(ScheduleArray,text.substr(3));
				ScheduleArray.splice(index,1);

                nScheduleCount--;
              }else if(text.indexOf("跟随相位") != -1){

				index = GetArrayIndex(FollowPhaseArray,text.substr(4));
				FollowPhaseArray.splice(index,1);

                nFollowPhaseCount--;
              }else if(text.indexOf("车辆检测") != -1){

				index = GetArrayIndex(VehicleDetectorArray,text.substr(5));
				VehicleDetectorArray.splice(index,1);

                nVehicleDetectorCount--;
              }else if(text.indexOf("行人检测") != -1){

				index = GetArrayIndex(PedestrianDetectorArray,text.substr(5));
				PedestrianDetectorArray.splice(index,1);

                nPedestrianDetectorCount--;
              }					  
			  
			  SaveTreeParam();
			  $("#navTree").omTree("remove", node);
          }

        }
      });
      }
    });
  </script>
</head>
<body>
  <div id="north-panel">
    <table cellpadding="0" style="width:100%;height:0px;border-collapse:collapse">
      <tr style="width:100%;height:80px">
        <td style="width:781px;background:url(../images/public/banner/mid.png) no-repeat;">
        </td>
        <td id="headBgLeft" bgcolor="#1e0a07">&nbsp;</td>
        <td id="headBgRight" style="background:url(../images/public/banner/other.png);width:212px"></td>
      </tr>
    </table>
  </div>
  <div id="center-panel">
    <div id="center-tab" style="float:left;width:810px;padding-left:10px" onchange="SaveChange()">
      <ul>
       <!--  <li><a href="#tab1">信号机配置</a></li> -->
      </ul>
	 
    </div>
  </div>
  <div id="west-panel" >
      <ul id="navTree" ></ul>
      <ul id="menu"></ul>
  </div>
  <div id="east-panel">
    <table>
      <tr>
        <td>
          <input type="submit" value="手动保存当前参数" id="SaveConfigBtn" onClick="SaveChange()"/>
        </td>
      </tr>
	  <tr>
        <td>
          <input type="submit" value="重新获取信号机参数" id="BtnReset" onClick="ResetPara()"/>
        </td>
	  </tr>
	  <tr>
        <td>
          <input type="submit" value="清空所有配置信息" id="BtnClear" onClick="ClearPara()"/>
        </td>
	  </tr>
	  <tr>
        <td>
          <input type="submit" value="下发参数到板卡" id="BtnSaveAll" onClick="SaveAll()"/>
        </td>
	  </tr>
      <tr>
        <td>
          <form name="saveas" action="" method="post">
              <textarea id="content_1" disabled="disabled">下载信息</textarea>
              <br />
              <input type="button" value="导出配置信息到文件" id="download" onClick="saveFile('content_1')">
          </form>
        </td>
      </tr>
      <tr>
        <td>
          <form action="/goform/upldForm" method="post" enctype="multipart/form-data">  
              <input id="fileUpload" type="file" name="fileUpload">
              <input id="fileSubmit" type="submit" value='从文件导入信息'name="fileSubmit">
          </form> 
        </td>
      </tr>
	  <tr></tr><tr></tr>
	  <tr>
		<td>WebApp 版本号:</td>		
	  </tr>
	  <tr><td><input type="text"  id="WebAppInfo" value="0" readonly="readonly"/></td></tr>	  
	  <tr>
		<td>INI 解析库版本号:</td>		
	  </tr>
	  <tr><td><input type="text"  id="IniInfo" value="0" readonly="readonly"/></td></tr>
	  <tr>
		<td>HIKCFG 解析库版本号: </td>
	  </tr>
		<tr><td><input type="text"  id="HikCfgInfo" value="0" readonly="readonly"/></td></tr>	  
    </table>
  </div>
  
</body>
</html>
