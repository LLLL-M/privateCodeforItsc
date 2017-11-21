<script type="text/javascript">
    $(document).ready(function() {
        var test = $("#reg").validate({
            rules : {
                StartFlashingYellowTime : {
                      required:true,
                      min:0,
                      max:20,
                      number:true
                    },
                StartAllRedTime : {
                      required:true,
                      min:0,
                      max:30,
                      number:true
                    },
                DegradationTime : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                SpeedFactor : {
                      required:true,
                      min:0,
                      max:9999,
                      number:true
                    },
                MinimumRedLightTime : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                CommunicationTimeout : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                FlashingFrequency : {
                      required:true,
                      min:60,
                      max:120,
                      number:true
                    },
                TwiceCrossingTimeInterval : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                TwiceCrossingReverseTimeInterval : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                SmoothTransitionPeriod : {
                      required:true,
                      min:0,
                      max:5,
                      number:true
                    },
                FlowCollectionPeriod : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    }
                },
                messages : {
                  StartFlashingYellowTime : {
                      required:"此处不能为空",
                      min:"取值范围为[0,20]",
                      max:"取值范围为[0,20]"
                    },
                  StartAllRedTime : {
                      required:"此处不能为空",
                      min:"取值范围为[0,30]",
                      max:"取值范围为[0,30]"
                    },
                  DegradationTime : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  SpeedFactor : {
                      required:"此处不能为空",
                      min:"取值范围为[0,9999]",
                      max:"取值范围为[0,9999]"
                    },
                  MinimumRedLightTime : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  CommunicationTimeout : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  FlashingFrequency : {
                      required:"此处不能为空",
                      min:"取值范围为[60,120]",
                      max:"取值范围为[60,120]"
                    },
                  TwiceCrossingTimeInterval : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  TwiceCrossingReverseTimeInterval : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  SmoothTransitionPeriod : {
                      required:"此处不能为空",
                      min:"取值范围为[0,5]",
                      max:"取值范围为[0,5]"
                    },
                  FlowCollectionPeriod : {
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
</script>
<form traffic='unitParams' id="reg">
    <br />
    <h1>单元参数</h1>
    <br />
	  <table border="1" cellspacing="0" cellpadding="0"  class="edittableNo">
        <tr>
          <td class="tdpaddingleft" rowspan="13">参数配置&nbsp;&nbsp;</td>
        </tr>
        <tr>
          <td class="tdpaddingleft">启动黄闪时间:</td>
          <td class="tdpaddingleft" colspan="3">
            <input type="text" name="StartFlashingYellowTime" id="StartFlashingYellowTime" class="inputwidth" value="6"/>
          </td>
        </tr>  
        <tr>
          <td class="tdpaddingleft">启动全红时间:</td>
          <td class="tdpaddingleft" colspan="3">
            <input type="text" name="StartAllRedTime" id="StartAllRedTime" class="inputwidth" value="6"/>
          </td>
        </tr>
        <tr>
          <td class="tdpaddingleft">降级时间:</td>
          <td class="tdpaddingleft" colspan="3">
            <input type="text" name="DegradationTime" id="DegradationTime" class="inputwidth"  value="0"/>
          </td>
        </tr>
        <tr>  
          <td class="tdpaddingleft">速度因子:</td>
          <td class="tdpaddingleft" colspan="3">
            <input type="text" name="SpeedFactor" id="SpeedFactor" class="inputwidth"  value="0"/>
          </td>
        </tr>
        <tr>
          <td class="tdpaddingleft">最小红灯时间:</label></td>
          <td class="tdpaddingleft" colspan="3">
            <input type="text" name="MinimumRedLightTime" id="MinimumRedLightTime" class="inputwidth"  value="0"/>
          </td>
        </tr>
        <tr>  
          <td class="tdpaddingleft">通信超时:</td>
          <td class="tdpaddingleft" colspan="3">
            <input name="CommunicationTimeout" id="CommunicationTimeout" type="text" class="inputwidth" value="0"/> 
          </td>
        </tr>
        <tr>
          <td class="tdpaddingleft">闪光频率:</td>
          <td class="tdpaddingleft" colspan="3">
            <input name="FlashingFrequency" id="FlashingFrequency" type="text" class="inputwidth"  value="60"/> 
          </td>
        </tr>
        <tr>
			    <td class="tdpaddingleft">二次过街时差:</td>
          <td class="tdpaddingleft" colspan="3">
            <input name="TwiceCrossingTimeInterval" id="TwiceCrossingTimeInterval" type="text" class="inputwidth" value="0"/>    
          </td>
        </tr>
        <tr>
          <td class="tdpaddingleft">二次过街逆向时差:</td>
          <td class="tdpaddingleft" colspan="3">
            <input name="TwiceCrossingReverseTimeInterval" id="TwiceCrossingReverseTimeInterval" type="text" class="inputwidth" value="0"/> 
          </td>
        </tr>
        <tr>  
			    <td class="tdpaddingleft">平滑过渡周期:</td>
          <td class="tdpaddingleft" colspan="3">
            <input name="SmoothTransitionPeriod" id="SmoothTransitionPeriod" type="text" class="inputwidth" value="2"/>
          </td>
        </tr>
		    <tr>
		      <td class="tdpaddingleft">流量采集周期:</td>
          <td class="tdpaddingleft" colspan="3">
            <input name="FlowCollectionPeriod" id="FlowCollectionPeriod" type="text" class="inputwidth" value="120"/>
          </td>
        </tr>
        <tr>  
          <td class="tdpaddingleft">采集单位</td>
          <td class="tdpaddingleft" colspan="3">
            <select name="CollectUnit" id="CollectUnit" class="inputwidth">
							<option value="0">秒</option>
							<option value="1">分钟</option>
            </select> 
          </td>
        </tr>
    		<tr>
    		  <td class="tdpaddingleft" rowspan="3">选项开关 </td>
    		</tr>
    		<tr>
          <td class="tdpaddingleft">自动行人清空</td>
    		  <td class="tdpaddingleft" colspan="3">
            <input name="AutoPedestrianEmpty" id="AutoPedestrianEmpty" type="checkbox" class="inputwidth"/>
          </td>
     		</tr>
    		<tr>
          <td class="tdpaddingleft">检测过压,降级黄闪&nbsp;&nbsp;</td>
    		  <td class="tdpaddingleft" colspan="3">
            <input name="OverpressureDetection" id="OverpressureDetection" type="checkbox" class="inputwidth"/> 			
          </td>
        </tr>	
	   </table>
 </form>
