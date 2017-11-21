<script type="text/javascript">
    $(document).ready(function() {
        var test = $("#reg").validate({
            rules : {
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
                InductionNumber : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    }
                 },
                messages : {
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
                  InductionNumber : {
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
	for(i = 1 ;  i <= 8; i++)
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
		
		
	var node = $("#navTree").omTree("getSelected").text;
	var nodeNo = node.substr(5);
	$("#DetectorNo").attr('value',nodeNo);		//用来确定显示的ID号		
		
</script>
<form traffic='pedestrianDetector' id="reg">
  	<br />
  	<h1>行人检测器</h1>
  	<br />
	<table border="1" cellspacing="0" cellpadding="0"  class="edittableNo">
		<tr>
			<td class="tdpaddingleft" colspan="4">检测器编号:</td>
			<td class="tdpaddingleft" colspan="10">
				<select name="DetectorNo" id="DetectorNo" class="selectwidth" disabled="disabled">

	            </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2" rowspan="5">属性&nbsp;&nbsp;</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2">请求相位:</td>
			<td class="tdpaddingleft" colspan="10">
				<select name="RequestPhase" id="RequestPhase" class="selectwidth" >

	            </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2">无响应时间：</td>
			<td class="tdpaddingleft" colspan="10">
				<input type="text" name="NoResponseTime" id="NoResponseTime" value="0"/>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2">最大持续时间：</td>
			<td class="tdpaddingleft" colspan="10">
				<input type="text" name="MaxDuration" id="MaxDuration" value="0"/>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2">感应数/分钟：</td>
			<td class="tdpaddingleft" colspan="10">
				<input type="text" name="InductionNumber" id="InductionNumber" value="0"/>
			</td>
		</tr>
	</table>
</form>
