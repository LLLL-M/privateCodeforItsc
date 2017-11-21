<script type="text/javascript">
    $(document).ready(function() {
        var test = $("#reg").validate({
            rules : {
                MinimumGreen : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                MaximumGreenOne : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                MaximumGreenTwo : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                ExtensionGreen : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                MaximumRestrict : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                DynamicStep : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                YellowLightTime : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                AllRedTime : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                RedLightProtect : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                PedestrianRelease : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                PedestrianCleaned : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                IncreaseInitValue : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                MaximumInitialValue : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                TimeBeforeDecrease : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                VehicleBeforeDecrease : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                DecreaseTime : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                UnitDeclineTime : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                MinimumInterval : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    }
                },
                messages : {
                  MinimumGreen : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  MaximumGreenOne : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  MaximumGreenTwo : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  ExtensionGreen : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  MaximumRestrict : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  DynamicStep : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  YellowLightTime : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  AllRedTime : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  RedLightProtect : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  PedestrianRelease : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  PedestrianCleaned : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  IncreaseInitValue : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  MaximumInitialValue : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  TimeBeforeDecrease : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  VehicleBeforeDecrease : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  DecreaseTime : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    }, 
                  UnitDeclineTime : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  MinimumInterval : {
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

        var node = $("#navTree").omTree("getSelected").text;
        var nodeNo = node.substr(3);
        $("#PhaseAccount").attr('value',nodeNo);
</script>
<form traffic='phaseTable' id="reg">
  <br />
  <h1>相位表</h1>
  <br />
	<table border="1" cellspacing="0" cellpadding="0"  class="edittableNo">
		<tr>
			<td class="tdpaddingleft" colspan="2">相位号:</td>
			<td class="tdpaddingleft" colspan="3">
				<select name="PhaseAccount" id="PhaseAccount" class="selectwidth" disabled="disabled">
					<option value="1">相位1</option>
	        <option value="2">相位2</option>
					<option value="3">相位3</option>
	        <option value="4">相位4</option>
          <option value="5">相位5</option>
          <option value="6">相位6</option>
          <option value="7">相位7</option>
          <option value="8">相位8</option>
          <option value="9">相位9</option>
          <option value="10">相位10</option>
          <option value="11">相位11</option>
          <option value="12">相位12</option>
          <option value="13">相位13</option>
          <option value="14">相位14</option>
          <option value="15">相位15</option>
          <option value="16">相位16</option>
	      </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="2">环号:</td>
			<td class="tdpaddingleft" colspan="3">
				<select name="CircleAccount" id="CircleAccount" class="selectwidth">
					<option value="1">环1</option>
					<option value="2">环2</option>
					<option value="3">环3</option>
					<option value="4">环4</option>
				  </select>
			</td>
		</tr>

		<tr>
			<td class="tdpaddingleft" rowspan="11">基础时间&nbsp;&nbsp;</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">最小绿:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="MinimumGreen" id="MinimumGreen" class="inputwidth" value="0"/>
	    	</td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">最大绿1:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="MaximumGreenOne" id="MaximumGreenOne" class="inputwidth" value="0"/></td>
	    </tr>
	    <tr>
			<td class="tdpaddingleft">最大绿2:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="MaximumGreenTwo" id="MaximumGreenTwo" class="inputwidth" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">延长绿:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="ExtensionGreen" id="ExtensionGreen" class="inputwidth" value="0"/></td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">最大值限制:</td>
	        <td class="tdpaddingleft" colspan="3">
	            <input type="text" name="MaximumRestrict" id="MaximumRestrict" class="inputwidth" value="0"/></td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">动态步长:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="DynamicStep" id="DynamicStep" class="inputwidth" value="0"/>
	    	</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">黄灯时间:</td>
	        <td class="tdpaddingleft" colspan="3">
	        	<input type="text" name="YellowLightTime" id="YellowLightTime" class="inputwidth" value="3"/></td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">全红时间:</td>
	        <td class="tdpaddingleft" colspan="3">
	        	<input type="text" name="AllRedTime" id="AllRedTime" class="inputwidth" value="2"/>
	        </td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">绿闪时间:</td>
	        <td class="tdpaddingleft" colspan="3">
	        	<input type="text" name="GreenLightTime" id="GreenLightTime" class="inputwidth" value="2"/>
	        </td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">红灯保护:</td>
	    		<td class="tdpaddingleft" colspan="3">
	    			<input type="text" name="RedLightProtect" id="RedLightProtect" class="inputwidth" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft" rowspan="4">行人</td>		
		</tr>
		<tr>
			<td class="tdpaddingleft">行人放行:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="PedestrianRelease" id="PedestrianRelease" class="inputwidth" value="10"/></td>						
		</tr>
		<tr>
			<td class="tdpaddingleft">行人清空:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="PedestrianCleaned" id="PedestrianCleaned" class="inputwidth" value="6"/></td>						
		</tr>	
		<tr>
			<td class="tdpaddingleft">保持行人放行:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="KeepPedestrianRelease" id="KeepPedestrianRelease" class="inputwidth"/></td>						
		</tr>
<!-- 		<tr>
			<td class="tdpaddingleft" rowspan="9">密度时间</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">增加初始值(/10):</td>
	        <td class="tdpaddingleft" colspan="3">
	        	<input type="text" name="IncreaseInitValue" id="IncreaseInitValue" class="inputwidth" value="0"/></td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">增加初始值计算</td>
			<td class="tdpaddingleft" colspan="3">
				<input type="checkbox" name="IncreaseInitialValueCalculation" id="IncreaseInitialValueCalculation" class="inputwidth"/></td>
		</tr>
		<tr>	
	    	<td class="tdpaddingleft">最大初始值:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="MaximumInitialValue" id="MaximumInitialValue" class="inputwidth" value="0"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">递减前时间:</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="TimeBeforeDecrease" id="TimeBeforeDecrease" class="inputwidth" value="0"/>
	    	</td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">递减前车辆:</td>
			<td class="tdpaddingleft" colspan="3">
				<input type="text" name="VehicleBeforeDecrease" id="VehicleBeforeDecrease" class="inputwidth" value="0"/></td>
		</tr>
		<tr>	
			<td class="tdpaddingleft">递减时间:</td>
			<td class="tdpaddingleft" colspan="3">
				<input type="text" name="DecreaseTime" id="DecreaseTime" class="inputwidth" value="0"/>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">单位递减率</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="UnitDeclineTime" id="UnitDeclineTime" class="inputwidth" value="0"/></td>
	    </tr>
	    <tr>	
			<td class="tdpaddingleft">最小间隔(/10):</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="text" name="MinimumInterval" id="MinimumInterval" class="inputwidth" value="0"/></td>
		</tr> -->
		<tr>
			<td class="tdpaddingleft" rowspan="7">选项</td>
    </tr>
		<tr>
			<td class="tdpaddingleft">不锁定滞留请求</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="NoLockDetentionRequest" id="NoLockDetentionRequest" class="inputwidth"/></td>						
		</tr>
		<tr>
			<td class="tdpaddingleft">双入口相位</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="DoubleEntrancePhase" id="DoubleEntrancePhase" class="inputwidth"/></td>						
		</tr>	
		<tr>
			<td class="tdpaddingleft">保证流量密度延长绿&nbsp;&nbsp;</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="GuaranteeFluxDensityExtensionGreen" id="GuaranteeFluxDensityExtensionGreen" class="inputwidth"/></td>						
		</tr>
		<tr>
			<td class="tdpaddingleft">有条件服务有效</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="ConditionalServiceValid" id="ConditionalServiceValid" class="inputwidth"/></td>						
		</tr>
		<tr>
			<td class="tdpaddingleft">同时空当失效</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="MeanwhileEmptyLoseEfficacy" id="MeanwhileEmptyLoseEfficacy" class="inputwidth"/></td>						
		</tr>
		<tr>
			<td class="tdpaddingleft">使能相位</td>
	    	<td class="tdpaddingleft" colspan="3">
	    		<input type="checkbox" name="Enable" id="Enable" class="inputwidth" checked="checked"/></td>						
		</tr>		

    <tr>
      <td class="tdpaddingleft" rowspan="8">综合</td>
    </tr>
		<tr>
			<td class="tdpaddingleft">初始化:</td>
			<td class="tdpaddingleft" colspan="3">
				<select name="Initialize" id="Initialize" class="selectwidth" >
					<option value="1">未启用</option>
	    			<option value="2">绿灯</option>
					<option value="3">机动车绿灯</option>
	    			<option value="4">黄灯</option>
					<option value="5">红灯</option>
	            	<option value="6">关灯</option>
	            </select>
			</td>
		</tr>
		<tr>	
			<td class="tdpaddingleft">非感应</td>
			<td class="tdpaddingleft" colspan="3">
				<select name="NonInduction" id="NonInduction" class="selectwidth" >
					<option value="1">未定义</option>
	        		<option value="2">TS非感应1</option>
					<option value="3">TS非感应2</option>
	        		<option value="4">TS非感应1-2</option>
	        	</select>
			</td>
		</tr>
		<tr>	
			<td class="tdpaddingleft">自动闪光接入</td>
	        <td class="tdpaddingleft" colspan="3">
	        	<input type="checkbox" name="AutomaticFlashInto" id="AutomaticFlashInto" class="inputwidth"/></td>
		</tr>
		<tr>
			<td class="tdpaddingleft">机动车自动请求:</td>
			<td class="tdpaddingleft" colspan="3">
				<select name="VehicleAutomaticRequest" id="VehicleAutomaticRequest" class="selectwidth">
					<option value="1">未定义</option>
	        		<option value="2">最小请求</option>
					<option value="3">最大请求</option>
	        		<option value="4">软请求</option>
	        	</select>
			</td>
		</tr>
		<tr>	
			<td class="tdpaddingleft">行人自动请求</td>
			<td class="tdpaddingleft" colspan="3">
				<input type="checkbox" name="PedestrianAutomaticRequest" id="PedestrianAutomaticRequest" class="inputwidth"/></td>
		</tr>
		<tr>	
			<td class="tdpaddingleft">自动闪光退出</td>
			<td class="tdpaddingleft" colspan="3">
				<input type="checkbox" name="AutomaticFlashExit" id="AutomaticFlashExit" class="inputwidth"/>
			</td>
		</tr>
		<tr>	
			<td class="tdpaddingleft">自动行人放行</td>
			<td class="tdpaddingleft" colspan="3">
				<input type="checkbox" name="AutoPedestrianPass" id="AutoPedestrianPass" class="inputwidth"/>
			</td>
		</tr>

	</table>
</form>
