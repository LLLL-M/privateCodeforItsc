<script type="text/javascript">
$(document).ready(function() {
	var test = $("#reg").validate({
		rules : {
			Delay : {
				  required:true,
				  min:0,
				  max:255,
				  number:true
				},
			FailureTime : {
				  required:true,
				  min:0,
				  max:255,
				  number:true
				},
			QueueLimit : {
				  required:true,
				  min:0,
				  max:255,
				  number:true
				},
			NoResponseTime : {
				  required:true,
				  min:0,
				  max:255,
				  number:true
				},
			MaxDuration : {
				  required:true,
				  min:0,
				  max:255,
				  number:true
				},
			Extend : {
				  required:true,
				  min:0,
				  max:255,
				  number:true
				},
			MaxVehicle : {
				  required:true,
				  min:0,
				  max:255,
				  number:true
				}
			 },
			messages : {
			  Delay : {
				  required:"此处不能为空",
				  min:"取值范围为[0,255]",
				  max:"取值范围为[0,255]"
				},
			  FailureTime : {
				  required:"此处不能为空",
				  min:"取值范围为[0,255]",
				  max:"取值范围为[0,255]"
				},
			  QueueLimit : {
				  required:"此处不能为空",
				  min:"取值范围为[0,255]",
				  max:"取值范围为[0,255]"
				},
			  NoResponseTime : {
				  required:"此处不能为空",
				  min:"取值范围为[0,255]",
				  max:"取值范围为[0,255]"
				},
			  MaxDuration : {
				  required:"此处不能为空",
				  min:"取值范围为[0,255]",
				  max:"取值范围为[0,255]"
				},
			  Extend : {
				  required:"此处不能为空",
				  min:"取值范围为[0,255]",
				  max:"取值范围为[0,255]"
				},
			  MaxVehicle : {
				  required:"此处不能为空",
				  min:"取值范围为[0,255]",
				  max:"取值范围为[0,255]"
				}
			},
			 submitHandler : function(){
			 return false;
			}
		});
	});

		
	var i ;
	var x = document.getElementById("DetectorNo");
	var z;		
	for(i = 1 ;  i <= 72; i++)
	{
		z = document.createElement("option");
		z.text = i;
		x.add(z,null);
	}//用来生成这么多的选项	

	x = document.getElementById("RequestPhase");
	for(i = 1 ;  i <= addPhase; i++)
	{
		z = document.createElement("option");
		z.text = phaseArray[i-1];
		x.add(z,null);
	}//用来生成这么多的选项	

	x = document.getElementById("SwitchPhase");
	for(i = 1 ;  i <= addPhase; i++)
	{
		z = document.createElement("option");
		z.text = phaseArray[i-1];
		x.add(z,null);
	}//用来生成这么多的选项		
		
	var node = $("#navTree").omTree("getSelected").text;
	var nodeNo = node.substr(5);
	$("#DetectorNo").attr('value',nodeNo);		//用来确定显示的ID号
	
		
		
</script>
<form traffic='vehicleDetector' id="reg">
	<br />
	<h1>车辆检测器</h1>
	<br />
	<table border="1" cellspacing="0" cellpadding="0"  class="edittableNo">
		<tr>
			<td class="tdpaddingleft" colspan="4">检测器号:</td>
			<td class="tdpaddingleft" colspan="10">
				<select name="DetectorNo" id="DetectorNo" class="selectwidth" disabled="disabled">

		        </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" rowspan="3" colspan="2">相位</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2">请求相位:</td>
			<td class="tdpaddingleft" colspan="10">
				<select name="RequestPhase" id="RequestPhase" class="selectwidth">

		        </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2">开关相位:</td>
			<td class="tdpaddingleft" colspan="10">
				<select name="SwitchPhase" id="SwitchPhase" class="selectwidth">

		        </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" rowspan="8" colspan="2">时间</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2">延迟(/10):</td>
			<td class="tdpaddingleft" colspan="10">
				<input type="text" name="Delay" id="Delay"/>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2">失败时间:</td>
			<td class="tdpaddingleft" colspan="10">
				<input type="text" name="FailureTime" id="FailureTime"/>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2">队列限制:</td>
			<td class="tdpaddingleft" colspan="10">
				<input type="text" name="QueueLimit" id="QueueLimit"/>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2">无响应时间:</td>
			<td class="tdpaddingleft" colspan="10">
				<input type="text" name="NoResponseTime" id="NoResponseTime"/>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2">最大持续时间:</td>
			<td class="tdpaddingleft" colspan="10">
				<input type="text" name="MaxDuration" id="MaxDuration"/>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2">延长(/10):</td>
			<td class="tdpaddingleft" colspan="10">
				<input type="text" name="Extend" id="Extend"/>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2">最大车辆数/分钟:</td>
			<td class="tdpaddingleft" colspan="10">
				<input type="text" name="MaxVehicle" id="MaxVehicle"/>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2">选项</td>
			<td class="tdpaddingleft" colspan="2">
				<input type="checkbox" name="Flow" id="Flow"/>流量
			</td>
			<td class="tdpaddingleft" colspan="2">
				<input type="checkbox" name="Occupancy" id="Occupancy"/>占有率
			</td>
			<td class="tdpaddingleft" colspan="2">
				<input type="checkbox" name="ProlongGreen" id="ProlongGreen"/>延长绿
			</td>
			<td class="tdpaddingleft" colspan="2">
				<input type="checkbox" name="AccumulateInitial" id="AccumulateInitial"/>积累初始
			</td>
			<td class="tdpaddingleft" colspan="2">
				<input type="checkbox" name="Queue" id="Queue"/>排队
			</td>
			<td class="tdpaddingleft" colspan="2">
				<input type="checkbox" name="Request" id="Request"/>请求
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" rowspan="4" colspan="2">锁定请求</td>
			<td class="tdpaddingleft" colspan="2">
				<input type="checkbox" name="RedInterval" id="RedInterval"/>红灯区间
			</td>
			<td class="tdpaddingleft" colspan="10">
				<input type="checkbox" name="YellowInterval" id="YellowInterval"/>黄灯时间
			</td>
		</tr>
	</table>
</form>
