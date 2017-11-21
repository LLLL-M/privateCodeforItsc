<script>
 
	var PhaseTurnNode = $("#navTree").omTree("getSelected").text;
    var NodeNo = PhaseTurnNode.substr(3);
    $("#TimeIntervalNo").attr('value',NodeNo);

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

//参数分别是小时和分钟及动作号,新增一个标志flag，flag==1时表明是手动添加的，则向板卡发送修改信息。
function addNewLine(hourIndex,minuteIndex,actionId,flag)
{
	var lineNum = document.getElementById('TimeIntervalTable').rows.length;//获得表格的行数

	var x = document.getElementById('TimeIntervalTable').insertRow(lineNum);//插入到最后一行
	
	var y = x.insertCell(0);//一次插入3列
	var z = x.insertCell(1);
	var w = x.insertCell(2);//注意，JS里面的colspan属性在IE下面是不支持的，必须手动设置!!!!!!!!!!!!!!!!!

	var hour = "Hour_"+(lineNum-1);
	var minute = "Minute_"+(lineNum-1);
	var action = "ActionId_"+(lineNum-1);

	y.innerHTML = "<td>"+(lineNum-1)+"</td>";//依次递增行数
	y.className = "tdpaddingleft";//其实是所有属性都需要调用JS的方法~!!!!

	z.innerHTML = "<td><select id="+hour+"></select> : <select id="+minute+"></select></td>";
	z.colSpan = 2;
	z.className = "tdpaddingleft";

	w.innerHTML = "<td><select id="+action+"></select></td>";
	w.colSpan = 2;
	w.className = "tdpaddingleft";


	//分别给时间下拉框和分钟下拉框赋值
	y = document.getElementById(hour);
	for(var i = 0 ; i < 24 ; i++)
	{
		z = document.createElement("option");
		z.text = i;
		y.add(z,null);
	}
	y.options[hourIndex].selected = true;//设置时间

	y = document.getElementById(minute);
	for(var i = 0 ; i < 60 ; i++)
	{
		z = document.createElement("option");
		z.text = i;
		y.add(z,null);
	}
	y.options[minuteIndex].selected = true;
	
	y = document.getElementById(action);
	for(var i = 1 ; i <= nActionCount ; i++)
	{
		z = document.createElement("option");
		z.text = ActionArray[i-1];
		y.add(z,null);
	}
	//添加特殊控制
	z = document.createElement("option");
	z.text="关灯控制";
	y.add(z,null);
	
	z = document.createElement("option");
	z.text="全红控制";
	y.add(z,null);
	
	z = document.createElement("option");
	z.text="感应控制";
	y.add(z,null);
	
	z = document.createElement("option");
	z.text="黄闪控制";
	y.add(z,null);	
	
	var index = 0;
	index = GetArrayIndex(ActionArray,actionId);
	
	if((actionId >= 115)&&(actionId <= 116))//如果找不到动作号，则说明是特殊控制
	{
		index = (actionId - 115+nActionCount);
	}
	else if((actionId >= 118)&&(actionId <= 119))//115 116 118 119才是有效的特殊控制ID
	{
		index = (actionId - 115+nActionCount - 1);
	}
	
	//alert("===>  index  "+index + "   actionId : " +actionId + " nActionCount " +nActionCount);
	//alert("==>   Action Id   "+actionId + "   diff  "+());
	y.options[index].selected = true;

	$("#TotalTimeCount").text(lineNum - 1);//这里有个隐藏字段用来传递总数，方便BOARD轮询。
	
	if(flag == 1)
	{
		doTrafficParams("timeInterval:set");
	}
	//alert($("#TotalTimeCount").text());

}

function delLastLine()
{
	var lineNum = document.getElementById('TimeIntervalTable').rows.length;//获得表格的行数
	
	if(lineNum > 3)
	{
		document.getElementById('TimeIntervalTable').deleteRow(lineNum-1);

		$("#TotalTimeCount").text(document.getElementById('TimeIntervalTable').rows.length - 2);//这里有个隐藏字段用来传递总数，方便BOARD轮询。

	}
}


</script>
<form traffic='sequenceTable'>
  	<br />
  	<h1>时段</h1>
  	<br />
	<table border="1" cellspacing="0" cellpadding="0"  class="edittableNo" id="TimeIntervalTable">
		<tr>
			<td class="tdpaddingleft">时段表 :</td>
			<td colspan="4">
				<select name="TimeIntervalNo" id="TimeIntervalNo" class="selectwidth" disabled="disabled">
					<option value="1">1</option>
		            <option value="2">2</option>
		            <option value="3">3</option>
		            <option value="4">4</option>
		            <option value="5">5</option>
		            <option value="6">6</option>
		            <option value="7">7</option>
		            <option value="8">8</option>
		            <option value="9">9</option>
		            <option value="10">10</option>
		            <option value="11">11</option>
		            <option value="12">12</option>
		            <option value="13">13</option>
		            <option value="14">14</option>
		            <option value="15">15</option>
		            <option value="16">16</option>
	            </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" id="xxx">时段号</td>
			<td class="tdpaddingleft" colspan="2">时段</td>
			<td class="tdpaddingleft" colspan="2">动作</td>
		</tr>
	</table>
<!--	<p id="TotalTimeCount"  style="display:none" >时段表总数</p>  -->
	<p id="TotalTimeCount" style="display:none" >时段表总数</p>
</form>
<form>
<input type="button" value="新增" onclick="addNewLine(0,0,1,1)">
<input type="button" value="删除" onclick="delLastLine()">
</form>