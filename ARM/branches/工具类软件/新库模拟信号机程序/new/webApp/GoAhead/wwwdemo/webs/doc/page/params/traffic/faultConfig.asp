<!--<script type="text/javascript" src="../../../script/trafficParams.js"></script>-->
<script>
var startId, endId;	//声明全局变量，用来记录每次获取的故障记录的起始点和结束点，一次获取10条记录
var tableName = "";	//表名，用以区分当前显示的是日志表还是故障表
function getRecordMessage(){
		getFaultConfInfo(fillForm);
	function getFaultConfInfo(callback){
		var o = {RecordMessage:$("#RecordMessage").val()};
		var xmlStr = $.json2xml(o,{root:'FaultConfig'});
		$.ajax({
			type: 'GET',
			dataType: 'xml',
			data: xmlStr,
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
		
		var T = document.getElementById('RecordTable'),
			R = document.getElementById('RecordMessage');
		var i, j, row, cell, str;
		var msg, N;
		
		msg = R.value;
		if (msg.length == 0) {
			alert("没有记录了");
			return;
		}
		N = msg.split(';');
		//删除之前存在的记录
		for (i = T.rows.length - 1; i > 0; i--) {
			T.deleteRow(i);
		}
		//添加新获取的记录
		for (i = 0; i < N.length; i++) {
			str = N[i].split('|');
			if (str.length != 4) {
				alert("Error: str.length != 4, str = " + N[i] + '\nlength = ' + str.length);
				return;
			}
			//记录起始id
			if (i == 0) {
				startId = str[0];
			}
			row = T.insertRow(i + 1);
			for (j = 0; j < str.length; j++) {
				cell = row.insertCell(j);
				cell.innerHTML = str[j];
				cell.style.textAlign = 'center';
			}
			//每行最后插入一个复选框用于删除使用
			cell = row.insertCell(j);
			cell.innerHTML = '<input type="checkbox"/>';
			cell.style.textAlign = 'center';
		}
		//记录结束id
		endId = str[0];
	}
}

function deleteRecord()
{
	var T = document.getElementById('RecordTable');
	var	checkbox, i;
	var cmd = "delete from " + tableName + " where ";	//数据库删除操作的命令
	var flag = false;	//用以判断是否有数据被选中了
	
	for (i = T.rows.length - 1; i > 0; i--) {
		checkbox = T.rows[i].cells[4].firstChild;
		if (checkbox.checked) {
			cmd = cmd + "id=" + T.rows[i].cells[0].innerHTML + " or ";
			T.deleteRow(i);
			flag = true;
		}
	}
	if (!flag) {
		alert("你还没有选中数据，请选中之后再点击删除！");
		return;
	}
	cmd = cmd.slice(0, -4) + ";";	//去掉结尾多余的" or "
	//alert("数据库删除操作的命令：" + cmd);
	
	var o = {DeleteCommand:cmd};
	var xmlStr = $.json2xml(o,{root:'FaultConfig'});
	$.ajax({
		type: 'POST',
		dataType: 'xml',
		data: xmlStr,
		url: m_lHttp+m_szHostName+":"+m_lHttpPort +'/goform/faultConfig',
		success: function(data, textStatus){
					optResultDefault(data);
				}
	});
}

function showLastPage()
{
	var T = document.getElementById('RecordTable');
	var R = document.getElementById('RecordMessage');
	R.value = "select * from " + tableName + " where id<" + startId + " limit 10;";	//输入数据库的筛选条件
	getRecordMessage();	//获取记录
}

function showNextPage()
{
	var T = document.getElementById('RecordTable');
	var R = document.getElementById('RecordMessage');
	R.value = "select * from " + tableName + " where id>" + endId + " limit 10;";	//输入数据库的筛选条件
	getRecordMessage();	//获取记录
}

function showRecordTable(name)
{
	var T = document.getElementById('RecordTable');
	var R = document.getElementById('RecordMessage');
	T.caption.innerHTML = (name.localeCompare("T_Fault") == 0) ? "故障记录表" : "日志记录表";
	tableName = name;
	startId = "0";
	R.value = "select * from " + tableName + " where id>=" + startId + " limit 10;";	//输入数据库的筛选条件
	//alert("筛选条件:" + R.value);
	getRecordMessage();	//获取记录
	document.getElementById('TableArea').style.visibility='visible';		
}
</script>

<form traffic='faultConfig'>
	<br />
	<h1>故障配置</h1>
	<br />
	<table border="1" cellspacing="0" cellpadding="0"  class="edittableNo">
		<tr>
			<td class="tdpaddingleft">ID</td>
			<td class="tdpaddingleft">类型</td>
			<td class="tdpaddingleft">模式</td>
			<td class="tdpaddingleft" colspan="5">操作</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">1</td>
			<td class="tdpaddingleft">控制器故障</td>
			<td class="tdpaddingleft">其他</td>	
			<td class="tdpaddingleft" colspan="5">
				<select name="ControlRecord" id="ControlRecord" class="selectwidth" >
					<option value="1">记录</option>
		            <option value="0">不记录</option>
	            </select>
			</td>
			<!--<td><input type="button" value="显示控制器故障信息" onclick='showRecordTable("T_Fault")'></td>-->
			<td><button type="button" style="color:red" id="ShowFaultRecord" onclick='showRecordTable("T_Fault")'>显示控制器故障信息</button></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">2</td>
			<td class="tdpaddingleft">日志记录</td>
			<td class="tdpaddingleft">其他</td>	
			<td class="tdpaddingleft" colspan="5">
				<select name="LogRecord" id="LogRecord" class="selectwidth" >
					<option value="1">记录</option>
		            <option value="0">不记录</option>
	            </select>
			</td>
			<td><button type="button" style="color:red" id="ShowLogRecord" onclick='showRecordTable("T_Record")'>显示日志信息</button></td>
		</tr>
		<!--
		<tr>
			<td class="tdpaddingleft">2</td>
			<td class="tdpaddingleft">通信故障</td>
			<td class="tdpaddingleft">其他</td>	
			<td class="tdpaddingleft" colspan="5">
				<select name="CommunicatRecord" id="CommunicatRecord" class="selectwidth" >
					<option value="1">记录</option>
		            <option value="2">不记录</option>
	            </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">3</td>
			<td class="tdpaddingleft">检测器故障</td>
			<td class="tdpaddingleft">其他</td>	
			<td class="tdpaddingleft" colspan="5">
				<select name="DetectorRecord" id="DetectorRecord" class="selectwidth" >
					<option value="1">记录</option>
		            <option value="2">不记录</option>
	            </select>
			</td>
		</tr>
		-->
	</table>
	<!-- added by Jicky -->
	<input type="text" id="RecordMessage" name="RecordMessage" class="inputwidth" style="visibility:hidden" value="0"/>
	<div id="TableArea"  style="visibility:hidden" onchange="event.stopPropagation()">
	<table border="1" cellspacing="0" cellpadding="0" class="edittableNo" id="RecordTable">
		<caption>记录表</caption>
		<tr>
			<th class="tdpaddingleft">NO.</th>
			<th class="tdpaddingleft">类型</th>
			<th class="tdpaddingleft">时间</th>
			<th class="tdpaddingleft">信息</th>
			<th class="tdpaddingleft">删除与否</th>
		</tr>
	</table>
	<br />
	&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
	<input type="button" onclick="showLastPage()" style="width:60px;height:20px" id="LastPage" value="上一页">
	<input type="button" onclick="showNextPage()" style="width:60px;height:20px" id="NextPage" value="下一页">
	<input type="button" onclick="deleteRecord()" style="width:60px;height:20px" id="DeleteRecord" value="删除记录">
	</div>
</form>

