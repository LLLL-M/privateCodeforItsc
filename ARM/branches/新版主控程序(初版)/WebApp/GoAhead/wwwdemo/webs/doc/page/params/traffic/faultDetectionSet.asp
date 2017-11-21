<script type="text/javascript">
	function isChanged(){
		var tempValue1 = $('#VoltageDetectionTimes').val()*20;
		document.getElementById("VDT").innerHTML = tempValue1; 
		var tempValue2 = $('#RedLightDetectionTimes').val();
		document.getElementById("RLDT").innerHTML = (tempValue2-1)*250+500;
		var tempValue3 = $('#ConflictDetectionAttempts').val();
		document.getElementById("CDA").innerHTML = (tempValue3-1)*250+500; 
		var tempValue4 = $('#ManualPanelKeyNumber').val()*20;
		document.getElementById("MPKN").innerHTML = tempValue4; 
		var tempValue5 = $('#RemoteControlKeyNumber').val()*20;
		document.getElementById("RCKN").innerHTML = tempValue5;  
	}
    $(document).ready(function() {
        var test = $("#reg").validate({
            rules : {
                VoltageDetectionTimes : {
                      required:true,
                      min:50,
                      max:15000,
                      number:true
                    },
                RedLightDetectionTimes : {
                      required:true,
                      min:2,
                      max:127,
                      number:true
                    },
                ConflictDetectionAttempts : {
                      required:true,
                      min:2,
                      max:20,
                      number:true
                    },
                ManualPanelKeyNumber : {
                      required:true,
                      min:5,
                      max:200,
                      number:true
                    },
                RemoteControlKeyNumber : {
                      required:true,
                      min:50,
                      max:200,
                      number:true
                    }
                 },
                messages : {
                  VoltageDetectionTimes : {
                      required:"此处不能为空",
                      min:"取值范围为[50,15000]",
                      max:"取值范围为[50,15000]"
                    },
                  RedLightDetectionTimes : {
                      required:"此处不能为空",
                      min:"取值范围为[2,127]",
                      max:"取值范围为[2,127]"
                    },
                  ConflictDetectionAttempts : {
                      required:"此处不能为空",
                      min:"取值范围为[2,20]",
                      max:"取值范围为[2,20]"
                    },
                  ManualPanelKeyNumber : {
                      required:"此处不能为空",
                      min:"取值范围为[5,200]",
                      max:"取值范围为[5,200]"
                    },
                  RemoteControlKeyNumber : {
                      required:"此处不能为空",
                      min:"取值范围为[50,200]",
                      max:"取值范围为[50,200]"
                    }
                },
                 submitHandler : function(){
                 return false;
                }
            });
        });
</script>
<form traffic='faultDetectionSet' id="reg" onchange="event.stopPropagation()">
  	<br />
  	<h1>故障配置设置</h1>
  	<br />
	<table border="1" cellspacing="0" cellpadding="0"  class="edittableNo">
		<tr>
			<td class="tdpaddingleft">过欠压检测次数[50,15000]:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="VoltageDetectionTimes" id="VoltageDetectionTimes" value="100" onchange="isChanged()"/><span id="VDT">10</span>ms
	    	</td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">红灯熄灭检测次数[2,127]:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="RedLightDetectionTimes" id="RedLightDetectionTimes"  value="100" onchange="isChanged()"/><span id="RLDT">0</span>ms
	    	</td>
	    </tr>
	    <tr>
			<td class="tdpaddingleft">冲突检测次数[2,20]:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="ConflictDetectionAttempts" id="ConflictDetectionAttempts" value="10" onchange="isChanged()"/><span id="CDA">0</span>ms</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">手动面板按键次数[5,200]:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="ManualPanelKeyNumber" id="ManualPanelKeyNumber" value="100" onchange="isChanged()"/><span id="MPKN">0</span>ms</td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">遥控器按键次数[50,200]:</td>
	        <td class="tdpaddingleft" colspan="3">
	            <input type="text" name="RemoteControlKeyNumber" id="RemoteControlKeyNumber" value="100" onchange="isChanged()"/><span id="RCKN">0</span>ms</td>
	    </tr>
	    <tr>
			<td class="tdpaddingleft">检测开关</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="SenseSwitch" id="SenseSwitch" value="0"/>
	    	</td>	
		</tr>
		<tr>
			<td class="tdpaddingleft">电流故障检测</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="DynamicStep" id="DynamicStep" value="0"/>
	    	</td>	
	    </tr>
	    <tr>
			<td class="tdpaddingleft">电压故障检测</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="CurrentFaultDetection" id="CurrentFaultDetection"  value="0"/>
	    	</td>	
	    </tr>
	    <tr>
			<td class="tdpaddingleft">电流故障报警并处理</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="AlarmAndFaultCurrent" id="AlarmAndFaultCurrent" value="0"/>
	    	</td>	
		</tr>
		<tr>
			<td class="tdpaddingleft">电压故障报警并处理</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="AlarmAndFaultVoltage" id="AlarmAndFaultVoltage" value="0"/>
	    	</td>					
		</tr>
		<tr>
			<td class="tdpaddingleft">启用看门狗</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="EnableWatchdog" id="EnableWatchdog" value="0"/>
	    	</td>	
		</tr>
		<tr>
			<td class="tdpaddingleft">启用GPS</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="EnableGPS" id="EnableGPS" value="0"/>
	    	</td>	
		</tr>
		<tr>
			<td class="tdpaddingleft">通道号</td>
			<td class="tdpaddingleft">红灯电流基准值</td>
			<td class="tdpaddingleft">红灯电流比较差值</td>
			<td class="tdpaddingleft">参考电流值</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">1</td>
			<td><input type="text" name="CNum1_0" id="CNum1_0" value="0"/></td>
			<td><input type="text" name="CNum1_1" id="CNum1_1" value="0"/></td>
			<td><input type="text" name="CNum1_2" id="CNum1_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">2</td>
			<td><input type="text" name="CNum2_0" id="CNum2_0" value="0"/></td>
			<td><input type="text" name="CNum2_1" id="CNum2_1" value="0"/></td>
			<td><input type="text" name="CNum2_2" id="CNum2_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">3</td>
			<td><input type="text" name="CNum3_0" id="CNum3_0" value="0"/></td>
			<td><input type="text" name="CNum3_1" id="CNum3_1" value="0"/></td>
			<td><input type="text" name="CNum3_2" id="CNum3_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">4</td>
			<td><input type="text" name="CNum4_0" id="CNum4_0" value="0"/></td>
			<td><input type="text" name="CNum4_1" id="CNum4_1" value="0"/></td>
			<td><input type="text" name="CNum4_2" id="CNum4_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">5</td>
			<td><input type="text" name="CNum5_0" id="CNum5_0" value="0"/></td>
			<td><input type="text" name="CNum5_1" id="CNum5_1" value="0"/></td>
			<td><input type="text" name="CNum5_2" id="CNum5_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">6</td>
			<td><input type="text" name="CNum6_0" id="CNum6_0" value="0"/></td>
			<td><input type="text" name="CNum6_1" id="CNum6_1" value="0"/></td>
			<td><input type="text" name="CNum6_2" id="CNum6_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">7</td>
			<td><input type="text" name="CNum7_0" id="CNum7_0" value="0"/></td>
			<td><input type="text" name="CNum7_1" id="CNum7_1" value="0"/></td>
			<td><input type="text" name="CNum7_2" id="CNum7_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">8</td>
			<td><input type="text" name="CNum8_0" id="CNum8_0" value="0"/></td>
			<td><input type="text" name="CNum8_1" id="CNum8_1" value="0"/></td>
			<td><input type="text" name="CNum8_2" id="CNum8_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">9</td>
			<td><input type="text" name="CNum9_0" id="CNum9_0" value="0"/></td>
			<td><input type="text" name="CNum9_1" id="CNum9_1" value="0"/></td>
			<td><input type="text" name="CNum9_2" id="CNum9_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">10</td>
			<td><input type="text" name="CNum10_0" id="CNum10_0" value="0"/></td>
			<td><input type="text" name="CNum10_1" id="CNum10_1" value="0"/></td>
			<td><input type="text" name="CNum10_2" id="CNum10_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">11</td>
			<td><input type="text" name="CNum11_0" id="CNum11_0" value="0"/></td>
			<td><input type="text" name="CNum11_1" id="CNum11_1" value="0"/></td>
			<td><input type="text" name="CNum11_2" id="CNum11_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">12</td>
			<td><input type="text" name="CNum12_0" id="CNum12_0" value="0"/></td>
			<td><input type="text" name="CNum12_1" id="CNum12_1" value="0"/></td>
			<td><input type="text" name="CNum12_2" id="CNum12_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">13</td>
			<td><input type="text" name="CNum13_0" id="CNum13_0" value="0"/></td>
			<td><input type="text" name="CNum13_1" id="CNum13_1" value="0"/></td>
			<td><input type="text" name="CNum13_2" id="CNum13_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">14</td>
			<td><input type="text" name="CNum14_0" id="CNum14_0" value="0"/></td>
			<td><input type="text" name="CNum14_1" id="CNum14_1" value="0"/></td>
			<td><input type="text" name="CNum14_2" id="CNum14_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">15</td>
			<td><input type="text" name="CNum15_0" id="CNum15_0" value="0"/></td>
			<td><input type="text" name="CNum15_1" id="CNum15_1" value="0"/></td>
			<td><input type="text" name="CNum15_2" id="CNum15_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">16</td>
			<td><input type="text" name="CNum16_0" id="CNum16_0" value="0"/></td>
			<td><input type="text" name="CNum16_1" id="CNum16_1" value="0"/></td>
			<td><input type="text" name="CNum16_2" id="CNum16_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">17</td>
			<td><input type="text" name="CNum17_0" id="CNum17_0" value="0"/></td>
			<td><input type="text" name="CNum17_1" id="CNum17_1" value="0"/></td>
			<td><input type="text" name="CNum17_2" id="CNum17_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">18</td>
			<td><input type="text" name="CNum18_0" id="CNum18_0" value="0"/></td>
			<td><input type="text" name="CNum18_1" id="CNum18_1" value="0"/></td>
			<td><input type="text" name="CNum18_2" id="CNum18_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">19</td>
			<td><input type="text" name="CNum19_0" id="CNum19_0" value="0"/></td>
			<td><input type="text" name="CNum19_1" id="CNum19_1" value="0"/></td>
			<td><input type="text" name="CNum19_2" id="CNum19_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">20</td>
			<td><input type="text" name="CNum20_0" id="CNum20_0" value="0"/></td>
			<td><input type="text" name="CNum20_1" id="CNum20_1" value="0"/></td>
			<td><input type="text" name="CNum20_2" id="CNum20_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">21</td>
			<td><input type="text" name="CNum21_0" id="CNum21_0" value="0"/></td>
			<td><input type="text" name="CNum21_1" id="CNum21_1" value="0"/></td>
			<td><input type="text" name="CNum21_2" id="CNum21_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">22</td>
			<td><input type="text" name="CNum22_0" id="CNum22_0" value="0"/></td>
			<td><input type="text" name="CNum22_1" id="CNum22_1" value="0"/></td>
			<td><input type="text" name="CNum22_2" id="CNum22_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">23</td>
			<td><input type="text" name="CNum23_0" id="CNum23_0" value="0"/></td>
			<td><input type="text" name="CNum23_1" id="CNum23_1" value="0"/></td>
			<td><input type="text" name="CNum23_2" id="CNum23_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">24</td>
			<td><input type="text" name="CNum24_0" id="CNum24_0" value="0"/></td>
			<td><input type="text" name="CNum24_1" id="CNum24_1" value="0"/></td>
			<td><input type="text" name="CNum24_2" id="CNum24_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">25</td>
			<td><input type="text" name="CNum25_0" id="CNum25_0" value="0"/></td>
			<td><input type="text" name="CNum25_1" id="CNum25_1" value="0"/></td>
			<td><input type="text" name="CNum25_2" id="CNum25_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">26</td>
			<td><input type="text" name="CNum26_0" id="CNum26_0" value="0"/></td>
			<td><input type="text" name="CNum26_1" id="CNum26_1" value="0"/></td>
			<td><input type="text" name="CNum26_2" id="CNum26_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">27</td>
			<td><input type="text" name="CNum27_0" id="CNum27_0" value="0"/></td>
			<td><input type="text" name="CNum27_1" id="CNum27_1" value="0"/></td>
			<td><input type="text" name="CNum27_2" id="CNum27_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">28</td>
			<td><input type="text" name="CNum28_0" id="CNum28_0" value="0"/></td>
			<td><input type="text" name="CNum28_1" id="CNum28_1" value="0"/></td>
			<td><input type="text" name="CNum28_2" id="CNum28_1" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">29</td>
			<td><input type="text" name="CNum29_0" id="CNum29_0" value="0"/></td>
			<td><input type="text" name="CNum29_1" id="CNum29_1" value="0"/></td>
			<td><input type="text" name="CNum29_2" id="CNum29_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">30</td>
			<td><input type="text" name="CNum30_0" id="CNum30_0" value="0"/></td>
			<td><input type="text" name="CNum30_1" id="CNum30_1" value="0"/></td>
			<td><input type="text" name="CNum30_2" id="CNum30_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">31</td>
			<td><input type="text" name="CNum31_0" id="CNum31_0" value="0"/></td>
			<td><input type="text" name="CNum31_1" id="CNum31_1" value="0"/></td>
			<td><input type="text" name="CNum31_2" id="CNum31_2" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">32</td>
			<td><input type="text" name="CNum32_0" id="CNum32_0" value="0"/></td>
			<td><input type="text" name="CNum32_1" id="CNum32_1" value="0"/></td>
			<td><input type="text" name="CNum32_2" id="CNum32_2" value="0"/></td>
		</tr>
	</table>
</form>
