<script type="text/javascript">
    $(document).ready(function() {
        var test = $("#reg").validate({
            rules : {
                split : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                pedestrianMove : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                pedestrianEmpty : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                frontierGreen : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    }
                 },
                messages : {
                  split : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  pedestrianMove : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  pedestrianEmpty : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  frontierGreen : {
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

	/*具体行的内容*/
    var Sphase;
    for(Sphase=1;Sphase<=addPhase;Sphase++)
    {
    	var x=document.getElementById('splitTable').insertRow(Sphase+1);
		var y=x.insertCell(0);
		var z=x.insertCell(1);
		var w=x.insertCell(2);
		var m=x.insertCell(3);
		y.innerHTML="相位"+phaseArray[Sphase-1];
		z.innerHTML="<input type='text' name=splitForPhase"+phaseArray[Sphase-1]+" id=splitForPhase"+phaseArray[Sphase-1]+"  value='0'/>";
		w.innerHTML="<select name=splitMode"+phaseArray[Sphase-1]+" id=splitMode"+phaseArray[Sphase-1]+" class='selectwidth'><option value='0'>未定义</option><option value='1'>其他类型</option><option value='2'>无</option><option value='3'>最小车辆响应</option><option value='4'>最大车辆响应</option><option value='5'>行人响应</option><option value='6'>最大车辆/行人响应</option><option value='7'>忽略相位</option></select>";
		m.innerHTML="<input type='checkbox' class='tdpaddingleft' name='coordinate'"+phaseArray[Sphase-1]+" id=coordinate"+phaseArray[Sphase-1]+" value='0'/>";
	}
	/*表头*/
	var splitNode = $("#navTree").omTree("getSelected").text;
    var SplitNodeNo = splitNode.substr(4);
    $("#splitNo").attr('value',SplitNodeNo);

	/*传参使用*/
	$("#TotalPhaseCount").text(document.getElementById('splitTable').rows.length - 2);//这里有个隐藏字段用来传递相位总数，方便BOARD轮询。

	//alert($("#TotalPhaseCount").text());

</script>
<form traffic='greenRatio' id="reg">
	<br />
	<h1>绿信比表</h1>
	<br />
	<table border="1" cellspacing="0" cellpadding="0"  class="edittableNo" id="splitTable">
		<tr>
			<td>绿信比表</td>
			<td colspan="3" >
				<select id="splitNo" disabled="disabled" class="selectwidth">
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
			<td>相位</td>
			<td>绿信比(秒)</td>
			<td>模式</td>
			<td>作为协调相位</td>
		</tr>		
<!-- 		<tr>
			<td class="tdpaddingleft">绿信比表:</td>
			<td class="tdpaddingleft" colspan="2">
				<select name="greenNum" id="greenNum" class="selectwidth" >
					<option value="1">相位1</option>
	               	<option value="2">相位2</option>
					<option value="3">相位3</option>
	               	<option value="4">相位4</option>
	               	<option value="5">相位5</option>
	               	<option value="6">相位6</option>
	            </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">绿信比:</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="text" name="split" id="split" value="0"/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;秒
	    	</td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">模式:</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<select name="greenType" id="greenType" class="selectwidth">
					<option value="1">无</option>
	               	<option value="2">可变标志</option>
					<option value="3">最小车辆响应</option>
	               	<option value="4">最大车辆响应</option>
	               	<option value="5">行人响应</option>
	               	<option value="6">最大车辆/行人响应</option>
	               	<option value="7">忽略相位</option>
	            </select>
			</td>
	    </tr>
	    <tr>
			<td class="tdpaddingleft">作为协调相位:&nbsp;&nbsp;</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="checkbox" name="coordinatePhase" id="coordinatePhase" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">作为关键相位:</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="checkbox" name="keyPhase" id="keyPhase" value="0"/></td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">作为固定相位:</td>
	        <td class="tdpaddingleft" colspan="2">
	            <input type="checkbox" name="fixedPhase" id="fixedPhase" value="0"/></td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">行人放行:</td>
	    	<td class="tdpaddingleft" colspan="2">
	    		<input type="text" name="pedestrianMove" id="pedestrianMove" value="0"/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;秒
	    	</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">行人清空:</td>
	        <td class="tdpaddingleft" colspan="2">
	        	<input type="text" name="pedestrianEmpty" id="pedestrianEmpty" value="0"/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;秒
	        </td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">临界绿灯:</td>
	        <td class="tdpaddingleft" colspan="2">
	        	<input type="text" name="frontierGreen" id="frontierGreen" value="0"/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;秒
	        </td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">清空模式:</td>
	    	<td class="tdpaddingleft" colspan="2">
		    	<select name="emptyType" id="emptyType" class="selectwidth" >
					<option value="1">红闪</option>
		            <option value="2">绿闪</option>
		        </select>
		    </td>
		</tr> -->
	</table>
	<p id="TotalPhaseCount"  style="display:none" >相位总数</p>
</form>
