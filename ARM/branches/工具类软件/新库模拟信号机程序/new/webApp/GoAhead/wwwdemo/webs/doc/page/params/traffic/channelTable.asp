<!-- <script type="text/javascript" src="../script/paramconfig.js"></script> -->
<!-- 此处添加js调用会因重复加载产生数据变化 -->
<script type="text/javascript">
    var ChannelNode = $("#navTree").omTree("getSelected").text;
    var NodeNo = ChannelNode.substr(3);
    $("#channelNum").attr('value',NodeNo);

    var obj=document.getElementById('controlSource');
    var Cphase;
   // if(addPhase<2)
   // {
    //  alert("请先添加相位");
  //  }
    for(Cphase=0;Cphase<=addPhase;Cphase++)
    {
      //obj.add(new Option("文本","值"));    //这个只能在IE中有效
	  if(Cphase == 0)
		  //
		obj.options.add(new Option(0,0)); 
	  else
      	obj.options.add(new Option(phaseArray[Cphase-1],phaseArray[Cphase-1])); //这个兼容IE与firefox
	  //alert("===>  " + phaseArray[Cphase-1]);
    }
</script>
<form traffic='channelTable'>
  <br />
  <h1>通道表</h1>
  <br />
  <table border="1" cellspacing="0" cellpadding="0"  class="edittableNo">
    <tr>
      <td class="tdpaddingleft">通道号:</td>
      <td class="tdpaddingleft">
        <select name="channelNum" id="channelNum" class="selectwidth" disabled="disabled">
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
          <option value="17">17</option>
          <option value="18">18</option>
          <option value="19">19</option>
          <option value="20">20</option>
          <option value="21">21</option>
          <option value="22">22</option>
          <option value="23">23</option>
          <option value="24">24</option>
          <option value="25">25</option>
          <option value="26">26</option>
          <option value="27">27</option>
          <option value="28">28</option>
          <option value="29">29</option>
          <option value="30">30</option>
          <option value="31">31</option>
          <option value="32">32</option>
        </select>
      </td>
    </tr>
    <tr>
      <td class="tdpaddingleft">控制源:</td>
      <td class="tdpaddingleft">
        <select name="controlSource" id="controlSource" class="selectwidth" >
        </select>
      </td>
    </tr>
    <tr>
      <td class="tdpaddingleft">控制类型:&nbsp;&nbsp;</td>
      <td class="tdpaddingleft">
        <select name="controlType" id="controlType" class="selectwidth">
          <option value="1">其他</option>
          <option value="2" selected="selected">机动车</option>
          <option value="3">行人</option>
          <option value="4">跟随</option>
        </select>
      </td>
    </tr>
    <tr>
      <td class="tdpaddingleft">闪光模式:</td>
      <td class="tdpaddingleft">
        <select name="flashMode" id="flashMode" class="selectwidth" >
          <option value="1">交替</option>
          <option value="2">红闪</option>
          <option value="3" selected="selected">黄闪</option>
        </select>
      </td>
    </tr>
    <tr>
      <td class="tdpaddingleft">辉度模式:</td>
      <td class="tdpaddingleft">
        <select name="brightMode" id="brightMode" class="selectwidth" >
          <option value="1">交替</option>
          <option value="2">红灯</option>
          <option value="3">黄灯</option>
          <option value="4">绿灯</option>
        </select>
      </td>
    </tr>
  </table>
</form>
